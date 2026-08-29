#include "MontageForgeSubsystem.h"

#include "MontageForge.h"
#include "MontageForgeGameplayEventNotify.h"
#include "MontageRecipe.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "Factories/AnimMontageFactory.h"
#include "IAssetTools.h"
#include "Misc/PackageName.h"
#include "ObjectTools.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace MontageForgePrivate
{
	static void SaveAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return;
		}

		UPackage* Package = Asset->GetOutermost();
		if (!Package || !Package->IsDirty())
		{
			return;
		}

		const FString FileName = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());

		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		Args.SaveFlags = SAVE_NoError;

		UPackage::SavePackage(Package, Asset, *FileName, Args);
	}
}

UMontageForgeSubsystem* UMontageForgeSubsystem::Get()
{
	return GEditor ? GEditor->GetEditorSubsystem<UMontageForgeSubsystem>() : nullptr;
}

// -------------------------------------------------------------------------------------------------
// Build
// -------------------------------------------------------------------------------------------------

UAnimMontage* UMontageForgeSubsystem::CreateMontageAsset(
	UMontageRecipe* Recipe,
	UAnimSequence* Sequence,
	bool bOverwriteHandEdits,
	bool& bOutReplacedExisting,
	FString& OutError)
{
	const FString FolderPath = Recipe->OutputPath.IsEmpty()
		? FPackageName::GetLongPackagePath(Recipe->GetOutermost()->GetName())
		: Recipe->OutputPath;

	const FString AssetName = ObjectTools::SanitizeObjectName(Recipe->GetMontageAssetName());
	const FString ObjectPath = FolderPath / AssetName + TEXT(".") + AssetName;

	if (UAnimMontage* Existing = LoadObject<UAnimMontage>(nullptr, *ObjectPath))
	{
		if (!bOverwriteHandEdits)
		{
			OutError = FString::Printf(
				TEXT("'%s' already exists. Rebuilding replaces it and discards anything tuned by hand, "
					 "which is usually the only work here that is not reproducible - capture the "
					 "timings first if you want to keep them, then rebuild with overwrite."),
				*ObjectPath);
			return nullptr;
		}

		bOutReplacedExisting = true;

		// Rebuild in place rather than deleting and recreating: every reference to this montage -
		// abilities, items, dialogue lines - stays pointing at it, which is the difference between
		// a rebuild and a rewire.
		Existing->SlotAnimTracks.Empty();
		Existing->CompositeSections.Empty();
		Existing->Notifies.Empty();

		FSlotAnimationTrack& Track = Existing->SlotAnimTracks.AddDefaulted_GetRef();
		Track.SlotName = Recipe->SlotName;

		FAnimSegment Segment;
		Segment.SetAnimReference(Sequence, true);
		Track.AnimTrack.AnimSegments.Add(Segment);

		Existing->SetCompositeLength(Sequence->GetPlayLength());
		UAnimMontageFactory::EnsureStartingSection(Existing);

		// UpdateCommonTargetFrameRate is private to UAnimMontage and reachable only from the factory,
		// so a rebuilt montage keeps the frame rate it was created with. Harmless while the
		// replacement sequence comes from the same pipeline at the same rate; if that ever stops
		// being true, delete and rebuild rather than reusing - at the cost of breaking references.

		return Existing;
	}

	UAnimMontageFactory* Factory = NewObject<UAnimMontageFactory>();
	Factory->SourceAnimation = Sequence;
	Factory->TargetSkeleton = Sequence->GetSkeleton();

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UAnimMontage* Montage = Cast<UAnimMontage>(AssetToolsModule.Get().CreateAsset(
		AssetName, FolderPath, UAnimMontage::StaticClass(), Factory));

	if (!Montage)
	{
		OutError = FString::Printf(TEXT("Could not create a montage at '%s'."), *ObjectPath);
		return nullptr;
	}

	// The factory always writes into the first slot track; name it after the recipe asked.
	if (Montage->SlotAnimTracks.Num() > 0)
	{
		Montage->SlotAnimTracks[0].SlotName = Recipe->SlotName;
	}

	return Montage;
}

int32 UMontageForgeSubsystem::ApplyNotifies(UAnimMontage* Montage, const UMontageRecipe* Recipe)
{
	Montage->Notifies.Empty();

	const float Length = Montage->GetPlayLength();

	// Sorted, because the notify track is read in order and an out-of-order array is confusing to
	// anyone opening the asset even though the engine copes.
	TArray<FMontageForgeNotifySpec> Sorted = Recipe->Notifies;
	Sorted.Sort([](const FMontageForgeNotifySpec& A, const FMontageForgeNotifySpec& B)
	{
		return A.Position < B.Position;
	});

	int32 Placed = 0;

	for (const FMontageForgeNotifySpec& Spec : Sorted)
	{
		if (!Spec.EventTag.IsValid())
		{
			UE_LOG(LogMontageForge, Warning,
				TEXT("'%s' has a notify with no tag - skipped. An untagged notify would fire into "
					 "nothing and look identical to one that was never placed."),
				*Recipe->GetName());
			continue;
		}

		UMontageForgeGameplayEventNotify* Notify =
			NewObject<UMontageForgeGameplayEventNotify>(Montage, NAME_None, RF_Transactional);
		Notify->EventTag = Spec.EventTag;
		Notify->EventMagnitude = Spec.Magnitude;

		FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
		Event.NotifyName = FName(*Spec.EventTag.ToString());
		Event.Notify = Notify;
		Event.NotifyStateClass = nullptr;

		// Link to the montage so the event moves with the segment if the montage is later edited,
		// rather than sitting at a fixed time the animation has drifted away from.
		Event.Link(Montage, FMath::Clamp(Spec.Position, 0.f, 1.f) * Length);
		Event.SetTime(FMath::Clamp(Spec.Position, 0.f, 1.f) * Length);

		++Placed;
	}

	Montage->RefreshCacheData();
	return Placed;
}

int32 UMontageForgeSubsystem::ApplyCurves(UAnimMontage* Montage, const UMontageRecipe* Recipe)
{
	const TArray<FMontageForgeCurveSpec> Curves = Recipe->GetResolvedCurves();
	if (Curves.Num() == 0)
	{
		return 0;
	}

	const float Length = Montage->GetPlayLength();

	IAnimationDataController& Controller = Montage->GetController();
	IAnimationDataController::FScopedBracket Bracket(Controller, NSLOCTEXT("MontageForge", "StampCurves", "Stamp layering curves"));

	int32 Stamped = 0;

	for (const FMontageForgeCurveSpec& Spec : Curves)
	{
		if (Spec.CurveName.IsNone())
		{
			continue;
		}

		const FAnimationCurveIdentifier CurveId(Spec.CurveName, ERawCurveTrackTypes::RCT_Float);

		// AddCurve fails when one of that name already exists, which on a rebuild is the normal case -
		// not an error, so the result is ignored and the keys below overwrite whatever was there.
		Controller.AddCurve(CurveId);

		// Two keys, flat. A layering curve is a statement about the whole clip - "this montage drives the
		// arms" - not something that varies through it, and the montage's own blend already eases the
		// pose in and out.
		TArray<FRichCurveKey> Keys;
		Keys.Add(FRichCurveKey(0.f, Spec.Value));
		Keys.Add(FRichCurveKey(Length, Spec.Value));

		if (Controller.SetCurveKeys(CurveId, Keys))
		{
			++Stamped;
		}
	}

	// Curve names have to exist on the skeleton to be readable at runtime. Without this a stamped curve
	// is present in the asset and invisible to the anim blueprint - the same silent nothing as before.
	if (USkeleton* Skeleton = Montage->GetSkeleton())
	{
		Controller.FindOrAddCurveNamesOnSkeleton(Skeleton, ERawCurveTrackTypes::RCT_Float);
	}

	return Stamped;
}

FMontageBuildResult UMontageForgeSubsystem::BuildFromRecipe(UMontageRecipe* Recipe, bool bOverwriteHandEdits)
{
	FMontageBuildResult Result;

	if (!Recipe)
	{
		Result.Error = TEXT("No recipe given.");
		return Result;
	}

	UAnimSequence* Sequence = Recipe->Sequence.LoadSynchronous();
	if (!Sequence)
	{
		Result.Error = FString::Printf(
			TEXT("'%s' has no Sequence set, so there is nothing to build a montage around."),
			*Recipe->GetName());
		return Result;
	}

	if (!Sequence->GetSkeleton())
	{
		Result.Error = FString::Printf(TEXT("'%s' is not bound to a skeleton."), *Sequence->GetName());
		return Result;
	}

	UAnimMontage* Montage = CreateMontageAsset(
		Recipe, Sequence, bOverwriteHandEdits, Result.bReplacedExisting, Result.Error);

	if (!Montage)
	{
		return Result;
	}

	// Applied after creation either way: the factory writes its own defaults and a rebuild keeps whatever
	// the montage already had, so neither path picks these up on its own.
	Montage->BlendIn.SetBlendTime(Recipe->BlendInTime);
	Montage->BlendIn.SetBlendOption(Recipe->BlendOption);
	Montage->BlendOut.SetBlendTime(Recipe->BlendOutTime);
	Montage->BlendOut.SetBlendOption(Recipe->BlendOption);

	Result.NotifiesPlaced = ApplyNotifies(Montage, Recipe);
	Result.CurvesStamped = ApplyCurves(Montage, Recipe);

	Montage->MarkPackageDirty();
	MontageForgePrivate::SaveAsset(Montage);

	Recipe->BuiltMontage = Montage;
	Recipe->MarkPackageDirty();
	MontageForgePrivate::SaveAsset(Recipe);

	Result.bSuccess = true;
	Result.MontagePath = Montage->GetPathName();

	UE_LOG(LogMontageForge, Log, TEXT("%s '%s' from '%s' - %d notify(s), %d curve(s), %.2fs."),
		Result.bReplacedExisting ? TEXT("Rebuilt") : TEXT("Built"),
		*Montage->GetName(), *Recipe->GetName(), Result.NotifiesPlaced, Result.CurvesStamped,
		Montage->GetPlayLength());

	return Result;
}

TArray<FMontageBuildResult> UMontageForgeSubsystem::BuildMany(
	const TArray<UMontageRecipe*>& Recipes, bool bOverwriteHandEdits)
{
	TArray<FMontageBuildResult> Results;
	Results.Reserve(Recipes.Num());

	for (UMontageRecipe* Recipe : Recipes)
	{
		Results.Add(BuildFromRecipe(Recipe, bOverwriteHandEdits));
	}

	return Results;
}

// -------------------------------------------------------------------------------------------------
// Capture
// -------------------------------------------------------------------------------------------------

int32 UMontageForgeSubsystem::CaptureTimingsFromMontage(UMontageRecipe* Recipe, FString& OutError)
{
	if (!Recipe)
	{
		OutError = TEXT("No recipe given.");
		return -1;
	}

	UAnimMontage* Montage = Recipe->BuiltMontage.LoadSynchronous();
	if (!Montage)
	{
		OutError = FString::Printf(
			TEXT("'%s' has no built montage to read from. Build it first."), *Recipe->GetName());
		return -1;
	}

	const float Length = Montage->GetPlayLength();
	if (Length <= 0.f)
	{
		OutError = FString::Printf(TEXT("'%s' has no length."), *Montage->GetName());
		return -1;
	}

	int32 Updated = 0;

	for (const FAnimNotifyEvent& Event : Montage->Notifies)
	{
		const UMontageForgeGameplayEventNotify* Notify =
			Cast<UMontageForgeGameplayEventNotify>(Event.Notify);

		if (!Notify || !Notify->EventTag.IsValid())
		{
			continue;
		}

		const float Position = FMath::Clamp(Event.GetTriggerTime() / Length, 0.f, 1.f);

		FMontageForgeNotifySpec* Existing = Recipe->Notifies.FindByPredicate(
			[&Notify](const FMontageForgeNotifySpec& Spec)
			{
				return Spec.EventTag == Notify->EventTag;
			});

		if (Existing)
		{
			Existing->Position = Position;
			Existing->Magnitude = Notify->EventMagnitude;
		}
		else
		{
			// Added by hand in the montage editor. Take it - the recipe is meant to describe what
			// the montage actually is, and refusing would make capture lossy.
			FMontageForgeNotifySpec Added;
			Added.EventTag = Notify->EventTag;
			Added.Position = Position;
			Added.Magnitude = Notify->EventMagnitude;
			Recipe->Notifies.Add(Added);
		}

		++Updated;
	}

	// Entries whose tag no longer appears are deliberately left alone. A notify removed on purpose
	// and one that was never placed are indistinguishable from here, and silently deleting authored
	// intent is the worse mistake.

	Recipe->MarkPackageDirty();
	MontageForgePrivate::SaveAsset(Recipe);

	UE_LOG(LogMontageForge, Log, TEXT("Captured %d timing(s) from '%s' into '%s'."),
		Updated, *Montage->GetName(), *Recipe->GetName());

	OutError.Reset();
	return Updated;
}

TArray<FString> UMontageForgeSubsystem::FindRecipes() const
{
	TArray<FString> Paths;

	const FAssetRegistryModule& Registry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FAssetData> Assets;
	Registry.Get().GetAssetsByClass(
		UMontageRecipe::StaticClass()->GetClassPathName(), Assets, /*bSearchSubClasses*/ true);

	for (const FAssetData& Asset : Assets)
	{
		Paths.Add(Asset.GetSoftObjectPath().ToString());
	}

	return Paths;
}
