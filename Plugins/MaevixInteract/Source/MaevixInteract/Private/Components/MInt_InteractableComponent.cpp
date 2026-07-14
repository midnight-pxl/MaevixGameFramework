// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Components/MInt_InteractableComponent.h"

#include "Engine/CollisionProfile.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMaevixInteract.h"
#include "Settings/MInt_DeveloperSettings.h"

namespace
{
	/** True when Channel is a project-defined game Object channel, not a built-in and not a Trace channel. */
	bool IsValidObjectChannel(ECollisionChannel Channel)
	{
		return Channel >= ECC_GameTraceChannel1
			&& Channel <= ECC_GameTraceChannel18
			&& UCollisionProfile::Get()->ConvertToObjectType(Channel) != ObjectTypeQuery_MAX;
	}

	/** Configured display name of Channel, or its raw enum name when unnamed. For log messages. */
	FString ChannelDisplayName(ECollisionChannel Channel)
	{
		return StaticEnum<ECollisionChannel>()->GetDisplayNameTextByValue(static_cast<int64>(Channel)).ToString();
	}
}

UMInt_InteractableComponent::UMInt_InteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	InitSphereRadius(DetectionRadius);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetGenerateOverlapEvents(true);
}

void UMInt_InteractableComponent::OnRegister()
{
	Super::OnRegister();

	// CDO/archetype has no owner to name and never runs detection; only real instances configure collision.
	if (IsTemplate())
	{
		return;
	}

	const UMInt_DeveloperSettings* Settings = UMInt_DeveloperSettings::Get();
	const ECollisionChannel InteractableChannel = Settings->InteractableObjectChannel.GetValue();
	const ECollisionChannel InteractorChannel = Settings->InteractorObjectChannel.GetValue();

	if (!IsValidObjectChannel(InteractableChannel) || !IsValidObjectChannel(InteractorChannel))
	{
		UE_LOG(LogMaevixInteract, Error,
			TEXT("UMInt_InteractableComponent on actor '%s' has invalid interaction channels (Interactable: '%s', Interactor: '%s'). ")
			TEXT("Create custom Object channels under Project Settings > Engine > Collision, then set both under ")
			TEXT("Project Settings > Game > Maevix Interact. Detection is disabled until configured."),
			*GetNameSafe(GetOwner()), *ChannelDisplayName(InteractableChannel), *ChannelDisplayName(InteractorChannel));
		return;
	}

	// The two channels must differ. If the interactable's object type equalled the channel it responds
	// Overlap to, every interactable sphere would overlap every other interactable sphere: in a densely
	// placed loot room that is a quadratic number of overlap pairs the physics scene tracks and dispatches
	// events for, all doing nothing. Two distinct channels is the whole reason for this model over a single
	// shared one, and a settings typo must not be able to collapse it back.
	if (InteractableChannel == InteractorChannel)
	{
		UE_LOG(LogMaevixInteract, Error,
			TEXT("UMInt_InteractableComponent on actor '%s' has Interactable and Interactor Object Channels both set to '%s'. ")
			TEXT("They must be two distinct channels. Set them under Project Settings > Game > Maevix Interact. ")
			TEXT("Detection is disabled until configured."),
			*GetNameSafe(GetOwner()), *ChannelDisplayName(InteractableChannel));
		return;
	}

	SetCollisionObjectType(InteractableChannel);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(InteractorChannel, ECR_Overlap);
	// MANDATORY: ignore every other channel, the P2b interaction trace channel included. This is a ~150cm
	// volume that passes through walls; if it blocked or was traceable, the camera trace would hit the
	// sphere instead of the interactable's mesh and players could focus interactables through geometry.
	// Do not "fix" this by making the sphere Block or Overlap anything besides the Interactor channel.
}

bool UMInt_InteractableComponent::CanInteract_Implementation(const FMCore_InteractionContext& Context) const
{
	if (const AActor* Owner = GetOwner(); Owner && Owner->Implements<UMCore_Interactable>())
	{
		return IMCore_Interactable::Execute_CanInteract(Owner, Context);
	}
	return true;
}

void UMInt_InteractableComponent::OnInteract_Implementation(const FMCore_InteractionContext& Context)
{
	if (AActor* Owner = GetOwner(); Owner && Owner->Implements<UMCore_Interactable>())
	{
		IMCore_Interactable::Execute_OnInteract(Owner, Context);
	}
	// Authored default: no-op. The game supplies commit behavior via the owner's interface override.
}

FMCore_InteractionData UMInt_InteractableComponent::GetInteractionData_Implementation(const FMCore_InteractionContext& Context) const
{
	if (const AActor* Owner = GetOwner(); Owner && Owner->Implements<UMCore_Interactable>())
	{
		return IMCore_Interactable::Execute_GetInteractionData(Owner, Context);
	}

	FMCore_InteractionData Data;
	Data.PromptText = PromptText;
	Data.InteractionTypeTag = InteractionTypeTag;
	return Data;
}

int32 UMInt_InteractableComponent::GetInteractionStateVersion_Implementation() const
{
	if (const AActor* Owner = GetOwner(); Owner && Owner->Implements<UMCore_Interactable>())
	{
		return IMCore_Interactable::Execute_GetInteractionStateVersion(Owner);
	}
	return 0;
}

FMInt_ExecutionConfig UMInt_InteractableComponent::GetExecutionConfig_Implementation() const
{
	if (const AActor* Owner = GetOwner(); Owner && Owner->Implements<UMInt_ExecutionConfigProvider>())
	{
		return IMInt_ExecutionConfigProvider::Execute_GetExecutionConfig(Owner);
	}
	return ExecutionConfig;
}

#if WITH_EDITOR
void UMInt_InteractableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMInt_InteractableComponent, DetectionRadius))
	{
		SetSphereRadius(DetectionRadius);
	}
}
#endif
