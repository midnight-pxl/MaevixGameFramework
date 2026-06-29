// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreNetworking/MCore_NetworkingComponent.h"

#include "CoreData/Logging/LogMaevixNetworking.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UMCore_NetworkingComponent::UMCore_NetworkingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMCore_NetworkingComponent::BeginPlay()
{
	Super::BeginPlay();

	/* Read authority once here purely to publish the init-time verdict. This is a transient
	   local, not a cache; ExecuteWithAuthority re-reads authority live on every call and
	   never consults it. */
	const bool bHasAuthority = HasNetworkAuthority();

	OnNetworkInitialized();
	OnAuthorityChanged(bHasAuthority);

	UE_LOG(LogMaevixNetworking, Verbose, TEXT("NetworkingComponent::BeginPlay: initialized %s (Authority: %s)"),
		*GetClass()->GetName(), bHasAuthority ? TEXT("Yes") : TEXT("No"));
}

void UMCore_NetworkingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnNetworkShutdown();
	Super::EndPlay(EndPlayReason);
}

void UMCore_NetworkingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ComponentNetworkID);
}

void UMCore_NetworkingComponent::ForceNetUpdate()
{
	if (!HasNetworkAuthority()) { return; }

	if (AActor* Owner = GetOwner()) { Owner->ForceNetUpdate(); }
}
