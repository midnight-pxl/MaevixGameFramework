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
#include "Components/WidgetSwitcher.h"
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

	UE_LOG(LogModulusUI, Verbose, TEXT("ButtonBase::NativeOnInitialized: initialized, widget=%s"), *GetNameSafe(this));

	BindThemeDelegate();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UISubsystem->GetActiveTheme());
		}
	}

	SyncPropertiesToWidgets();
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
	UE_LOG(LogModulusUI, Verbose, TEXT("ButtonBase::NativeDestruct: destructing, widget=%s"), *GetNameSafe(this));

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

void UMCore_ButtonBase::SetButtonIconBrush(const FSlateBrush& InBrush)
{
	if (Img_BtnIcon)
	{
		Img_BtnIcon->SetBrush(InBrush);
		Img_BtnIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UMCore_ButtonBase::SetDisplayMode(EMCore_ButtonDisplayMode InMode)
{
	DisplayMode = InMode;

	if (WS_BtnContent)
	{
		WS_BtnContent->SetActiveWidgetIndex(static_cast<int32>(DisplayMode));
		return;
	}

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
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UISubsystem->GetActiveTheme());
		}
	}
}

void UMCore_ButtonBase::SetTextStyleOverride(TSubclassOf<UCommonTextStyle> InStyle)
{
	TextStyleOverride = InStyle;

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UISubsystem->GetActiveTheme());
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
		const TArray<TSubclassOf<UCommonTextStyle>>& TextStyles =
			(ButtonStyleMode == EMCore_ButtonStyleMode::Tab)
			? Theme->TabTextStyle
			: Theme->LabelTextStyle;
		
		UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
			GetOwningLocalPlayer(), Txt_BtnLabel, TextStyles);
	}

	K2_OnThemeApplied(Theme);
}

void UMCore_ButtonBase::SetStyleMode(EMCore_ButtonStyleMode NewMode)
{
	if (ButtonStyleMode == NewMode)
	{
		return;
	}

	ButtonStyleMode = NewMode;

	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UISubsystem->GetActiveTheme());
		}
	}
}

void UMCore_ButtonBase::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_ButtonBase::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	UISubsystem->OnThemeChanged.AddDynamic(this, &UMCore_ButtonBase::HandleThemeChanged);
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

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (UISubsystem)
	{
		UISubsystem->OnThemeChanged.RemoveDynamic(this, &UMCore_ButtonBase::HandleThemeChanged);
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
