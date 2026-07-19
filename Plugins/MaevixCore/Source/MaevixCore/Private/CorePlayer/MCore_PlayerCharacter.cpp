// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CorePlayer/MCore_PlayerCharacter.h"

#include "CoreData/Logging/LogMaevixPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

AMCore_PlayerCharacter::AMCore_PlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	// First person arms: only the owning client sees this mesh.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCamera->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
	FirstPersonCamera->bEnableFirstPersonScale = true;
	FirstPersonCamera->FirstPersonFieldOfView = 70.0f;
	FirstPersonCamera->FirstPersonScale = 0.6f;

	// Full body: hidden from the owning client, rendered for others and in shadows.
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AMCore_PlayerCharacter::NotifyControllerChanged()
{
	// Super broadcasts ReceiveControllerChanged; BP subclasses depend on it firing first.
	Super::NotifyControllerChanged();

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!InputSubsystem)
	{
		return;
	}

	for (const TObjectPtr<UInputMappingContext>& InputContext : InputContexts)
	{
		if (InputContext)
		{
			InputSubsystem->AddMappingContext(InputContext, 1);
		}
	}
}

void AMCore_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogMaevixPlayer, Error,
			TEXT("PlayerCharacter::SetupPlayerInputComponent: '%s' has no UEnhancedInputComponent. ")
			TEXT("MaevixCore requires Enhanced Input; set it as the Default Input Component in Project Settings > Input."),
			*GetNameSafe(this));
		return;
	}

	// Actions are assigned on a Blueprint subclass and are null in C++ defaults; bind only what exists.
	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMCore_PlayerCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMCore_PlayerCharacter::DoJumpEnd);
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMCore_PlayerCharacter::MoveInput);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMCore_PlayerCharacter::LookInput);
	}

	if (LookMouseAction)
	{
		EnhancedInputComponent->BindAction(LookMouseAction, ETriggerEvent::Triggered, this, &AMCore_PlayerCharacter::LookInput);
	}
}

void AMCore_PlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMCore_PlayerCharacter::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AMCore_PlayerCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMCore_PlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AMCore_PlayerCharacter::DoJumpStart()
{
	Jump();
}

void AMCore_PlayerCharacter::DoJumpEnd()
{
	StopJumping();
}
