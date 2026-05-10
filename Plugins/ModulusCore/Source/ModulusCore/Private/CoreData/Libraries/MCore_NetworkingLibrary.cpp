// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Libraries/MCore_NetworkingLibrary.h"

#include "Blueprint/UserWidget.h"

EMCore_NetMode UMCore_NetworkingLibrary::GetNetworkMode(const UObject* WorldContextObject)
{
	if (!WorldContextObject) { return EMCore_NetMode::Standalone; }

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) { return EMCore_NetMode::Standalone; }

	switch (World->GetNetMode())
	{
	case NM_DedicatedServer:
		return EMCore_NetMode::DedicatedServer;
	case NM_ListenServer:
		return EMCore_NetMode::ListenServer;
	case NM_Client:
		return EMCore_NetMode::Client;
	case NM_Standalone:
	default:
		return EMCore_NetMode::Standalone;
	}
}

bool UMCore_NetworkingLibrary::IsServer(const UObject* WorldContextObject)
{
	const EMCore_NetMode CurrentNetMode = GetNetworkMode(WorldContextObject);
	return CurrentNetMode == EMCore_NetMode::DedicatedServer || CurrentNetMode == EMCore_NetMode::ListenServer;
}

bool UMCore_NetworkingLibrary::IsClient(const UObject* WorldContextObject)
{
	const EMCore_NetMode CurrentNetMode = GetNetworkMode(WorldContextObject);
	return CurrentNetMode == EMCore_NetMode::Client || CurrentNetMode == EMCore_NetMode::ListenServer;
}

bool UMCore_NetworkingLibrary::IsDedicatedServer(const UObject* WorldContextObject)
{
	const EMCore_NetMode CurrentNetMode = GetNetworkMode(WorldContextObject);
	return CurrentNetMode == EMCore_NetMode::DedicatedServer;
}

bool UMCore_NetworkingLibrary::IsListenServer(const UObject* WorldContextObject)
{
	const EMCore_NetMode CurrentNetMode = GetNetworkMode(WorldContextObject);
	return CurrentNetMode == EMCore_NetMode::ListenServer;
}

bool UMCore_NetworkingLibrary::Widget_HasNetworkAuthority(const UUserWidget* Widget)
{
	if (!Widget) { return false; }

	APawn* OwningPawn = Widget_GetOwningPawn(Widget);
	return OwningPawn ? OwningPawn->HasAuthority() : false;
}

bool UMCore_NetworkingLibrary::Widget_CanExecuteServerOperation(const UUserWidget* Widget)
{
	if (!Widget) { return false; }

	APlayerController* OwningPlayerController = Widget_GetOwningPlayerController(Widget);
	return OwningPlayerController != nullptr;
}

bool UMCore_NetworkingLibrary::Widget_CanExecuteClientOperation(const UUserWidget* Widget)
{
	if (!Widget) { return false; }

	APlayerController* OwningPlayerController = Widget_GetOwningPlayerController(Widget);
	if (!OwningPlayerController) { return false; }

	UWorld* World = Widget->GetWorld();
	if (!World) { return false; }

	return World->GetNetMode() != NM_DedicatedServer;
}

APlayerController* UMCore_NetworkingLibrary::Widget_GetOwningPlayerController(const UUserWidget* Widget)
{
	return Widget ? Widget->GetOwningPlayer() : nullptr;
}

APawn* UMCore_NetworkingLibrary::Widget_GetOwningPawn(const UUserWidget* Widget)
{
	APlayerController* OwningPlayerController = Widget_GetOwningPlayerController(Widget);
	return OwningPlayerController ? OwningPlayerController->GetPawn() : nullptr;
}

bool UMCore_NetworkingLibrary::Subsystem_IsServer(const USubsystem* Subsystem)
{
	return Subsystem ? IsServer(Subsystem) : false;
}

bool UMCore_NetworkingLibrary::Subsystem_CanExecuteServerOperation(const USubsystem* Subsystem)
{
	return Subsystem_IsServer(Subsystem);
}
