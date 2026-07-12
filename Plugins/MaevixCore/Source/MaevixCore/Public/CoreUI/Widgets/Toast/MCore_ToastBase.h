// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreUI/Widgets/Primitives/MCore_ContainerBase.h"
#include "CoreData/Types/UI/MCore_ToastTypes.h"
#include "MCore_ToastBase.generated.h"

class UCommonTextBlock;
class UImage;
class UMCore_ToastBase;

/** Native completion delegates the service binds to drive toast lifetime. */
DECLARE_MULTICAST_DELEGATE_OneParam(FMCore_OnToastIntroComplete, UMCore_ToastBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FMCore_OnToastOutroComplete, UMCore_ToastBase*);

/**
 * Base class for a single toast: non-activatable, themed (inherits the
 * ContainerBase OnThemeChanged binding), populated from a request struct, with
 * Blueprint intro/outro animation hooks and a native completion handshake.
 * The widget owns no timers; the subsystem arms a watchdog when an awaited
 * animation defers completion.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = "MaevixUI", meta = (DisableNativeTick))
class MAEVIXCORE_API UMCore_ToastBase : public UMCore_ContainerBase
{
	GENERATED_BODY()

public:
	/** Bound by the service; not Blueprint exposed. Fired at most once each. */
	FMCore_OnToastIntroComplete OnIntroComplete;
	FMCore_OnToastOutroComplete OnOutroComplete;

	/**
	 * Returns true when completion is deferred to a WBP animation (subsystem arms a
	 * watchdog), false when it completed synchronously during this call.
	 */
	bool PlayIntro();
	bool PlayOutro();

	/**
	 * Populate visuals from the request. Default fills Txt_Title / Txt_Body / Img_Icon
	 * and collapses the body and icon when empty.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "MaevixCore|UI|Toast")
	void PopulateToast(const FMCore_ToastRequest& Request);
	virtual void PopulateToast_Implementation(const FMCore_ToastRequest& Request);

	/** Call from a WBP intro animation Finished event when bAwaitIntroAnimation is true. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI|Toast")
	void NotifyIntroComplete();

	/** Call from a WBP outro animation Finished event when bAwaitOutroAnimation is true. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI|Toast")
	void NotifyOutroComplete();

protected:
	/** WBP intro animation hook. Call NotifyIntroComplete when it finishes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "MaevixCore|UI|Toast", meta = (DisplayName = "On Play Intro"))
	void K2_OnPlayIntro();

	/** WBP outro animation hook. Call NotifyOutroComplete when it finishes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "MaevixCore|UI|Toast", meta = (DisplayName = "On Play Outro"))
	void K2_OnPlayOutro();

	/** Extra WBP population, e.g. swap colors by StyleVariant using the cached theme. */
	UFUNCTION(BlueprintImplementableEvent, Category = "MaevixCore|UI|Toast", meta = (DisplayName = "On Populate Toast"))
	void K2_OnPopulateToast(const FMCore_ToastRequest& Request);

	/** Set true when the WBP plays an intro animation and will call NotifyIntroComplete. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|UI|Toast")
	bool bAwaitIntroAnimation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MaevixCore|UI|Toast")
	bool bAwaitOutroAnimation = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Title;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Body;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Icon;

private:
	/* Idempotency guards: Notify*Complete fires its delegate at most once, so a late
	   real Notify after the subsystem watchdog forced completion is a harmless no-op. */
	bool bIntroDone = false;
	bool bOutroDone = false;
};
