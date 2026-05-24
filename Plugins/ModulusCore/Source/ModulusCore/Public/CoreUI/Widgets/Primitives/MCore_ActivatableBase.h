// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Input/UIActionBindingHandle.h"
#include "MCore_ActivatableBase.generated.h"

class UMCore_PDA_UITheme_Base;

/** Blueprint-friendly wrapper for CommonUI's FUIActionBindingHandle. */
USTRUCT(BlueprintType)
struct MODULUSCORE_API FMCore_InputActionBindingHandle
{
	GENERATED_BODY()

	FUIActionBindingHandle CommonHandle;

	bool IsValid() const { return CommonHandle.IsValid(); }
};

/**
 * Dynamic delegate signature for input action callbacks.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FInputActionExecutedDelegate, FName, ActionName);

/**
 * Base activatable widget with theme integration, input binding management, and BlockTag support.
 * Automatically cleans up input bindings on deactivation to prevent memory leaks.
 *
 * Derive in Blueprint: use RegisterBinding() for input actions with auto-cleanup,
 * override ApplyTheme for theme changes, and set BlockTags to conditionally prevent activation.
 */
UCLASS(Abstract, Blueprintable, ClassGroup="ModulusUI", meta=(DisableNativeTick))
class MODULUSCORE_API UMCore_ActivatableBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UMCore_ActivatableBase(const FObjectInitializer& ObjectInitializer);

	/** GameplayTags that block this widget from activating when present on OwningPlayer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|Activation", meta=(Categories = "State"))
	FGameplayTagContainer BlockTags;

	/** Register an input action binding with automatic cleanup on deactivation. */
	UFUNCTION(BlueprintCallable, Category="UI|Input", meta=(AutoCreateRefTerm = "OverrideDisplayName"))
	void RegisterBinding(
		FDataTableRowHandle InputAction,
		const FInputActionExecutedDelegate& Callback,
		UPARAM(ref) FMCore_InputActionBindingHandle& IABindingHandle,
		FText OverrideDisplayName = FText::GetEmpty(),
		bool bShouldDisplayInActionBar = true);

	/**
	 * Unregister all input bindings tracked by this widget.
	 * Called automatically on deactivation. Safe to call manually.
	 */
	UFUNCTION(BlueprintCallable, Category="UI|Input")
	void UnregisterAllBindings();

#if WITH_EDITOR
	// ============================================================================
	// EDITOR VALIDATION
	// ============================================================================

	/**
	 * Validates GetDesiredFocusTarget implementation at compile time.
	 * Prevents common mistake of missing focus target for gamepad navigation.
	 */
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree,
		class IWidgetCompilerLog& CompileLog) const override;
#endif

protected:
	// ============================================================================
	// LIFECYCLE
	// ============================================================================

	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// THEME
	// ============================================================================

	/** Apply theme styling to this widget. Override for custom theme handling. */
	UFUNCTION(BlueprintNativeEvent, Category = "ModulusCore|Theme")
	void ApplyTheme(UMCore_PDA_UITheme_Base* NewTheme);
	virtual void ApplyTheme_Implementation(UMCore_PDA_UITheme_Base* NewTheme);

	UFUNCTION(BlueprintImplementableEvent, Category = "ModulusCore|Theme", meta = (DisplayName = "On Theme Applied"))
	void K2_OnThemeApplied(UMCore_PDA_UITheme_Base* Theme);

	/* Check if activation should be blocked based on OwningPlayer's tags */
	bool bShouldBlockActivation() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI|Activation")
	bool bShouldFocusOnActivation{false};

	UPROPERTY(Transient)
	mutable TWeakObjectPtr<UMCore_PDA_UITheme_Base> CachedTheme;

private:
	UPROPERTY(Transient)
	TArray<FUIActionBindingHandle> IABindingHandles;

	UFUNCTION()
	void HandleThemeChanged(UMCore_PDA_UITheme_Base* NewTheme);

	void BindThemeDelegate();
	void UnbindThemeDelegate();

	bool bThemeDelegateBound{false};

	/* Saved during deactivation for focus restore on reactivation */
	TWeakObjectPtr<UWidget> SavedFocusTarget;
};
