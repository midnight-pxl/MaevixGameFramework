// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"

#include "CoreUI/MCore_UISubsystem.h"
#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"
#include "CoreData/Logging/LogModulusUI.h"

#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

UMCore_ButtonBase::UMCore_ButtonBase()
{
	ButtonText = FText::GetEmpty();
}

void UMCore_ButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		ApplyTheme(UMCore_CoreSettings::GetDesignTimeTheme());
	}
	SyncPropertiesToWidgets();
}

void UMCore_ButtonBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UE_LOG(LogModulusUI, Verbose, TEXT("ButtonBase::NativeOnInitialized -- initialized, widget=%s"), *GetNameSafe(this));

	BindThemeDelegate();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UI->GetActiveTheme());
		}
	}

	SyncPropertiesToWidgets();

	// TODO: Remove after button theme init diagnostic (2026-05-08)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const UMCore_UISubsystem* UI =
			LP ? LP->GetSubsystem<UMCore_UISubsystem>() : nullptr;
		const UMCore_PDA_UITheme_Base* CurrentTheme =
			UI ? UI->GetActiveTheme() : nullptr;
		UE_LOG(LogModulusUI, Log,
			TEXT("ButtonBase::NativeOnInitialized -- widget=%s ActiveTheme=%s CachedTheme=%s"),
			*GetName(),
			CurrentTheme ? *CurrentTheme->GetName() : TEXT("null"),
			CachedTheme.IsValid() ? *CachedTheme->GetName() : TEXT("null/invalid"));
	}
}

void UMCore_ButtonBase::NativeOnClicked()
{
	Super::NativeOnClicked();
	OnButtonClicked.Broadcast();
}

void UMCore_ButtonBase::SimulateClick()
{
	HandleButtonClicked();
}

void UMCore_ButtonBase::NativeDestruct()
{
	UE_LOG(LogModulusUI, Verbose, TEXT("ButtonBase::NativeDestruct -- destructing, widget=%s"), *GetNameSafe(this));

	UnbindThemeDelegate();

	Super::NativeDestruct();
}

void UMCore_ButtonBase::SetButtonText(const FText& InText)
{
	ButtonText = InText;

	if (Txt_BtnLabel)
	{
		Txt_BtnLabel->SetText(InText);
	}
}

FText UMCore_ButtonBase::GetButtonText() const
{
	return ButtonText;
}

void UMCore_ButtonBase::SetButtonIcon(UTexture2D* InIcon)
{
	ButtonIcon = InIcon;

	if (Img_BtnIcon)
	{
		if (InIcon)
		{
			Img_BtnIcon->SetBrushFromTexture(InIcon);
			Img_BtnIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			Img_BtnIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMCore_ButtonBase::SetButtonIconSoft(TSoftObjectPtr<UTexture2D> InIcon)
{
	if (InIcon.IsNull())
	{
		SetButtonIcon(nullptr);
		return;
	}

	/* Synchronous load - for async, use StreamableManager pattern */
	SetButtonIcon(InIcon.LoadSynchronous());
}

void UMCore_ButtonBase::SetDisplayMode(EMCore_ButtonDisplayMode InMode)
{
	DisplayMode = InMode;

	const bool bShowText = (DisplayMode == EMCore_ButtonDisplayMode::TextOnly
		|| DisplayMode == EMCore_ButtonDisplayMode::TextAndIcon);
	const bool bShowIcon = (DisplayMode == EMCore_ButtonDisplayMode::IconOnly
		|| DisplayMode == EMCore_ButtonDisplayMode::TextAndIcon);

	if (Txt_BtnLabel)
	{
		Txt_BtnLabel->SetVisibility(bShowText ?
			ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Img_BtnIcon)
	{
		Img_BtnIcon->SetVisibility(bShowIcon ?
			ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UMCore_ButtonBase::SetButtonStyleOverride(TSubclassOf<UCommonButtonStyle> InStyle)
{
	ButtonStyleOverride = InStyle;

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UI->GetActiveTheme());
		}
	}
}

void UMCore_ButtonBase::SetTextStyleOverride(TSubclassOf<UCommonTextStyle> InStyle)
{
	TextStyleOverride = InStyle;

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UI->GetActiveTheme());
		}
	}
}

void UMCore_ButtonBase::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* Theme)
{
	CachedTheme = Theme;

	const TSubclassOf<UCommonButtonStyle> StyleToApply =
		UMCore_ThemeLibrary::ResolveButtonStyle(
			ButtonStyleOverride, Theme ? Theme->PrimaryButtonStyle : nullptr);
	
	if (StyleToApply) { SetStyle(StyleToApply); }
	
	if (TextStyleOverride)
	{
		if (Txt_BtnLabel)
		{
			Txt_BtnLabel->SetStyle(TextStyleOverride);
		}
	}
	else if (Theme)
	{
		UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
			GetOwningLocalPlayer(), Txt_BtnLabel, Theme->LabelTextStyle);
	}

	K2_OnThemeApplied(Theme);
}

void UMCore_ButtonBase::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	// TODO: Remove after button theme init diagnostic (2026-05-08)
	UE_LOG(LogModulusUI, Log,
		TEXT("ButtonBase::HandleThemeChanged -- widget=%s NewTheme=%s"),
		*GetName(),
		NewTheme ? *NewTheme->GetName() : TEXT("null"));

	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_ButtonBase::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UI = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UI) { return; }

	UI->OnThemeChanged.AddDynamic(this, &UMCore_ButtonBase::HandleThemeChanged);
	bThemeDelegateBound = true;
}

void UMCore_ButtonBase::UnbindThemeDelegate()
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
		UI->OnThemeChanged.RemoveDynamic(this, &UMCore_ButtonBase::HandleThemeChanged);
	}

	bThemeDelegateBound = false;
}

void UMCore_ButtonBase::SyncPropertiesToWidgets()
{
	if (Txt_BtnLabel && !ButtonText.IsEmpty())
	{
		Txt_BtnLabel->SetText(ButtonText);
	}

	if (Img_BtnIcon && ButtonIcon)
	{
		Img_BtnIcon->SetBrushFromTexture(ButtonIcon);
	}

	SetDisplayMode(DisplayMode);

	if (ButtonStyleOverride)
	{
		SetStyle(ButtonStyleOverride);
	}
	if (TextStyleOverride && Txt_BtnLabel)
	{
		Txt_BtnLabel->SetStyle(TextStyleOverride);
	}
}
