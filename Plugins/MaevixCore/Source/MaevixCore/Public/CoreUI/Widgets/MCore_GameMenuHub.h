// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Primitives/MCore_ActivatableBase.h"
#include "MCore_GameMenuHub.generated.h"

class UCommonButtonBase;
class UCommonActivatableWidget;
class UMCore_TabbedContainer;
struct FGameplayTag;

/**
 * In-game menu hub providing a tabbed interface for plugin menu pages.
 * Composes UMCore_TabbedContainer and rebuilds tabs from UISubsystem's registered screens.
 *
 * Open via UISubsystem->OpenMenuHub(), register screens via RegisterMenuScreen().
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class MAEVIXCORE_API UMCore_GameMenuHub : public UMCore_ActivatableBase
{
    GENERATED_BODY()

public:
    UMCore_GameMenuHub(const FObjectInitializer& ObjectInitializer);

    // ============================================================================
    // PUBLIC API
    // ============================================================================

    /**
     * Rebuild tab bar from currently registered screens.
     * Called by OpenMenuHub() on creation and RegisterMenuScreen() on dynamic registration.
     */
    UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub")
    void RebuildTabBar();

    UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub", meta = (Keywords = "Toggle Lock Tab Button"))
    bool SetTabEnabled(FGameplayTag TabID, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "MaevixCore|Menu Hub", meta = (Keywords = "Toggle Tab Visibility"))
    bool SetTabHidden(FGameplayTag TabID, bool bIsHidden);

    UFUNCTION(BlueprintPure, Category = "MaevixCore|Menu Hub", meta = (Keywords = "Is Tab Enabled"))
    bool IsTabEnabled(FGameplayTag TabID) const;

    UFUNCTION(BlueprintPure, Category = "MaevixCore|Menu Hub", meta = (Keywords = "Is Tab Hidden"))
    bool IsTabHidden(FGameplayTag TabID) const;

    // ============================================================================
    // BLUEPRINT EXTENSION POINTS
    // ============================================================================

    UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Menu Hub")
    void OnTabCreated(FName TabID, UCommonButtonBase* TabButton);
    void OnTabCreated_Implementation(FName TabID, UCommonButtonBase* TabButton)
    {}

    UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|Menu Hub")
    void OnPageCreated(FName TabID, UCommonActivatableWidget* PageWidget);
    void OnPageCreated_Implementation(FName TabID, UCommonActivatableWidget* PageWidget)
    {}

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

    // Menu input mode: cursor visible, no capture, full menu navigation
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

    // Focus the active tab's content widget for gamepad navigation
    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UMCore_TabbedContainer> TabbedContainer;

    UPROPERTY(EditDefaultsOnly, Category = "MaevixCore|Menu Hub")
    TSubclassOf<UCommonActivatableWidget> EmptyStateWidgetClass;

private:
    UFUNCTION()
    void HandleContainerTabAdded(FName TabID, UCommonButtonBase* TabButton);
};
