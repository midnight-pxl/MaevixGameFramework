// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"
#include "MCore_PrimaryGameLayout.generated.h"

/**
 * Primary game layout providing the 4-layer CommonUI widget stack.
 * Visual container only; access layers via UISubsystem's screen management API.
 *
 * Layers: GameLayer (HUD), GameMenuLayer (in-game menus),
 * MenuLayer (full-screen menus), ModalLayer (dialogs).
 */

class UMCore_UISubsystem;

UCLASS(BlueprintType, Blueprintable, Meta = (DisableNativeTick))
class MODULUSCORE_API UMCore_PrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

	friend class UMCore_UISubsystem;

public:
    UMCore_PrimaryGameLayout(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

private:
	// ============================================================================
	// LAYER STACKS: accessed by UISubsystem via friend
	// ============================================================================

	/** HUD and persistent gameplay UI */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_GameLayer;

	/** In-game menus (inventory, map, crafting) */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_GameMenuLayer;

	/** Full-screen menus (main menu, settings, pause) */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_MenuLayer;

	/** Dialogs and confirmation popups */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_ModalLayer;
};
