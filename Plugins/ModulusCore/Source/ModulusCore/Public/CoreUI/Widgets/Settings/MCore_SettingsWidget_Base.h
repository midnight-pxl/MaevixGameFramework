// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_RowBase.h"
#include "GameplayTagContainer.h"
#include "MCore_SettingsWidget_Base.generated.h"

class UMCore_DA_SettingDefinition;
class UMCore_PDA_UITheme_Base;
struct FMCore_EventData;

/**
 * Fired when user changes a setting value.
 * Value has already been applied to engine.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSettingValueChanged,
    FGameplayTag, SettingTag, const FString&, NewValueString);

/** Fires when this widget gains hover or enters the focus path. Use to drive description/preview panes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSettingFocused,
    FGameplayTag, SettingTag,
    FText, Description);

/**
 * Abstract base for all setting input widgets (Slider, Toggle, Switcher).
 * Immediate-apply model: every user interaction writes to engine immediately.
 *
 * Derived classes implement type-specific value handling:
 * - UMCore_SettingsWidget_Slider (float values: volume, brightness, sensitivity)
 * - UMCore_SettingsWidget_Switcher (discrete options: resolution, quality presets)
 */
UCLASS(Abstract, Blueprintable, ClassGroup= "ModulusUI", meta = (DisableNativeTick))
class MODULUSCORE_API UMCore_SettingsWidget_Base : public UMCore_RowBase
{
	GENERATED_BODY()

public:
    // ====================================================================
    // INITIALIZATION
    // ====================================================================

    /** Bind this widget to a setting definition. Populates display name and calls OnDefinitionSet. */
    UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
    void InitFromDefinition(const UMCore_DA_SettingDefinition* InDefinition);

    UFUNCTION(BlueprintPure, Category = "ModulusCore|Settings")
    const UMCore_DA_SettingDefinition* GetSettingDefinition() const { return SettingDefinition; }

    UFUNCTION(BlueprintPure, Category = "ModulusCore|Settings")
    FGameplayTag GetSettingTag() const;

    // ====================================================================
    // VALUE INTERFACE
    // ====================================================================

    /** Reset to DataAsset default value. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ModulusCore|Settings")
    void ResetToDefault();
    
    /** Re-reads the current value from the settings library and updates the widget display. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ModulusCore|Settings")
    void RefreshValueFromSettings();

    /** String representation of current value (for debug/display). */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "ModulusCore|Settings")
    FString GetValueAsString() const;

    /**
     * Step the setting value left (decrement).
     * Panel calls this for gamepad input forwarding.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ModulusCore|Settings")
    void StepLeft();

    /**
     * Step the setting value right (increment).
     * Panel calls this for gamepad input forwarding.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ModulusCore|Settings")
    void StepRight();

    // ====================================================================
    // EVENTS
    // ====================================================================

    UPROPERTY(BlueprintAssignable, Category = "ModulusCore|Settings")
    FOnSettingValueChanged OnSettingValueChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "ModulusCore|Settings")
    FOnSettingFocused  OnSettingFocused;

protected:
    // ====================================================================
    // SUBCLASS HOOKS
    // ====================================================================

    /**
     * Called after InitFromDefinition; subclass reads type-specific
     * DataAsset properties and populates UI controls.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "ModulusCore|Settings")
    void OnDefinitionSet(const UMCore_DA_SettingDefinition* Definition);

    /** Subclasses call this after applying a new value internally. Fires OnSettingValueChanged with the current GetValueAsString(). */
    UFUNCTION(BlueprintCallable, Category = "ModulusCore|Settings")
    void BroadcastValueChanged();

    // ====================================================================
    // DATA
    // ====================================================================

    UPROPERTY(Transient, BlueprintReadOnly, Category = "ModulusCore|Settings")
    TObjectPtr<const UMCore_DA_SettingDefinition> SettingDefinition;
    
    UPROPERTY(Transient, BlueprintReadOnly, Category = "ModulusCore|Settings")
    bool bIsSettingEnabled{true};

    // ====================================================================
    // LIFECYCLE
    // ====================================================================

    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

    /* Override of UMCore_RowBase hook. Broadcasts OnSettingFocused so the
     * settings panel can update its description pane when a row gains
     * hover or focus. */
    virtual void NotifyRowFocusGained() override;

    /* Broadcasts OnSettingFocused using the cached definition. No-op if definition is unset. */
    void BroadcastFocusedIfValid();

private:
    /* Filters local events for MCore.Settings.Event.ExternalValueChange and refreshes display. */
    void HandleLocalEvent(const FMCore_EventData& EventData);
    FDelegateHandle EventSubscriptionHandle;
};
