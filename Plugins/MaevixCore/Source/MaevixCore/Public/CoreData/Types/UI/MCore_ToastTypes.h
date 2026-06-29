// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MCore_ToastTypes.generated.h"

class UMCore_ToastBase;
class UTexture2D;

/** Delivery scope for a toast request. Toasts never replicate; AllLocal fans the
 *  same request to every LocalPlayer's own service on this client. */
UENUM(BlueprintType)
enum class EMCore_ToastScope : uint8
{
	/* Explicit values: toasts never persist or replicate, but the house style
	   (see EMCore_EventScope) pins enum values for forward safety. */
	LocalPlayer = 0 UMETA(DisplayName = "Local Player (This Player)"),
	AllLocal    = 1 UMETA(DisplayName = "All Local Players")
};

/** Opaque handle to a submitted toast. Pass to DismissToast. Scoped to the
 *  submitting LocalPlayer's service. For AllLocal it refers to the caller's own
 *  player; sibling players' copies expire by Duration. */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_ToastHandle
{
	GENERATED_BODY()

public:
	FMCore_ToastHandle() : HandleID(0) {}
	explicit FMCore_ToastHandle(uint32 InID) : HandleID(InID) {}

	bool IsValid() const { return HandleID != 0; }
	uint32 GetID() const { return HandleID; }
	void Invalidate() { HandleID = 0; }

	bool operator==(const FMCore_ToastHandle& Other) const { return HandleID == Other.HandleID; }
	bool operator!=(const FMCore_ToastHandle& Other) const { return HandleID != Other.HandleID; }

private:
	UPROPERTY()
	uint32 HandleID;
};

/** Caller-submitted toast content and policy. Only Title is required.
 *  StyleVariant is interpreted by the toast WBP (mapped to visuals via the cached
 *  theme); an empty tag is treated as the WBP's default style. Empty AnchorTag
 *  resolves to the configured default anchor. Duration <= 0 means persistent until
 *  DismissToast. */
USTRUCT(BlueprintType)
struct MAEVIXCORE_API FMCore_ToastRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast")
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast",
		meta = (Categories = "MCore.UI.Toast.Style"))
	FGameplayTag StyleVariant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast",
		meta = (Categories = "MCore.UI.Toast.Anchor"))
	FGameplayTag AnchorTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast")
	EMCore_ToastScope Scope = EMCore_ToastScope::LocalPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast", meta = (Units = "s"))
	float Duration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast")
	FName DedupKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaevixCore|UI|Toast")
	TSubclassOf<UMCore_ToastBase> WidgetOverride;
};
