// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Components/MInt_InteractableComponent.h"

#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMaevixInteract.h"
#include "MInt_CollisionChannels.h"
#include "Settings/MInt_DeveloperSettings.h"

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

	/** Has no owner and never runs detection; only real instances configure collision. */
	if (IsTemplate())
	{
		return;
	}

	const UMInt_DeveloperSettings* Settings = UMInt_DeveloperSettings::Get();
	const ECollisionChannel InteractableChannel = Settings->InteractableObjectChannel.GetValue();
	const ECollisionChannel InteractorChannel = Settings->InteractorObjectChannel.GetValue();

	if (!MInt_IsValidObjectChannel(InteractableChannel) || !MInt_IsValidObjectChannel(InteractorChannel))
	{
		UE_LOG(LogMaevixInteract, Error,
			TEXT("UMInt_InteractableComponent on actor '%s' has invalid interaction channels (Interactable: '%s', Interactor: '%s'). ")
			TEXT("Create custom Object channels under Project Settings > Engine > Collision, then set both under ")
			TEXT("Project Settings > Game > Maevix Interact. Detection is disabled until configured."),
			*GetNameSafe(GetOwner()), *MInt_ChannelDisplayName(InteractableChannel), *MInt_ChannelDisplayName(InteractorChannel));
		return;
	}

	/** Channels for Interactable and Interactor must differ */ 
	if (InteractableChannel == InteractorChannel)
	{
		UE_LOG(LogMaevixInteract, Error,
			TEXT("UMInt_InteractableComponent on actor '%s' has Interactable and Interactor Object Channels both set to '%s'. ")
			TEXT("They must be two distinct channels. Set them under Project Settings > Game > Maevix Interact. ")
			TEXT("Detection is disabled until configured."),
			*GetNameSafe(GetOwner()), *MInt_ChannelDisplayName(InteractableChannel));
		return;
	}

	SetCollisionObjectType(InteractableChannel);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(InteractorChannel, ECR_Overlap);
	/**
	 * MANDATORY: ignore every other channel. Do not "fix" this by making the sphere
	 * Block or Overlap anything besides the Interactor channel.
	 */

	UE_LOG(LogMaevixInteract, Verbose,
		TEXT("UMInt_InteractableComponent::OnRegister: interactable registered: Owner='%s', InteractableChannel='%s'"),
		*GetNameSafe(GetOwner()), *MInt_ChannelDisplayName(InteractableChannel));
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
	/** NOP. The game supplies commit behavior via the owner's interface override. */
}

FMCore_InteractionData UMInt_InteractableComponent::GetInteractionData_Implementation(const FMCore_InteractionContext& Context) const
{
	/**
	 * Component-authored props are the base; an owner that implements IMCore_Interactable overrides only
	 * the fields it fills (non-empty PromptText, valid InteractionTypeTag), the rest fall back here. Guards
	 * an owner that adopts the empty interface default which would otherwise blank the authored prompt.
	 */
	FMCore_InteractionData Data;
	Data.PromptText = PromptText;
	Data.InteractionTypeTag = InteractionTypeTag;

	if (const AActor* Owner = GetOwner(); Owner && Owner->Implements<UMCore_Interactable>())
	{
		const FMCore_InteractionData OwnerData = IMCore_Interactable::Execute_GetInteractionData(Owner, Context);
		if (!OwnerData.PromptText.IsEmpty())
		{
			Data.PromptText = OwnerData.PromptText;
		}
		if (OwnerData.InteractionTypeTag.IsValid())
		{
			Data.InteractionTypeTag = OwnerData.InteractionTypeTag;
		}
	}

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
	/**
	 * Owner override (when it implements IMInt_ExecutionConfigProvider) supplies the whole config, not a
	 * per-field merge. Author execution timing in a single place:
	 * the component, or a full owner override, never both.
	 */
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
