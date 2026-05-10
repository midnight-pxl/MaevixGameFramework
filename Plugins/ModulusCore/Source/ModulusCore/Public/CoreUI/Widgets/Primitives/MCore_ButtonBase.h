// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "MCore_ButtonBase.generated.h"

class UMCore_PDA_UITheme_Base;
class UCommonButtonStyle;
class UCommonTextStyle;
class UWidgetSwitcher;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModulusButtonClicked);

UENUM(BlueprintType)
enum class EMCore_ButtonDisplayMode : uint8
{
	TextOnly,
	IconOnly,
	TextAndIcon
};

UENUM(BlueprintType)
enum class EMCore_ButtonStyleMode : uint8
{
	/* Pulls LabelTextStyle. Default for all buttons */
	Standard,
	/* Pulls TabTextStyle. Set by UMCore_TabbedContainer for tab buttons */
	Tab
};

/**
 * Base button with theme integration, per-instance style overrides, and text/icon display modes.
 * Binds automatically to UISubsystem::OnThemeChanged for runtime theme switching.
 *
 * Optional BindWidgetOptional children: Txt_BtnLabel (text), Img_BtnIcon (icon).
 */
UCLASS(Abstract, Blueprintable, meta=(DisableNativeTick))
class MODULUSCORE_API UMCore_ButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UMCore_ButtonBase();

	// ============================================================================
	// TEXT API
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Button|Text")
	void SetButtonText(const FText& InText);

	UFUNCTION(BlueprintPure, Category = "Button|Text")
	FText GetButtonText() const;

	UFUNCTION(BlueprintPure, Category = "Button|Text")
	UCommonTextBlock* GetTextBlock() const { return Txt_BtnLabel; }

	// ============================================================================
	// STYLE API
	// ============================================================================

	/** Set button style override. Pass nullptr to revert to theme default. */
	UFUNCTION(BlueprintCallable, Category = "Button|Style")
	void SetButtonStyleOverride(TSubclassOf<UCommonButtonStyle> InStyle);

	UFUNCTION(BlueprintPure, Category = "Button|Style")
	TSubclassOf<UCommonButtonStyle> GetButtonStyleOverride() const { return ButtonStyleOverride; }

	/** Set text style override. Pass nullptr to revert to theme default. */
	UFUNCTION(BlueprintCallable, Category = "Button|Style")
	void SetTextStyleOverride(TSubclassOf<UCommonTextStyle> InStyle);

	UFUNCTION(BlueprintPure, Category = "Button|Style")
	TSubclassOf<UCommonTextStyle> GetTextStyleOverride() const { return TextStyleOverride; }

	UFUNCTION(BlueprintPure, Category = "Button|Style")
	bool IsUsingStyleOverrides() const { return ButtonStyleOverride != nullptr || TextStyleOverride != nullptr; }
	
	/**
	 * Sets the button's style mode and immediately re-applies the active theme.
	 * Used by container widgets (e.g. UMCore_TabbedContainer) to retroactively
	 * style buttons spawned through CommonUI's RegisterTab path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Theme")
	void SetStyleMode(EMCore_ButtonStyleMode NewMode);

	// ============================================================================
	// ICON AND DISPLAY
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetButtonIcon(UTexture2D* InIcon);

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetButtonIconSoft(TSoftObjectPtr<UTexture2D> InIcon);

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetDisplayMode(EMCore_ButtonDisplayMode InMode);

	UFUNCTION(BlueprintPure, Category = "Button")
	EMCore_ButtonDisplayMode GetDisplayMode() const { return DisplayMode; }

	// ============================================================================
	// WIDGET BINDINGS AND DELEGATES
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_BtnLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_BtnIcon;

	UPROPERTY(BlueprintAssignable, Category = "Button")
	FOnModulusButtonClicked OnButtonClicked;

	/**
	 * Programmatically trigger this button's click logic (sound, animation, delegates).
	 */
	UFUNCTION(BlueprintCallable, Category = "Button")
	void SimulateClick();

protected:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnClicked() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// THEME
	// ============================================================================

	/** Apply theme to button. Called on init and on theme change. Override for custom handling. */
	UFUNCTION(BlueprintNativeEvent, Category = "Theme")
	void ApplyTheme(UMCore_PDA_UITheme_Base* Theme);
	virtual void ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* Theme);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Theme", meta = (DisplayName = "On Theme Applied"))
	void K2_OnThemeApplied(UMCore_PDA_UITheme_Base* Theme);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Button")
	EMCore_ButtonDisplayMode DisplayMode = EMCore_ButtonDisplayMode::TextOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button|Text", meta = (MultiLine = false))
	FText ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button|Icon")
	TObjectPtr<UTexture2D> ButtonIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button|Theme")
	EMCore_ButtonStyleMode ButtonStyleMode = EMCore_ButtonStyleMode::Standard;
	
	/** Override takes precedence over theme's PrimaryButtonStyle. Leave None for theme default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button|Style")
	TSubclassOf<UCommonButtonStyle> ButtonStyleOverride;

	/** Override takes precedence over theme's BodyTextStyle. Leave None for theme default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button|Style")
	TSubclassOf<UCommonTextStyle> TextStyleOverride;

private:
	void SyncPropertiesToWidgets();

	UFUNCTION()
	void HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme);

	void BindThemeDelegate();
	void UnbindThemeDelegate();

	UPROPERTY(Transient)
	mutable TWeakObjectPtr<UMCore_PDA_UITheme_Base> CachedTheme;

	bool bThemeDelegateBound{false};
};
