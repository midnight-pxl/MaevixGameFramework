// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/Widgets/Primitives/MCore_RowBase.h"

#include "CoreData/Assets/UI/Themes/MCore_PDA_UITheme_Base.h"
#include "CoreData/DevSettings/MCore_CoreSettings.h"
#include "CoreData/Libraries/MCore_ThemeLibrary.h"
#include "CoreUI/MCore_UISubsystem.h"

#include "CommonBorder.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Engine/LocalPlayer.h"

// ============================================================================
// THEME
// ============================================================================

void UMCore_RowBase::ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;

	if (NewTheme)
	{
		UMCore_ThemeLibrary::ApplyTextStyleFromTheme(
			GetOwningLocalPlayer(), Txt_Label, NewTheme->LabelTextStyle);
	}

	if (HighlightBorder && NewTheme && NewTheme->HighlightBorderStyle)
	{
		if (const UCommonBorderStyle* BorderStyle = GetDefault<UCommonBorderStyle>(NewTheme->HighlightBorderStyle))
		{
			HighlightBorder->SetBrush(BorderStyle->Background);
		}
	}

	K2_OnThemeApplied(NewTheme);
}

void UMCore_RowBase::HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme)
{
	CachedTheme = NewTheme;
	ApplyTheme(NewTheme);
}

void UMCore_RowBase::BindThemeDelegate()
{
	if (bThemeDelegateBound) { return; }

	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer) { return; }

	UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>();
	if (!UISubsystem) { return; }

	UISubsystem->OnThemeChanged.AddDynamic(this, &UMCore_RowBase::HandleThemeChanged);
	bThemeDelegateBound = true;
}

void UMCore_RowBase::UnbindThemeDelegate()
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
		UISubsystem->OnThemeChanged.RemoveDynamic(this, &UMCore_RowBase::HandleThemeChanged);
	}

	bThemeDelegateBound = false;
}

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMCore_RowBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (IsDesignTime())
	{
		ApplyTheme(UMCore_CoreSettings::GetDesignTimeTheme());
	}
}

void UMCore_RowBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindThemeDelegate();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UMCore_UISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UMCore_UISubsystem>())
		{
			ApplyTheme(UISubsystem->GetActiveTheme());
		}
	}

	/* Enforce hidden initial state regardless of WBP designer configuration. */
	if (HighlightBorder)
	{
		HighlightBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMCore_RowBase::NativeDestruct()
{
	UnbindThemeDelegate();
	Super::NativeDestruct();
}

// ============================================================================
// HOVER AND FOCUS
// ============================================================================

void UMCore_RowBase::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bIsHovered = true;
	UpdateHighlightVisibility();
	NotifyRowFocusGained();
}

void UMCore_RowBase::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bIsHovered = false;
	UpdateHighlightVisibility();
}

void UMCore_RowBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	bIsInFocusPath = true;
	UpdateHighlightVisibility();
	NotifyRowFocusGained();
}

void UMCore_RowBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	bIsInFocusPath = false;
	UpdateHighlightVisibility();
}

void UMCore_RowBase::UpdateHighlightVisibility()
{
	if (!HighlightBorder)
	{
		return;
	}

	/* HitTestInvisible (rather than Visible) so the border renders without intercepting
	 * mouse events targeted at child widgets like sliders, step buttons, or rebind buttons. */
	const ESlateVisibility NewVisibility = (bIsHovered || bIsInFocusPath)
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Hidden;

	HighlightBorder->SetVisibility(NewVisibility);
}
