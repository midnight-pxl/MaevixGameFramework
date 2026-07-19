// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MCore_PlayerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Abstract first person character. Maevix base for player-controlled first person pawns.
 *
 * Enhanced Input driven: assign the Input Actions and Input Mapping Contexts on a Blueprint
 * subclass. Contexts add for the possessing local player on NotifyControllerChanged at priority 1.
 */
UCLASS(abstract)
class MAEVIXCORE_API AMCore_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** First person camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

public:
	AMCore_PlayerCharacter();

	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

protected:
	// ============================================================================
	// INPUT
	// ============================================================================

	/** Input Mapping Context(s) added for the possessing local player. Leave empty to add none. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> InputContexts;

	/** Move Input Action (Axis2D). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action (Axis2D). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** Mouse Look Input Action (Axis2D). Shares the look handler with LookAction. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookMouseAction;

	/** Jump Input Action (Digital). */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
