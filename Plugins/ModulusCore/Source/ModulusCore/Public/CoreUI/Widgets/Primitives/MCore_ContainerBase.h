// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCore_ContainerBase.generated.h"

class UMCore_PDA_UITheme_Base;
class UBorder;
class UImage;
class UNamedSlot;

/**
 * Base container widget with theme integration and optional background styling.
 * Binds automatically to UISubsystem::OnThemeChanged.
 *
 * Optional BindWidgetOptional children: Border_Background, Image_Background, Slot_Content (NamedSlot).
 */
UCLASS(Abstract, Blueprintable, meta=(DisableNativeTick))
class MODULUSCORE_API UMCore_ContainerBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UMCore_ContainerBase();

	// ============================================================================
	// PUBLIC API
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Container")
	void SetBackgroundColor(FLinearColor InColor);

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Container")
	void SetBackgroundBrush(const FSlateBrush& InBrush);

	/** Set background image on Image_Background. Pass nullptr to hide. */
	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Container")
	void SetBackgroundImage(UTexture2D* InTexture);

	UFUNCTION(BlueprintCallable, Category = "ModulusCore|Container")
	void SetContentPadding(FMargin InPadding);

protected:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// THEME
	// ============================================================================

	/** Apply theme to container. Called on init and on theme change. Override for custom handling. */
	UFUNCTION(BlueprintNativeEvent, Category = "ModulusCore|Theme")
	void ApplyTheme(UMCore_PDA_UITheme_Base* Theme);
	virtual void ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* Theme);

	UFUNCTION(BlueprintImplementableEvent, Category = "ModulusCore|Theme", meta = (DisplayName = "On Theme Applied"))
	void K2_OnThemeApplied(UMCore_PDA_UITheme_Base* Theme);

	// ============================================================================
	// WIDGET BINDINGS
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_Background;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Background;

	UPROPERTY(BlueprintReadOnly, Category = "Components", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> Slot_Content;

private:
	UFUNCTION()
	void HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme);

	void BindThemeDelegate();
	void UnbindThemeDelegate();

	UPROPERTY(Transient)
	mutable TWeakObjectPtr<UMCore_PDA_UITheme_Base> CachedTheme;

	bool bThemeDelegateBound{false};
};
