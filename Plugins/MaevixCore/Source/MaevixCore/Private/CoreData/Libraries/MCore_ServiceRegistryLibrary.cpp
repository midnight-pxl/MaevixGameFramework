// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Libraries/MCore_ServiceRegistryLibrary.h"

#include "CoreData/Logging/LogMaevixServices.h"
#include "CoreServiceRegistry/MCore_LocalServiceRegistrySubsystem.h"
#include "CoreServiceRegistry/MCore_GlobalServiceRegistrySubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

/* Resolves the LocalPlayer that owns WorldContext, trying in order:
 *   (1) a directly passed LocalPlayer;
 *   (2) PlayerController -> GetLocalPlayer();
 *   (3) Pawn -> Controller -> GetLocalPlayer();
 *   (4) ActorComponent -> recurse on its Owner;
 *   (5) UserWidget -> GetOwningLocalPlayer();
 *   (6) Actor -> Instigator -> Controller -> GetLocalPlayer(), then owner chain walk to the first PlayerController.
 * Returns nullptr (no log) when nothing resolves: that is the caller's signal to route to the
 * GameInstance scope. No Player 0 fallback, mirroring UMCore_EventFunctionLibrary::ResolveLocalPlayer
 * (split screen safety). */
static ULocalPlayer* MCore_DeriveLocalPlayer(const UObject* WorldContext)
{
	if (!WorldContext) { return nullptr; }

	if (ULocalPlayer* DirectLocalPlayer = const_cast<ULocalPlayer*>(Cast<ULocalPlayer>(WorldContext)))
	{
		return DirectLocalPlayer;
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
		return MCore_DeriveLocalPlayer(Component->GetOwner());
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

	/* No owner chain resolved to a specific LocalPlayer. Intentionally no Player 0 fallback (it would
	   contaminate split screen). The caller treats nullptr as route to the GameInstance scope. */
	return nullptr;
}

static UMCore_GlobalServiceRegistrySubsystem* MCore_GetGlobalRegistry(const UObject* WorldContext)
{
	UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull)
		: nullptr;
	if (!World) { return nullptr; }

	UGameInstance* GameInstance = World->GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMCore_GlobalServiceRegistrySubsystem>() : nullptr;
}

bool UMCore_ServiceRegistryLibrary::ResolveService(const UObject* WorldContext,
	TSubclassOf<UInterface> ServiceInterface, FGameplayTag Discriminator, UObject*& OutProvider)
{
	OutProvider = nullptr;

	if (!WorldContext || !ServiceInterface)
	{
		return false;
	}

	UClass* InterfaceClass = ServiceInterface.Get();

	/* Most specific first: try the caller's LocalPlayer scope, then fall back to the GameInstance scope so a
	   per player provider wins over a global one. */
	if (ULocalPlayer* LocalPlayer = MCore_DeriveLocalPlayer(WorldContext))
	{
		if (UMCore_LocalServiceRegistrySubsystem* LocalRegistry =
			LocalPlayer->GetSubsystem<UMCore_LocalServiceRegistrySubsystem>())
		{
			OutProvider = LocalRegistry->ResolveProvider(InterfaceClass, Discriminator);
		}
	}

	if (!OutProvider)
	{
		if (UMCore_GlobalServiceRegistrySubsystem* GlobalRegistry = MCore_GetGlobalRegistry(WorldContext))
		{
			OutProvider = GlobalRegistry->ResolveProvider(InterfaceClass, Discriminator);
		}
	}

	return OutProvider != nullptr;
}

FMCore_ServiceHandle UMCore_ServiceRegistryLibrary::RegisterService(const UObject* WorldContext,
	TSubclassOf<UInterface> ServiceInterface, UObject* Provider, FGameplayTag Discriminator)
{
	if (!WorldContext || !ServiceInterface || !Provider)
	{
		UE_LOG(LogMaevixServices, Warning,
			TEXT("ServiceRegistryLibrary::RegisterService: invalid arguments (WorldContext=%s, ServiceInterface=%s, Provider=%s)"),
			WorldContext ? TEXT("valid") : TEXT("null"),
			*GetNameSafe(ServiceInterface.Get()), *GetNameSafe(Provider));
		return FMCore_ServiceHandle();
	}

	UClass* InterfaceClass = ServiceInterface.Get();

	/* Gate at the facade so the BP and templated paths share it: a provider that does not implement the
	   interface would resolve back as an object that fails the interface cast, so refuse it up front. */
	if (!Provider->GetClass()->ImplementsInterface(InterfaceClass))
	{
		UE_LOG(LogMaevixServices, Warning,
			TEXT("ServiceRegistryLibrary::RegisterService: %s does not implement %s; registration refused."),
			*GetNameSafe(Provider), *GetNameSafe(InterfaceClass));
		return FMCore_ServiceHandle();
	}

	/* Route by scope: a player owned context registers in that LocalPlayer's registry, otherwise the
	   GameInstance registry. On a dedicated server no LocalPlayer is ever derivable, so this lands on Global. */
	if (ULocalPlayer* LocalPlayer = MCore_DeriveLocalPlayer(WorldContext))
	{
		if (UMCore_LocalServiceRegistrySubsystem* LocalRegistry =
			LocalPlayer->GetSubsystem<UMCore_LocalServiceRegistrySubsystem>())
		{
			return LocalRegistry->RegisterProvider(InterfaceClass, Discriminator, Provider);
		}

		UE_LOG(LogMaevixServices, Warning,
			TEXT("ServiceRegistryLibrary::RegisterService: LocalPlayer has no LocalServiceRegistrySubsystem; registration dropped."));
		return FMCore_ServiceHandle();
	}

	UMCore_GlobalServiceRegistrySubsystem* GlobalRegistry = MCore_GetGlobalRegistry(WorldContext);
	if (!GlobalRegistry)
	{
		UE_LOG(LogMaevixServices, Warning,
			TEXT("ServiceRegistryLibrary::RegisterService: no Global registry available for '%s'; registration dropped."),
			*GetNameSafe(WorldContext));
		return FMCore_ServiceHandle();
	}

	UE_LOG(LogMaevixServices, Verbose,
		TEXT("ServiceRegistryLibrary::RegisterService: no LocalPlayer derivable from '%s'; routing to the GameInstance scope."),
		*GetNameSafe(WorldContext));
	return GlobalRegistry->RegisterProvider(InterfaceClass, Discriminator, Provider);
}

void UMCore_ServiceRegistryLibrary::UnregisterService(FMCore_ServiceHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	/* The handle self routes: its weak OwningRegistry points at the exact subsystem that stamped it, so
	   unregister needs no WorldContext. A torn down subsystem (null weak ref) makes this a safe no-op. */
	USubsystem* OwningRegistry = Handle.OwningRegistry.Get();

	if (UMCore_LocalServiceRegistrySubsystem* LocalRegistry = Cast<UMCore_LocalServiceRegistrySubsystem>(OwningRegistry))
	{
		LocalRegistry->UnregisterById(Handle.GetID());
	}
	else if (UMCore_GlobalServiceRegistrySubsystem* GlobalRegistry = Cast<UMCore_GlobalServiceRegistrySubsystem>(OwningRegistry))
	{
		GlobalRegistry->UnregisterById(Handle.GetID());
	}

	Handle.Invalidate();
}
