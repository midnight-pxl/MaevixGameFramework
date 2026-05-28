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
	DetectNetworkingSystem();
	OnNetworkInitialized();

	UE_LOG(LogMaevixNetworking, Verbose, TEXT("NetworkingComponent::BeginPlay: initialized: %s "
		"(Authority: %s, Iris: %s)"), *GetClass()->GetName(),
		HasNetworkAuthority() ? TEXT("Yes") : TEXT("No"),
		bIrisDetected ? TEXT("Yes") : TEXT("No"));
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

void UMCore_NetworkingComponent::DetectNetworkingSystem()
{
	/* Honesty stub: Iris replication detection is targeted for v1.X once Iris graduates Beta.
	 * See MAEVIXCORE_WORK_TRACKER.md T-13 / T-19. */
	bIrisDetected = false;
}

bool UMCore_NetworkingComponent::IsUsingIrisReplication() const
{
	return bIrisDetected;
}

bool UMCore_NetworkingComponent::IsIrisAvailable() const
{
	/* Honesty stub: Iris availability detection is targeted for v1.X once Iris graduates Beta.
	 * See MAEVIXCORE_WORK_TRACKER.md T-13 / T-19. */
	return false;
}

void UMCore_NetworkingComponent::ForceNetUpdate()
{
	if (!HasNetworkAuthority()) { return; }

	if (AActor* Owner = GetOwner()) { Owner->ForceNetUpdate(); }
}
