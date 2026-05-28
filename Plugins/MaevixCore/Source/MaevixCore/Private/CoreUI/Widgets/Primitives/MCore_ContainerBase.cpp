// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_ContainerBase.h"

#include "CoreUI/MCore_UISubsystem.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Logging/LogMaevixUI.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/NamedSlot.h"
#include "Engine/Texture2D.h"

UMCore_ContainerBase::UMCore_ContainerBase()
{
}

void UMCore_ContainerBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		ApplyTheme(UMCore_CoreSettings::GetDesignTimeTheme());
	}
}

void UMCore_ContainerBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UE_LOG(LogMaevixUI, Verbose, TEXT("ContainerBase::NativeOnInitialized: initialized, widget=%s"), *GetNameSafe(this));

	BindThemeDelegate();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UISubsystem->GetActiveTheme());
		}
	}
}

void UMCore_ContainerBase::NativeDestruct()
{
	UE_LOG(LogMaevixUI, Verbose, TEXT("ContainerBase::NativeDestruct: destructing, widget=%s"), *GetNameSafe(this));

	UnbindThemeDelegate();

	Super::NativeDestruct();
}

void UMCore_ContainerBase::SetBackgroundColor(FLinearColor InColor)
{
	if (Border_Background)
	{
		Border_Background->SetBrushColor(InColor);
	}
}

void UMCore_ContainerBase::SetBackgroundBrush(const FSlateBrush& InBrush)
{
	if (Border_Background)
	{
		Border_Background->SetBrush(InBrush);
	}
}

void UMCore_ContainerBase::SetBackgroundImage(UTexture2D* InTexture)
{
	if (Image_Background)
	{
		if (InTexture)
		{
			Image_Background->SetBrushFromTexture(InTexture);
			Image_Background->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Image_Background->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMCore_ContainerBase::SetContentPadding(FMargin InPadding)
{
	if (Border_Background)
	{
		Border_Background->SetPadding(InPadding);
	}
}

void UMCore_ContainerBase::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* Theme)
{
	CachedTheme = Theme;

	if (Theme)
	{
		UE_LOG(LogMaevixUI, Verbose, TEXT("ContainerBase::ApplyTheme: theme applied, widget=%s"), *GetNameSafe(this));
	}

	K2_OnThemeApplied(Theme);
}

void UMCore_ContainerBase::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_ContainerBase::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	UISubsystem->OnThemeChanged.AddDynamic(this, &UMCore_ContainerBase::HandleThemeChanged);
	bThemeDelegateBound = true;
}

void UMCore_ContainerBase::UnbindThemeDelegate()
{
	if (!bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		bThemeDelegateBound = false;
		return;
	}

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (UISubsystem)
	{
		UISubsystem->OnThemeChanged.RemoveDynamic(this, &UMCore_ContainerBase::HandleThemeChanged);
	}

	bThemeDelegateBound = false;
}
