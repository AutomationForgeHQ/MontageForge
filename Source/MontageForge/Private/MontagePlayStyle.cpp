#include "MontagePlayStyle.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "MontageForge.h"

bool UMontagePlayStyleStatics::SetMovementBlocked(APawn* Pawn, const bool bBlocked)
{
	if (!IsValid(Pawn))
	{
		return false;
	}

	APlayerController* Controller = Cast<APlayerController>(Pawn->GetController());
	if (!Controller)
	{
		// An NPC is steered by its AI, not by input, so there is nothing to take away. Reported as
		// "did nothing" so the caller does not record a release it must not make.
		return false;
	}

	Controller->SetIgnoreMoveInput(bBlocked);

	UE_LOG(LogMontageForge, Verbose, TEXT("%s movement input for %s"),
		bBlocked ? TEXT("Blocked") : TEXT("Released"), *Pawn->GetName());

	return true;
}
