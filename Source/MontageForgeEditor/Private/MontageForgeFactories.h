// Copyright Blackcode SA. All rights reserved.
//
// What makes these types appear under right-click > Automation Forge in the Content Browser.
// The asset definitions beside this decide colour and category; a factory is what decides the
// thing can be made there in the first place.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MontageForgeFactories.generated.h"

/** Creates a Montage Recipe - the durable description a montage is built from, and rebuilt from. */
UCLASS()
class UMontageRecipeFactory : public UFactory
{
	GENERATED_BODY()

public:

	UMontageRecipeFactory();

	virtual UObject* FactoryCreateNew(
		UClass* Class,
		UObject* InParent,
		FName Name,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn) override;

	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
};

/** Creates a Curve Preset - a body mask, as the layering curves that produce it. */
UCLASS()
class UMontageCurvePresetFactory : public UFactory
{
	GENERATED_BODY()

public:

	UMontageCurvePresetFactory();

	virtual UObject* FactoryCreateNew(
		UClass* Class,
		UObject* InParent,
		FName Name,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn) override;

	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
};
