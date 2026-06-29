// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Libraries/MCore_ToastFunctionLibrary.h"

#include "CoreUI/MCore_ToastSubsystem.h"
#include "CoreData/Logging/LogMaevixToast.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

ULocalPlayer* UMCore_ToastFunctionLibrary::ResolveLocalPlayer(const UObject* WorldContext)
{
	/* Mirrors UMCore_EventFunctionLibrary::ResolveLocalPlayer, including the deliberate
	   absence of a Player 0 fallback so split-screen never contaminates the wrong player. */
	if (!WorldContext)
	{
		return nullptr;
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(WorldContext))
	{
		return PlayerController->GetLocalPlayer();
	}

	if (const APawn* Pawn = Cast<APawn>(WorldContext))
	{
		if (const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			return PlayerController->GetLocalPlayer();
		}
	}

	if (const UActorComponent* Component = Cast<UActorComponent>(WorldContext))
	{
		return ResolveLocalPlayer(Component->GetOwner());
	}

	if (const UUserWidget* Widget = Cast<UUserWidget>(WorldContext))
	{
		return Widget->GetOwningLocalPlayer();
	}

	if (const AActor* Actor = Cast<AActor>(WorldContext))
	{
		if (const APawn* Instigator = Actor->GetInstigator())
		{
			if (const APlayerController* PlayerController = Cast<APlayerController>(Instigator->GetController()))
			{
				return PlayerController->GetLocalPlayer();
			}
		}

		AActor* OwnerActor = Actor->GetOwner();
		while (OwnerActor)
		{
			if (const APlayerController* PlayerController = Cast<APlayerController>(OwnerActor))
			{
				return PlayerController->GetLocalPlayer();
			}
			OwnerActor = OwnerActor->GetOwner();
		}
	}

	UE_LOG(LogMaevixToast, Warning,
		TEXT("ToastFunctionLibrary::ResolveLocalPlayer: WorldContext '%s' has no LocalPlayer owner chain. ")
		TEXT("Pass a PlayerController, Pawn, or Widget, or use EMCore_ToastScope::AllLocal."),
		*GetNameSafe(WorldContext));
	return nullptr;
}

FMCore_ToastHandle UMCore_ToastFunctionLibrary::RequestToast(const UObject* WorldContext, const FMCore_ToastRequest& Request)
{
	if (!WorldContext)
	{
		return FMCore_ToastHandle();
	}

	switch (Request.Scope)
	{
	case EMCore_ToastScope::LocalPlayer:
	{
		ULocalPlayer* LocalPlayer = ResolveLocalPlayer(WorldContext);
		if (!LocalPlayer)
		{
			return FMCore_ToastHandle();
		}
		if (UMCore_ToastSubsystem* Subsystem = LocalPlayer->GetSubsystem<UMCore_ToastSubsystem>())
		{
			return Subsystem->RequestToastInternal(Request);
		}
		return FMCore_ToastHandle();
	}

	case EMCore_ToastScope::AllLocal:
	{
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		if (!GameInstance)
		{
			return FMCore_ToastHandle();
		}

		/* The handle refers only to the caller's own player; sibling copies expire by Duration. */
		const ULocalPlayer* CallerPlayer = ResolveLocalPlayer(WorldContext);
		FMCore_ToastHandle CallerHandle;

		for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
		{
			if (!LocalPlayer)
			{
				continue;
			}
			if (UMCore_ToastSubsystem* Subsystem = LocalPlayer->GetSubsystem<UMCore_ToastSubsystem>())
			{
				const FMCore_ToastHandle Handle = Subsystem->RequestToastInternal(Request);
				if (LocalPlayer == CallerPlayer)
				{
					CallerHandle = Handle;
				}
			}
		}
		return CallerHandle;
	}

	default:
		return FMCore_ToastHandle();
	}
}

void UMCore_ToastFunctionLibrary::DismissToast(const UObject* WorldContext, FMCore_ToastHandle& Handle)
{
	if (!WorldContext || !Handle.IsValid())
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = ResolveLocalPlayer(WorldContext))
	{
		if (UMCore_ToastSubsystem* Subsystem = LocalPlayer->GetSubsystem<UMCore_ToastSubsystem>())
		{
			Subsystem->DismissToastInternal(Handle);
		}
	}

	/* Handle is spent: dismissing again is a no-op. Mirrors the extension subsystem handle idiom. */
	Handle.Invalidate();
}
