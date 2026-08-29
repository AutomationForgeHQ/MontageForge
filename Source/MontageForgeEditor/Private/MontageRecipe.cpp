#include "MontageRecipe.h"

#include "MontageCurvePreset.h"

FString UMontageRecipe::GetMontageAssetName() const
{
	FString Name = GetName();

	// MR_PressPanel -> AM_PressPanel. Recipes and montages sit side by side in the browser, so the
	// prefix is what tells them apart at a glance.
	if (Name.StartsWith(TEXT("MR_")))
	{
		Name.RightChopInline(3);
	}

	return TEXT("AM_") + Name;
}

TArray<FMontageForgeCurveSpec> UMontageRecipe::GetResolvedCurves() const
{
	TArray<FMontageForgeCurveSpec> Resolved;

	if (const UMontageCurvePreset* Preset = CurvePreset.LoadSynchronous())
	{
		Resolved = Preset->Curves;
	}

	// Later wins, so a recipe can override one value from a shared preset without copying the whole thing.
	for (const FMontageForgeCurveSpec& Extra : ExtraCurves)
	{
		if (FMontageForgeCurveSpec* Existing = Resolved.FindByPredicate(
			[&Extra](const FMontageForgeCurveSpec& Spec) { return Spec.CurveName == Extra.CurveName; }))
		{
			Existing->Value = Extra.Value;
		}
		else
		{
			Resolved.Add(Extra);
		}
	}

	return Resolved;
}
