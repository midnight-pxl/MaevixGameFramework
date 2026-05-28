// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/MCore_PrimaryGameLayout.h"

#include "CoreData/Logging/LogMaevixUI.h"
#include "CoreUI/MCore_UISubsystem.h"

UMCore_PrimaryGameLayout::UMCore_PrimaryGameLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMCore_PrimaryGameLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (this->AreAllLayersBound())
	{
		UE_LOG(LogMaevixUI, Log, TEXT("PrimaryGameLayout::NativeOnInitialized: all 4 layer stacks bound successfully"));
	}
	else
	{
		UE_LOG(LogMaevixUI, Error,
			TEXT("PrimaryGameLayout::NativeOnInitialized: missing layer stacks - Game:%s GameMenu:%s Menu:%s Modal:%s"),
			MCore_GameLayer ? TEXT("OK") : TEXT("MISSING"),
			MCore_GameMenuLayer ? TEXT("OK") : TEXT("MISSING"),
			MCore_MenuLayer ? TEXT("OK") : TEXT("MISSING"),
			MCore_ModalLayer ? TEXT("OK") : TEXT("MISSING"));
	}
}

