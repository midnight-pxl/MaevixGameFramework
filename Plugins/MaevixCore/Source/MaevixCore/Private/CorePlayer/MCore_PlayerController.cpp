// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CorePlayer/MCore_PlayerController.h"

#include "CoreData/Tags/MCore_UILayerTags.h"
#include "CoreData/Logging/LogMaevixPlayer.h"
#include "CoreUI/MCore_UISubsystem.h"
#include "CoreUI/Widgets/Primitives/MCore_ActivatableBase.h"

AMCore_PlayerController::AMCore_PlayerController()
{
	PrimaryWidgetLayer = MCore_UILayerTags::MCore_UI_Layer_Game;
}

void AMCore_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) { return; }

	InitializeUISystem();
}

void AMCore_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(PrimaryWidget))
	{
		PrimaryWidget->DeactivateWidget();
		PrimaryWidget = nullptr;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			UISubsystem->OnPrimaryGameLayoutReady.RemoveDynamic(this, &ThisClass::OnPrimaryGameLayoutReady);
		}
	}

	bUISystemInitialized = false;
	Super::EndPlay(EndPlayReason);
}

void AMCore_PlayerController::InitializeUISystem()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	if (UISubsystem->HasPrimaryGameLayout())
	{
		OnUISystemReady(UISubsystem);
	}
	else
	{
		UISubsystem->OnPrimaryGameLayoutReady.AddDynamic(this, &ThisClass::OnPrimaryGameLayoutReady);
		UE_LOG(LogMaevixPlayer, Verbose, TEXT("PlayerController::InitializeUISystem: waiting for PrimaryGameLayout"));
	}
}

void AMCore_PlayerController::OnPrimaryGameLayoutReady(UMCore_PrimaryGameLayout* Layout)
{
	if (bUISystemInitialized) { return; }

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	UISubsystem->OnPrimaryGameLayoutReady.RemoveDynamic(this, &ThisClass::OnPrimaryGameLayoutReady);
	OnUISystemReady(UISubsystem);
}

void AMCore_PlayerController::OnUISystemReady_Implementation(UMCore_UISubsystem* UISubsystem)
{
	if (bUISystemInitialized) { return; }

	if (PrimaryWidgetClass && PrimaryWidgetLayer.IsValid())
	{
		PrimaryWidget = UISubsystem->OpenScreen(PrimaryWidgetClass, PrimaryWidgetLayer);

		if (PrimaryWidget)
		{
			UE_LOG(LogMaevixPlayer, Log, TEXT("PlayerController::OnUISystemReady: pushed '%s' to layer '%s'"),
				*PrimaryWidgetClass->GetName(), *PrimaryWidgetLayer.ToString());
		}
		else
		{
			UE_LOG(LogMaevixPlayer, Error, TEXT("PlayerController::OnUISystemReady: failed to push '%s' to layer '%s'"),
				*PrimaryWidgetClass->GetName(), *PrimaryWidgetLayer.ToString());
		}
	}

	bUISystemInitialized = true;
}
