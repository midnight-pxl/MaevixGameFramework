// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCore_RowBase.generated.h"

class UCommonTextBlock;
class UBorder;
class UMCore_PDA_UITheme_Base;

/**
 * Abstract shared base for selectable, themed row widgets (settings rows,
 * keybinding rows, and any future row-shaped widgets).
 *
 * Owns the theme delegate plumbing, the row-level hover/focus state machine,
 * and the optional Txt_Label / HighlightBorder slots common to all rows.
 *
 * Subclasses override ApplyTheme_Implementation (calling Super first) to
 * layer their own widget styling, and may override NotifyRowFocusGained to
 * react when the row gains hover or focus.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "ModulusUI", meta = (DisableNativeTick))
class MODULUSCORE_API UMCore_RowBase : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	// ====================================================================
	// BIND WIDGETS
	// ====================================================================

	/* Optional row label widget. Subclasses use this for the row's primary
	 * identifier text (setting name, action name, etc.). Themed automatically
	 * via UMCore_PDA_UITheme_Base::LabelTextStyle on ApplyTheme. If unbound
	 * in the WBP, label theming silently no-ops. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Label;

	/* Optional border highlighting the row while focused or hovered. Visibility
	 * and styling are managed automatically by ApplyTheme + focus path events.
	 * Theme authors control appearance via UMCore_PDA_UITheme_Base::HighlightBorderStyle.
	 * If unbound in the WBP, the highlight feature silently no-ops. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> HighlightBorder;

	// ====================================================================
	// THEME
	// ====================================================================

	UFUNCTION(BlueprintNativeEvent, Category = "ModulusCore|Theme")
	void ApplyTheme(UMCore_PDA_UITheme_Base* NewTheme);
	virtual void ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme);

	UFUNCTION(BlueprintImplementableEvent, Category = "ModulusCore|Theme",
		meta = (DisplayName = "On Theme Applied"))
	void K2_OnThemeApplied(UMCore_PDA_UITheme_Base* Theme);

	UPROPERTY(Transient)
	mutable TWeakObjectPtr<UMCore_PDA_UITheme_Base> CachedTheme;

	// ====================================================================
	// LIFECYCLE
	// ====================================================================

	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	//~ Begin UUserWidget interface
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	//~ End UUserWidget interface

	/* Virtual hook called after the row enters hover OR the focus path
	 * (after state bool + visibility update). Default empty. Subclasses
	 * override to publish row-attention events (e.g. driving an external
	 * description pane). Fires once per gain event; no symmetric Lost hook. */
	virtual void NotifyRowFocusGained() {}

	/* Recomputes HighlightBorder visibility from the OR of focus-path and hover state.
	 * Uses HitTestInvisible while visible so child widget mouse events are not intercepted.
	 * Safe to call when HighlightBorder is unbound. */
	void UpdateHighlightVisibility();

	/* Row-level hover and focus-path state. Distinct from any child widget's
	 * own hover/focus. Drives UpdateHighlightVisibility() via the four
	 * lifecycle overrides. */
	bool bIsHovered = false;
	bool bIsInFocusPath = false;

private:
	UFUNCTION()
	void HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme);
	void BindThemeDelegate();
	void UnbindThemeDelegate();
	bool bThemeDelegateBound{false};
};
