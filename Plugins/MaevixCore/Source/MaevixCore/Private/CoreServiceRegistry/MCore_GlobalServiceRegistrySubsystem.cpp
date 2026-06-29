// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreServiceRegistry/MCore_GlobalServiceRegistrySubsystem.h"

#include "CoreData/Logging/LogMaevixServices.h"

void UMCore_GlobalServiceRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogMaevixServices, Verbose, TEXT("GlobalServiceRegistrySubsystem::Initialize: ready"));
}

FMCore_ServiceHandle UMCore_GlobalServiceRegistrySubsystem::RegisterProvider(
	UClass* InterfaceClass, FGameplayTag Discriminator, UObject* Provider)
{
	EMCore_ServiceRegisterResult Result = EMCore_ServiceRegisterResult::InvalidArguments;
	const uint32 NewId = Registry.Register(InterfaceClass, Discriminator, Provider, Result);

	const FString DiscriminatorName = Discriminator.IsValid() ? Discriminator.ToString() : TEXT("<none>");

	switch (Result)
	{
	case EMCore_ServiceRegisterResult::Added:
		UE_LOG(LogMaevixServices, Verbose,
			TEXT("GlobalServiceRegistrySubsystem::RegisterProvider: registered %s [%s] (id=%u)"),
			*GetNameSafe(InterfaceClass), *DiscriminatorName, NewId);
		break;

	case EMCore_ServiceRegisterResult::ReclaimedStale:
		UE_LOG(LogMaevixServices, Verbose,
			TEXT("GlobalServiceRegistrySubsystem::RegisterProvider: reclaimed stale slot for %s [%s] (id=%u)"),
			*GetNameSafe(InterfaceClass), *DiscriminatorName, NewId);
		break;

	case EMCore_ServiceRegisterResult::RejectedDuplicate:
		UE_LOG(LogMaevixServices, Warning,
			TEXT("GlobalServiceRegistrySubsystem::RegisterProvider: rejected %s as a duplicate for %s [%s]; a live provider is already registered."),
			*GetNameSafe(Provider), *GetNameSafe(InterfaceClass), *DiscriminatorName);
		break;

	case EMCore_ServiceRegisterResult::InvalidArguments:
		UE_LOG(LogMaevixServices, Warning,
			TEXT("GlobalServiceRegistrySubsystem::RegisterProvider: invalid arguments (InterfaceClass=%s, Provider=%s)"),
			*GetNameSafe(InterfaceClass), *GetNameSafe(Provider));
		break;
	}

	const FMCore_ServiceHandle Handle =
		(NewId != 0) ? FMCore_ServiceHandle(NewId, this) : FMCore_ServiceHandle();
	if (Handle.IsValid())
	{
		/* In process notification on this scope's subsystem. Covers Added and ReclaimedStale (valid handle);
		   skips RejectedDuplicate / InvalidArguments. Works on a dedicated server via the Global subsystem;
		   never touches the network. */
		OnServiceRegistered.Broadcast(InterfaceClass, Discriminator);
	}
	return Handle;
}

UObject* UMCore_GlobalServiceRegistrySubsystem::ResolveProvider(UClass* InterfaceClass, FGameplayTag Discriminator)
{
	return Registry.Resolve(InterfaceClass, Discriminator);
}

bool UMCore_GlobalServiceRegistrySubsystem::UnregisterById(uint32 RegistrationId)
{
	return Registry.Unregister(RegistrationId);
}
