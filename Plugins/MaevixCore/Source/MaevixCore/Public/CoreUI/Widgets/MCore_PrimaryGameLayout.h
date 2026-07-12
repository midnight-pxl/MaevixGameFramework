// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "CommonActivatableWidget.h"
#include "MCore_PrimaryGameLayout.generated.h"

/**
 * Primary game layout providing the 4-layer CommonUI widget stack
 * (Game / GameMenu / Menu / Modal).
 * Visual container only; access layers via UISubsystem's screen management API.
 */

class UMCore_UISubsystem;
class UMCore_ToastHost;

UCLASS(BlueprintType, Blueprintable, Meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_PrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()

	friend class UMCore_UISubsystem;

public:
    UMCore_PrimaryGameLayout(const FObjectInitializer& ObjectInitializer);
	
	/** True when all four layer stacks (Game / GameMenu / Menu / Modal) are bound. */
	bool AreAllLayersBound() const
	{
		return MCore_GameLayer != nullptr &&
			MCore_GameMenuLayer != nullptr &&
				MCore_MenuLayer != nullptr &&
					MCore_ModalLayer != nullptr;
	};

protected:
	virtual void NativeOnInitialized() override;

private:
	// ============================================================================
	// LAYER STACKS: accessed by UISubsystem via friend
	// ============================================================================

	/** HUD and persistent gameplay UI. */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_GameLayer;

	/** In-game menus (inventory, map, crafting). */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_GameMenuLayer;

	/** Full-screen menus (main menu, settings, pause). */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_MenuLayer;

	/** Dialogs and confirmation popups. */
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MCore_ModalLayer;

	/**
	 * Optional non-activatable host for transient toasts. Bound only if the
	 * layout WBP supplies a matching widget; absence is non-fatal.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UMCore_ToastHost> MCore_ToastHost;

public:
	/** Toast host if the WBP provides one, else nullptr. Read by UMCore_ToastSubsystem. */
	UMCore_ToastHost* GetToastHost() const { return MCore_ToastHost; }
};
