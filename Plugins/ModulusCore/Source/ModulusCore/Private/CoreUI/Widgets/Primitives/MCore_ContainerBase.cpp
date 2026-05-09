// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_ContainerBase.h"

#include "CoreUI/MCore_UISubsystem.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Logging/LogModulusUI.h"

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

	UE_LOG(LogModulusUI, Verbose, TEXT("ContainerBase::NativeOnInitialized -- initialized, widget=%s"), *GetNameSafe(this));

	BindThemeDelegate();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UI->GetActiveTheme());
		}
	}

	// TODO: Remove after button theme init diagnostic (2026-05-08)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const UMCore_UISubsystem* UI =
			LP ? LP->GetSubsystem<UMCore_UISubsystem>() : nullptr;
		const UMCore_PDA_UITheme_Base* CurrentTheme =
			UI ? UI->GetActiveTheme() : nullptr;
		UE_LOG(LogModulusUI, Log,
			TEXT("ContainerBase::NativeOnInitialized -- widget=%s ActiveTheme=%s CachedTheme=%s"),
			*GetName(),
			CurrentTheme ? *CurrentTheme->GetName() : TEXT("null"),
			CachedTheme.IsValid() ? *CachedTheme->GetName() : TEXT("null/invalid"));
	}
}

void UMCore_ContainerBase::NativeDestruct()
{
	UE_LOG(LogModulusUI, Verbose, TEXT("ContainerBase::NativeDestruct -- destructing, widget=%s"), *GetNameSafe(this));

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
		UE_LOG(LogModulusUI, Verbose, TEXT("ContainerBase::ApplyTheme -- theme applied, widget=%s"), *GetNameSafe(this));
	}

	K2_OnThemeApplied(Theme);
}

void UMCore_ContainerBase::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	// TODO: Remove after button theme init diagnostic (2026-05-08)
	UE_LOG(LogModulusUI, Log,
		TEXT("ContainerBase::HandleThemeChanged -- widget=%s NewTheme=%s"),
		*GetName(),
		NewTheme ? *NewTheme->GetName() : TEXT("null"));

	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_ContainerBase::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UI) { return; }

	UI->OnThemeChanged.AddDynamic(this, &UMCore_ContainerBase::HandleThemeChanged);
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

	UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (UI)
	{
		UI->OnThemeChanged.RemoveDynamic(this, &UMCore_ContainerBase::HandleThemeChanged);
	}

	bThemeDelegateBound = false;
}
