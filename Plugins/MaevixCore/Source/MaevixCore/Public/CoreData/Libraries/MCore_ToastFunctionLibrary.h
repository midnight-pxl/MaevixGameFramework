// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreData/Types/UI/MCore_ToastTypes.h"
#include "MCore_ToastFunctionLibrary.generated.h"

class ULocalPlayer;

/**
 * Blueprint and C++ entry point for the toast service. Stateless statics that resolve
 * the LocalPlayer from a WorldContext and route to the per-player UMCore_ToastSubsystem.
 * LocalPlayer scope targets the WorldContext's player; AllLocal fans the same request
 * to every LocalPlayer's own service on this client.
 */
UCLASS()
class MAEVIXCORE_API UMCore_ToastFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Submit a toast. Only Request.Title is required. Returns a handle scoped to the caller's player. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI|Toast", meta = (DefaultToSelf = "WorldContext"))
	static FMCore_ToastHandle RequestToast(const UObject* WorldContext, const FMCore_ToastRequest& Request);

	/** Dismiss a toast on the WorldContext's player. Plays the outro, then removes it. */
	UFUNCTION(BlueprintCallable, Category = "MaevixCore|UI|Toast", meta = (DefaultToSelf = "WorldContext"))
	static void DismissToast(const UObject* WorldContext, UPARAM(ref) FMCore_ToastHandle& Handle);

private:
	// Resolve the owning LocalPlayer from a WorldContext. No Player 0 fallback (split-screen safe).
	static ULocalPlayer* ResolveLocalPlayer(const UObject* WorldContext);
};
