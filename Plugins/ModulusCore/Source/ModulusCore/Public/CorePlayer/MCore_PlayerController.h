// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "GameFramework/PlayerController.h"
#include "MCore_PlayerController.generated.h"

class UInputAction;
class UMCore_UISubsystem;
class UMCore_PrimaryGameLayout;
class UInputMappingContext;
class UCommonActivatableWidget;

/**
 * Base PlayerController for ModulusCore with deferred HUD widget setup.
 * Creates and pushes PrimaryWidgetClass to the specified layer when UISubsystem is ready.
 *
 * Set PrimaryWidgetClass and PrimaryWidgetLayer in your Blueprint subclass.
 */
UCLASS()
class MODULUSCORE_API AMCore_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMCore_PlayerController();

	// ============================================================================
	// ACTOR OVERRIDES
	// ============================================================================

	virtual void BeginPlay() override;

protected:
	// ============================================================================
	// PLAYERCONTROLLER OVERRIDES
	// ============================================================================

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Widget to push when UI system ready (HUD, MainMenu, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|UI")
	TSubclassOf<UCommonActivatableWidget> PrimaryWidgetClass;

	/** Layer to push PrimaryWidgetClass onto. Default: MCore_UI_Layer_Game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ModulusCore|UI", meta = (Categories = "MCore.UI.Layer"))
	FGameplayTag PrimaryWidgetLayer;

	UPROPERTY(EditDefaultsOnly, Category = "ModulusCore|Input")
	TArray<TObjectPtr<UInputMappingContext>> InputContexts;

	/**
	 * Called when UISubsystem and PrimaryGameLayout are ready.
	 * Default pushes PrimaryWidgetClass to PrimaryWidgetLayer.
	 * Override for custom setup (clear layers, push additional widgets, etc.)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ModulusCore|UI")
	void OnUISystemReady(UMCore_UISubsystem* UISubsystem);
	virtual void OnUISystemReady_Implementation(UMCore_UISubsystem* UISubsystem);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "ModulusCore|UI")
	TObjectPtr<UCommonActivatableWidget> PrimaryWidget;

private:
	UFUNCTION()
	void OnPrimaryGameLayoutReady(UMCore_PrimaryGameLayout* Layout);

	void InitializeUISystem();

	bool bUISystemInitialized{false};
};
