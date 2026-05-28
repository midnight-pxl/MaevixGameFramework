// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MCore_GamepadIconSwitcher.generated.h"

class UMCore_ButtonBase;
class UCommonTextBlock;

/**
 * Cycling selector that overrides which gamepad icon set is displayed. Lets PC
 * players pick which controller icons to display when auto-detection is wrong
 * (common with Steam input translation).
 *
 * Option 0 is always "Auto-Detect"; remaining options are pulled from registered
 * CommonInput controller data assets at runtime.
 *
 * Requires BindWidget: Btn_Previous, Btn_Next, Txt_PlatformName.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_GamepadIconSwitcher : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	// ====================================================================
	// PUBLIC API
	// ====================================================================

	/** Rebuild option list from registered configs and sync display to saved index. */
	UFUNCTION(BlueprintCallable, Category = "UI|Settings")
	void RefreshDisplay();

	UFUNCTION(BlueprintPure, Category = "UI|Settings")
	int32 GetCurrentIndex() const { return CurrentIndex; }

protected:

	// ====================================================================
	// BIND WIDGETS
	// ====================================================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Previous;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UMCore_ButtonBase> Btn_Next;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_PlatformName;

	// ====================================================================
	// LIFECYCLE
	// ====================================================================

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	// ====================================================================
	// STATE
	// ====================================================================

	UPROPERTY(BlueprintReadOnly, Category = "UI|Settings")
	TArray<FText> AvailableOptions;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Settings")
	int32 CurrentIndex = 0;

private:

	// ====================================================================
	// HANDLERS
	// ====================================================================

	UFUNCTION()
	void HandlePrevious();

	UFUNCTION()
	void HandleNext();

	void ApplySelection();
};
