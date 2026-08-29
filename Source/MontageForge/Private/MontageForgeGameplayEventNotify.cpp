#include "MontageForgeGameplayEventNotify.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "MontageForge.h"

UMontageForgeGameplayEventNotify::UMontageForgeGameplayEventNotify()
{
#if WITH_EDITORONLY_DATA
	// Distinct from the notify colours the engine ships, so a gameplay-critical marker is not lost
	// among decorative ones on a busy track.
	NotifyColor = FColor(220, 120, 40);
#endif
}

void UMontageForgeGameplayEventNotify::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (!EventTag.IsValid())
	{
		// Worth a warning rather than a silent return: an untagged notify looks identical at runtime
		// to one that fired correctly and was ignored, and that is a miserable thing to debug.
		UE_LOG(LogMontageForge, Warning,
			TEXT("Send Gameplay Event notify on '%s' has no tag set, so nothing was sent."),
			Animation ? *Animation->GetName() : TEXT("unknown animation"));
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = Owner;
	Payload.Target = Owner;
	Payload.EventMagnitude = EventMagnitude;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);

	UE_LOG(LogMontageForge, Verbose, TEXT("Sent %s to %s"), *EventTag.ToString(), *Owner->GetName());
}

FString UMontageForgeGameplayEventNotify::GetNotifyName_Implementation() const
{
	// Show the tag on the track. A row of identically named notifies is useless at a glance, and the
	// tag is the only thing that distinguishes them.
	return EventTag.IsValid()
		? EventTag.ToString()
		: TEXT("Send Gameplay Event (no tag)");
}
