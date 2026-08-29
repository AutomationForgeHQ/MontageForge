// What a montage should be, written down.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AlphaBlend.h"
#include "GameplayTagContainer.h"
#include "MontageRecipe.generated.h"

class UAnimSequence;
class UAnimMontage;
class UMontageCurvePreset;

/**
 * One named float curve stamped across the montage.
 *
 * Layered playback works by the *animation* telling the anim blueprint which body parts it wants. A clip
 * that carries no curves gets zero weight on every layered slot: it plays, its notifies fire, and it
 * animates nothing at all, with no warning anywhere. These curves are that instruction.
 */
USTRUCT(BlueprintType)
struct MONTAGEFORGEEDITOR_API FMontageForgeCurveSpec
{
	GENERATED_BODY()

	/**
	 * Curve name, exactly as the anim blueprint reads it.
	 *
	 * Spelling is unforgiving and failure is silent - a misnamed curve is indistinguishable from one that
	 * was never authored. Copy them from a montage that already works rather than from memory.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	FName CurveName;

	/** Held flat for the whole montage. Typically 1 for a body part this clip drives, 0 for one it leaves alone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve")
	float Value = 1.f;
};

/** Where a notify sits, and what it announces. */
USTRUCT(BlueprintType)
struct MONTAGEFORGEEDITOR_API FMontageForgeNotifySpec
{
	GENERATED_BODY()

	/** The event to send. This is the contract with whatever plays the montage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	FGameplayTag EventTag;

	/**
	 * Where in the clip it fires, as a fraction from 0 to 1.
	 *
	 * Proportional rather than absolute seconds, because the clip underneath will be regenerated and
	 * will come back a different length. "Sixty percent through" survives that; "at 2.4 seconds"
	 * quietly stops meaning what it meant.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify",
		meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0))
	float Position = 0.5f;

	/** Payload magnitude, for events that carry a number. Ignore where it means nothing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	float Magnitude = 0.f;
};

/**
 * The authored intent behind one montage: which animation, and what fires when.
 *
 * **This asset is the source of truth, not the montage it builds.** The working habit is build once,
 * nudge the timing by hand in the montage editor, and ship it - so a built montage is expected to
 * drift from its recipe and that is fine. Rebuilding is a deliberate act that says "throw my edits
 * away and start again", and capturing is the opposite: it reads the hand-tuned timings back into
 * here, for the clips worth making reproducible.
 *
 * Nothing in here knows what a tag means or what will play the result. Assign the finished montage
 * to whatever consumes it yourself - that step is one action, and coupling this tool to any
 * particular game to save it would be a bad trade.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Montage Recipe"))
class MONTAGEFORGEEDITOR_API UMontageRecipe : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The animation the montage plays. Usually something MotionForge imported. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source")
	TSoftObjectPtr<UAnimSequence> Sequence;

	/**
	 * Montage slot to play in.
	 *
	 * DefaultSlot suits full-body actions. Projects with an upper-body slot in their AnimBP will
	 * want that one for anything that should play while walking.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Source")
	FName SlotName = TEXT("DefaultSlot");

	/**
	 * How long the montage takes to blend in, in seconds.
	 *
	 * Costs real animation: a quarter second on a four second clip is the first six percent of the action,
	 * so a sharp movement usually wants less than the engine's 0.25 default and a slow deliberate one can
	 * afford more. Zero snaps straight in.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending", meta = (ClampMin = 0.0, Units = "Seconds"))
	float BlendInTime = 0.25f;

	/** How long the montage takes to blend back out. Same trade as blending in, at the other end. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending", meta = (ClampMin = 0.0, Units = "Seconds"))
	float BlendOutTime = 0.25f;

	/**
	 * Curve shape for both blends.
	 *
	 * Linear by default rather than the montage factory's Hermite Cubic, because it is what hand-authored
	 * montages overwhelmingly use - 48 of 60 sampled in Narrative Pro - and a generated clip that blends
	 * differently from everything around it reads as wrong without anyone being able to say why.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blending")
	EAlphaBlendOption BlendOption = EAlphaBlendOption::Linear;

	/**
	 * A named set of layering curves to stamp, so a body mask is chosen once and reused.
	 *
	 * Which curves mean "upper body" is a convention of the project's anim blueprint, not of this plugin,
	 * so it lives in an asset you author rather than in an enum here - that way a rig with a different
	 * set, or a mask nobody anticipated, needs no code change.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layering")
	TSoftObjectPtr<UMontageCurvePreset> CurvePreset;

	/**
	 * Curves stamped on top of the preset, for one-off adjustments.
	 *
	 * A name appearing in both wins here, so a recipe can override a single value without needing its own
	 * copy of the whole preset.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layering")
	TArray<FMontageForgeCurveSpec> ExtraCurves;

	/** Every curve this recipe stamps: the preset, with ExtraCurves layered over it. */
	UFUNCTION(BlueprintPure, Category = "Montage Recipe")
	TArray<FMontageForgeCurveSpec> GetResolvedCurves() const;

	/** Events fired during the montage, in whatever order; they are sorted by position on build. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notifies")
	TArray<FMontageForgeNotifySpec> Notifies;

	/**
	 * Where the montage is written. Empty builds it beside this recipe.
	 *
	 * The built asset is named AM_<recipe name minus its MR_ prefix>.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	FString OutputPath;

	/**
	 * The montage last built from this recipe.
	 *
	 * Written by the builder, so rebuild and capture know what they are acting on, and so a recipe
	 * can be traced to its result without searching.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	TSoftObjectPtr<UAnimMontage> BuiltMontage;

	/** The name the built montage should have, derived from this recipe's own. */
	UFUNCTION(BlueprintPure, Category = "Montage Recipe")
	FString GetMontageAssetName() const;
};
