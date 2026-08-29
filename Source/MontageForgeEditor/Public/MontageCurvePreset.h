// A named set of layering curves, authored once and reused.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MontageRecipe.h"
#include "MontageCurvePreset.generated.h"

/**
 * A body mask, expressed as the curves that produce it.
 *
 * "Upper body" is not a concept this plugin can own: which curves an anim blueprint reads, and what they
 * are called, differ per rig. Narrative Pro's biped uses LayerArmLeft, LayerArmRight, LayerSpine and
 * friends; another project will use something else entirely, and stock Game Animation Sample calls them
 * something else again.
 *
 * So the mask lives here, as data you author by copying from a montage that already works. Recipes point
 * at it, and adding a new mask is a new asset rather than a code change.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Montage Curve Preset"))
class MONTAGEFORGEEDITOR_API UMontageCurvePreset : public UDataAsset
{
	GENERATED_BODY()

public:

	/** What this mask is for, in words - it is the only thing that will make sense a year from now. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset", meta = (MultiLine = true))
	FString Description;

	/**
	 * The curves and their values.
	 *
	 * Include the parts you want at zero as well as the ones you want at one. Leaving a curve out is not
	 * the same as setting it to zero: an absent curve leaves whatever the anim blueprint was already doing
	 * in place, which is rarely what a mask means.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	TArray<FMontageForgeCurveSpec> Curves;
};
