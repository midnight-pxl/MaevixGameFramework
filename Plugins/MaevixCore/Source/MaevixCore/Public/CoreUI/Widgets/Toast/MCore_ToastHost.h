// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "MCore_ToastHost.generated.h"

class UPanelWidget;

/**
 * Non-activatable container of named anchor panels, hosted as a BindWidgetOptional
 * slot inside UMCore_PrimaryGameLayout. Maps the seven toast anchor tags to the
 * vertical-stacking panels authored in the WBP. Custom anchors are added by
 * subclassing the host WBP and overriding BuildAnchorMap (a compile time seam).
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_ToastHost : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** Panel bound for AnchorTag, or nullptr if this host has no such anchor. */
	UPanelWidget* GetAnchorPanel(FGameplayTag AnchorTag) const;

	bool HasAnchor(FGameplayTag AnchorTag) const;

protected:
	virtual void NativeOnInitialized() override;

	/** Fill AnchorMap from the bound panels. Override (call Super) to register custom anchors. */
	virtual void BuildAnchorMap();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_TopLeft;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_TopCenter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_TopRight;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_MiddleCenter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_BottomLeft;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_BottomCenter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> Panel_BottomRight;

	/** Resolved anchor tag to panel. Built once in NativeOnInitialized from bound panels. */
	TMap<FGameplayTag, TObjectPtr<UPanelWidget>> AnchorMap;
};
