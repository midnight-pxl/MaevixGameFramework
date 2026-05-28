// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "MCore_PlayerSettingsSubsystem.generated.h"

class APlayerController;
class UMCore_PlayerSettingsSave;

/**
 * Manages per-player settings persistence. Owns the PlayerSettingsSave cache and
 * provides typed accessors for player preferences (text size, gamepad icons, etc.).
 * Lazy-loads from disk on first access and saves on Deinitialize.
 *
 * Access via GetLocalPlayer()->GetSubsystem<UMCore_PlayerSettingsSubsystem>().
 *
 * Override GetSettingsSaveSlotName() in Blueprint or C++ subclass
 * for platform-specific identity (Steam ID, Epic Account, etc.).
 */
UCLASS()
class MAEVIXCORE_API UMCore_PlayerSettingsSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	// ============================================================================
	// PLAYER SETTINGS
	// ============================================================================

	/**
	 * Returns the save slot name for this player's settings.
	 * Default: "MCore_PlayerSettings_<PlayerIndex>".
	 * Override for platform identity (Steam ID, Epic Account, etc.).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MaevixCore|Settings")
	FString GetSettingsSaveSlotName() const;

	/** Returns cached player settings, loading from disk on first access. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Settings")
	UMCore_PlayerSettingsSave* GetPlayerSettings();

	/** Returns the active text size index from Accessibility.UITextSize (clamped >= 0). */
	UFUNCTION(BlueprintPure, Category = "MaevixCore|Settings")
	int32 GetActiveTextSizeIndex() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMCore_PlayerSettingsSave> CachedPlayerSettings;

	bool bBootApplyDone = false;
};
