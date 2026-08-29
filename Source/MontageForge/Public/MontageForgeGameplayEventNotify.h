// The notify that tells an ability "now".

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "MontageForgeGameplayEventNotify.generated.h"

/**
 * Sends a gameplay event to the animating actor at a point in a montage.
 *
 * This exists because nothing else does. GameplayAbilities ships UAnimNotify_GameplayCue, which
 * fires cosmetic cues, not events - so an ability that waits on a gameplay event has no stock way of
 * being told when the moment arrived, and every project ends up writing this class. It is the whole
 * reason abilities can be driven by animation rather than by a timer.
 *
 * The tag is the contract between an animation and whatever is listening. Nothing here knows or
 * cares what it means; that is the point.
 */
UCLASS(DisplayName = "Send Gameplay Event", meta = (ToolTip = "Sends a gameplay event to the owning actor."))
class MONTAGEFORGE_API UMontageForgeGameplayEventNotify : public UAnimNotify
{
	GENERATED_BODY()

public:

	UMontageForgeGameplayEventNotify();

	/**
	 * The event to send.
	 *
	 * Whatever is waiting decides what it means - an item being consumed, a hand reaching contact, a
	 * projectile leaving. Match it exactly to the tag the ability waits on; a typo here fails
	 * silently, because an event nobody listens for looks identical to one that was never sent.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Event")
	FGameplayTag EventTag;

	/**
	 * Payload magnitude, for events that carry a number.
	 *
	 * Arrives as EventMagnitude on the receiving side. Ignore it where it means nothing.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Event")
	float EventMagnitude = 0.f;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override { return true; }
#endif
};
