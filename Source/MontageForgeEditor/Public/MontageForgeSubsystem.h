// Building montages from recipes, and reading hand edits back.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "MontageForgeSubsystem.generated.h"

class UAnimMontage;
class UAnimSequence;
class UMontageRecipe;

/** What came back from building one recipe. */
USTRUCT(BlueprintType)
struct MONTAGEFORGEEDITOR_API FMontageBuildResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Build")
	bool bSuccess = false;

	/** Content path of the montage, whether newly built or rebuilt. */
	UPROPERTY(BlueprintReadOnly, Category = "Build")
	FString MontagePath;

	/** How many notifies were placed. */
	UPROPERTY(BlueprintReadOnly, Category = "Build")
	int32 NotifiesPlaced = 0;

	/** How many layering curves were stamped. Zero on a montage that needs no body mask. */
	UPROPERTY(BlueprintReadOnly, Category = "Build")
	int32 CurvesStamped = 0;

	/** True when an existing montage was replaced rather than a new one created. */
	UPROPERTY(BlueprintReadOnly, Category = "Build")
	bool bReplacedExisting = false;

	/** Why it failed, when it did. */
	UPROPERTY(BlueprintReadOnly, Category = "Build")
	FString Error;
};

/**
 * Turns recipes into montages.
 *
 * Everything here is deliberate and one-shot. Nothing rebuilds on save, nothing watches for changes,
 * nothing keeps a montage "in sync" with its recipe - because the expected life of a montage is
 * build once, hand-tune, ship, and an automatic rebuild would quietly undo the tuning that is the
 * whole point of the workflow.
 *
 * The plugin knows nothing about what plays these montages. Assigning a built montage to an ability,
 * an item or a dialogue line is left to whoever knows what those are.
 */
UCLASS()
class MONTAGEFORGEEDITOR_API UMontageForgeSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:

	static UMontageForgeSubsystem* Get();

	/**
	 * Build the montage a recipe describes.
	 *
	 * Creates the asset, adds the animation as its only segment in the requested slot, and places a
	 * notify per entry. Rebuilding an existing montage discards whatever was done to it by hand -
	 * see bOverwriteHandEdits.
	 *
	 * @param bOverwriteHandEdits Required to replace a montage that already exists. Refusing by
	 *        default is the point: the normal workflow tunes a montage after building it, so
	 *        clobbering one silently would throw away the only work that was not reproducible.
	 */
	UFUNCTION(BlueprintCallable, Category = "MontageForge")
	FMontageBuildResult BuildFromRecipe(UMontageRecipe* Recipe, bool bOverwriteHandEdits);

	/** Build several. Each is independent; one failure does not stop the rest. */
	UFUNCTION(BlueprintCallable, Category = "MontageForge")
	TArray<FMontageBuildResult> BuildMany(const TArray<UMontageRecipe*>& Recipes, bool bOverwriteHandEdits);

	/**
	 * Read a montage's current notify times back into its recipe.
	 *
	 * The other half of the workflow. Tune the timing by hand, then capture it so the next rebuild
	 * starts from what you decided rather than from the original guess. Optional by design - most
	 * clips never need it.
	 *
	 * Matches notifies to recipe entries by tag. Tags present in the montage but not the recipe are
	 * added; entries whose tag no longer appears are left alone rather than deleted, because a
	 * notify deliberately removed and a notify never placed look identical from here.
	 *
	 * @return how many entries were updated, or -1 on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "MontageForge")
	int32 CaptureTimingsFromMontage(UMontageRecipe* Recipe, FString& OutError);

	/** Every montage recipe in the project. */
	UFUNCTION(BlueprintCallable, Category = "MontageForge")
	TArray<FString> FindRecipes() const;

private:

	/** Create or replace the montage asset itself, without notifies. */
	UAnimMontage* CreateMontageAsset(
		UMontageRecipe* Recipe,
		UAnimSequence* Sequence,
		bool bOverwriteHandEdits,
		bool& bOutReplacedExisting,
		FString& OutError);

	/** Replace every notify on a montage with the recipe's, sorted by position. */
	int32 ApplyNotifies(UAnimMontage* Montage, const UMontageRecipe* Recipe);

	/** Stamp the recipe's layering curves flat across the montage. */
	int32 ApplyCurves(UAnimMontage* Montage, const UMontageRecipe* Recipe);
};
