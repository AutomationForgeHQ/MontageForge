// How a montage should coexist with whatever the character was already doing.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MontagePlayStyle.generated.h"

class APawn;

/**
 * How a montage shares the body with locomotion.
 *
 * A montage played on a full-body slot while the character keeps walking looks broken - the animation
 * takes the whole skeleton and the walk cycle never runs, so a character slides across the floor in a
 * standing pose. There are only two honest answers to that, and which one is right depends on the clip
 * rather than on the system playing it: stop the character, or stop taking the whole body.
 */
UENUM(BlueprintType)
enum class EMontagePlayStyle : uint8
{
	/**
	 * Play it and leave movement alone.
	 *
	 * Correct only for a montage that does not fight locomotion - one on a layered slot, or an additive.
	 * On a full-body slot this is what produces the sliding.
	 */
	Free			UMETA(DisplayName = "Free"),

	/**
	 * Take the whole body and stop the character moving for the duration.
	 *
	 * Movement input is ignored rather than the character being frozen, so they decelerate into the
	 * animation instead of stopping dead mid-stride. Looking around still works, because taking the
	 * camera away feels like a lockup rather than a deliberate action.
	 *
	 * The right choice for anything the character plants their feet for: injecting themselves, working a
	 * panel, pulling a lever.
	 */
	BlockMovement	UMETA(DisplayName = "Block Movement")
};

/**
 * Applying a play style to a pawn.
 *
 * Deliberately knows nothing beyond APawn and its controller - blocking movement input during an
 * animation is an engine-level idea, not a game-specific one, so anything that plays a montage can use
 * this without inheriting a framework.
 */
UCLASS()
class MONTAGEFORGE_API UMontagePlayStyleStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Start or stop ignoring movement input on a pawn's player controller.
	 *
	 * **Calls must be balanced.** SetIgnoreMoveInput is a counter, not a flag: two blocks and one release
	 * leave the player permanently unable to move, which is the worst failure this can have and is
	 * invisible until someone tries to walk. Whoever blocks is responsible for releasing on every exit
	 * path, including the ones that are not supposed to happen.
	 *
	 * A no-op on anything without a player controller. An NPC playing the same montage is steered by its
	 * AI rather than by input, so there is nothing here to take away.
	 *
	 * @return true when the call actually did something, so a caller can record whether it owes a release.
	 */
	UFUNCTION(BlueprintCallable, Category = "MontageForge|Play Style")
	static bool SetMovementBlocked(APawn* Pawn, const bool bBlocked);
};
