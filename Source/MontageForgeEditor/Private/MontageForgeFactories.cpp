// Copyright Blackcode SA. All rights reserved.

#include "MontageForgeFactories.h"

#include "MontageCurvePreset.h"
#include "MontageRecipe.h"

#define LOCTEXT_NAMESPACE "MontageForgeEditor"

// -------------------------------------------------------------------------------------------------

UMontageRecipeFactory::UMontageRecipeFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMontageRecipe::StaticClass();
}

UObject* UMontageRecipeFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject*, FFeedbackContext*)
{
	return NewObject<UMontageRecipe>(InParent, Class, Name, Flags);
}

FText UMontageRecipeFactory::GetDisplayName() const
{
	return LOCTEXT("NewMontageRecipe", "Montage Recipe");
}

FString UMontageRecipeFactory::GetDefaultNewAssetName() const
{
	// The prefix the builder uses, so a new one sorts with its neighbours rather than under N for
	// NewDataAsset.
	return TEXT("MR_NewRecipe");
}

// -------------------------------------------------------------------------------------------------

UMontageCurvePresetFactory::UMontageCurvePresetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMontageCurvePreset::StaticClass();
}

UObject* UMontageCurvePresetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject*, FFeedbackContext*)
{
	return NewObject<UMontageCurvePreset>(InParent, Class, Name, Flags);
}

FText UMontageCurvePresetFactory::GetDisplayName() const
{
	return LOCTEXT("NewMontageCurvePreset", "Curve Preset");
}

FString UMontageCurvePresetFactory::GetDefaultNewAssetName() const
{
	return TEXT("MCP_NewPreset");
}

#undef LOCTEXT_NAMESPACE
