// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/MCore_HUD_Base.h"

#include "CoreUI/MCore_UISubsystem.h"

AMCore_HUD_Base::AMCore_HUD_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool AMCore_HUD_Base::HasValidPrimaryGameLayout() const
{
	return GetUISubsystem() && GetUISubsystem()->HasPrimaryGameLayout();
}

UMCore_UISubsystem* AMCore_HUD_Base::GetUISubsystem() const
{
	if (const APlayerController* OwningPlayer = GetOwningPlayerController())
	{
		if (const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer())
		{
			return LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
		}
	}
	return nullptr;
}
