// Copyright Blackcode SA. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"

#include "MontageForgeAssetDefinitions.generated.h"

/**
 * Where MontageForge's assets live in the Content Browser.
 *
 * Every data asset in the Automation Forge family gets one of these plus a factory - the family
 * convention, no exceptions. Both types keep the default editor: a recipe and
 * a curve preset are fields, and a details panel is the right surface for fields.
 */
UCLASS()
class UMontageRecipeAssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:

	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
};

UCLASS()
class UMontageCurvePresetAssetDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:

	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
};
