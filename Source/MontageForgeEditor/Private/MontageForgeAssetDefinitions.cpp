// Copyright Blackcode SA. All rights reserved.

#include "MontageForgeAssetDefinitions.h"

#include "MontageCurvePreset.h"
#include "MontageRecipe.h"

#define LOCTEXT_NAMESPACE "MontageForgeEditor"

namespace
{
	/** One category for the whole family, with a submenu per set, so the types sit together. */
	const TArray<FAssetCategoryPath>& ForgeCategories()
	{
		static const TArray<FAssetCategoryPath> Categories
		{
			FAssetCategoryPath(
				FAssetCategoryPath(LOCTEXT("AutomationForge", "Automation Forge")),
				LOCTEXT("MontageForge", "MontageForge"))
		};
		return Categories;
	}
}

// -------------------------------------------------------------------------------------------------

FText UMontageRecipeAssetDefinition::GetAssetDisplayName() const
{
	return LOCTEXT("MontageRecipe", "Montage Recipe");
}

FLinearColor UMontageRecipeAssetDefinition::GetAssetColor() const
{
	// Magenta for the MontageForge set, apart from MotionForge's blue and teal next door.
	return FLinearColor(0.82f, 0.36f, 0.62f);
}

TSoftClassPtr<UObject> UMontageRecipeAssetDefinition::GetAssetClass() const
{
	return UMontageRecipe::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UMontageRecipeAssetDefinition::GetAssetCategories() const
{
	return ForgeCategories();
}

// -------------------------------------------------------------------------------------------------

FText UMontageCurvePresetAssetDefinition::GetAssetDisplayName() const
{
	return LOCTEXT("MontageCurvePreset", "Curve Preset");
}

FLinearColor UMontageCurvePresetAssetDefinition::GetAssetColor() const
{
	return FLinearColor(0.64f, 0.28f, 0.48f);
}

TSoftClassPtr<UObject> UMontageCurvePresetAssetDefinition::GetAssetClass() const
{
	return UMontageCurvePreset::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UMontageCurvePresetAssetDefinition::GetAssetCategories() const
{
	return ForgeCategories();
}

#undef LOCTEXT_NAMESPACE
