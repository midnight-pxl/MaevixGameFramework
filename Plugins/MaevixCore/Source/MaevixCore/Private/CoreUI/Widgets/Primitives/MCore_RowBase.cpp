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

	ThemeSub.Bind(this, GET_FUNCTION_NAME_CHECKED(UMCore_RowBase, HandleThemeChanged));

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
	ThemeSub.Unbind(this, GET_FUNCTION_NAME_CHECKED(UMCore_RowBase, HandleThemeChanged));
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
