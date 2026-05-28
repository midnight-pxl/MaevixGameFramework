// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_ButtonBase.h"
#include "MCore_ActionButton.generated.h"

class UCommonInputSubsystem;
enum class ECommonInputType : uint8;

/**
 * Button that displays the platform-appropriate input icon for a CommonUI action.
 * Subscribes to input method changes and refreshes automatically.
 *
 * Set InputAction to a CommonUI DT row, then the button resolves the correct
 * glyph for the current device (keyboard, Xbox, PlayStation, etc.).
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_ActionButton : public UMCore_ButtonBase
{
	GENERATED_BODY()

public:

	// ============================================================================
	// INPUT ACTION
	// ============================================================================

	/** CommonUI DataTable row this button represents. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Button|InputAction")
	FDataTableRowHandle InputAction;

	/** When true, forces IconOnly display mode on initialization. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|Button|InputAction")
	bool bShowIconOnly{false};

	/** Set the input action and refresh the icon for the current platform. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Button|InputAction")
	void SetInputAction(const FDataTableRowHandle& InAction);

	/** Re-resolve the platform icon from the current input method and apply it. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|Button|InputAction")
	void RefreshInputIcon();

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:

	void HandleInputMethodChanged(ECommonInputType NewInputType);

	FDelegateHandle InputMethodChangedHandle;
};