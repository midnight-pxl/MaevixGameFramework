// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/MCoreTest_ServiceRegistryFixtures.h"
#include "CoreData/Libraries/MCore_ServiceRegistryLibrary.h"
#include "CoreServiceRegistry/MCore_GlobalServiceRegistrySubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

namespace
{
	// Headless GameInstance with a dummy world + initialized GameInstance subsystems and ZERO LocalPlayers,
	// so a world WorldContext derives no LocalPlayer and the facade routes to Global (dedicated-server stand-in).
	struct FMCoreTest_StandaloneGI
	{
		UGameInstance* GI = nullptr;
		FMCoreTest_StandaloneGI()
		{
			GI = NewObject<UGameInstance>(GEngine);
			GI->AddToRoot();
			GI->InitializeStandalone();
		}
		~FMCoreTest_StandaloneGI()
		{
			if (GI) { GI->Shutdown(); GI->RemoveFromRoot(); GI = nullptr; }
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}
		UWorld* World() const { return GI ? GI->GetWorld() : nullptr; }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_SR_FacadeGlobalRoundTrip,
	"Maevix.ServiceRegistry.Facade.GlobalRegisterResolveUnregister",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_SR_FacadeGlobalRoundTrip::RunTest(const FString& Parameters)
{
	FMCoreTest_StandaloneGI Fixture;
	UObject* WC = Fixture.World();
	TestNotNull(TEXT("Standalone world"), WC);
	if (!WC) { return false; }
	const FGameplayTag NoTag;

	UMCoreTest_PingProvider* Provider = NewObject<UMCoreTest_PingProvider>(Fixture.GI);

	// Templated RegisterService<IX> -> valid handle (forces template instantiation: the hard requirement).
	FMCore_ServiceHandle Handle =
		UMCore_ServiceRegistryLibrary::RegisterService<IMCoreTest_PingInterface>(WC, Provider, NoTag);
	TestTrue(TEXT("Register handle valid"), Handle.IsValid());

	// Resolve<IX> -> non-null interface ptr and the same object.
	TScriptInterface<IMCoreTest_PingInterface> Resolved =
		UMCore_ServiceRegistryLibrary::Resolve<IMCoreTest_PingInterface>(WC);
	TestNotNull(TEXT("Resolve interface ptr"), Resolved.GetInterface());
	TestTrue(TEXT("Resolved object is provider"), Resolved.GetObject() == Provider);

	/* Unregister -> Resolve<IX> empty. */
	UMCore_ServiceRegistryLibrary::UnregisterService(Handle);
	TestNull(TEXT("Resolve null after unregister"),
		UMCore_ServiceRegistryLibrary::Resolve<IMCoreTest_PingInterface>(WC).GetInterface());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_SR_FacadeImplementsGate,
	"Maevix.ServiceRegistry.Facade.ImplementsInterfaceGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_SR_FacadeImplementsGate::RunTest(const FString& Parameters)
{
	FMCoreTest_StandaloneGI Fixture;
	UObject* WC = Fixture.World();
	if (!WC) { return false; }

	// Non-implementer via the BP UClass-keyed RegisterService (the template can't express a non-implementer).
	UMCoreTest_NonProvider* NonProvider = NewObject<UMCoreTest_NonProvider>(Fixture.GI);
	FMCore_ServiceHandle Handle = UMCore_ServiceRegistryLibrary::RegisterService(
		WC, UMCoreTest_PingInterface::StaticClass(), NonProvider, FGameplayTag());
	TestFalse(TEXT("Non-implementer rejected (invalid handle)"), Handle.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_SR_GlobalOnServiceRegistered,
	"Maevix.ServiceRegistry.Facade.GlobalOnServiceRegistered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_SR_GlobalOnServiceRegistered::RunTest(const FString& Parameters)
{
	FMCoreTest_StandaloneGI Fixture;
	UObject* WC = Fixture.World();
	if (!WC) { return false; }
	UMCore_GlobalServiceRegistrySubsystem* Global = Fixture.GI->GetSubsystem<UMCore_GlobalServiceRegistrySubsystem>();
	TestNotNull(TEXT("Global subsystem present"), Global);
	if (!Global) { return false; }
	const FGameplayTag NoTag;

	// Bind a listener; it re-resolves (read) inside the callback per the documented discipline.
	UMCoreTest_Listener* Listener = NewObject<UMCoreTest_Listener>(Fixture.GI);
	Listener->ReResolveWorldContext = WC;
	Global->OnServiceRegistered.AddDynamic(Listener, &UMCoreTest_Listener::HandleServiceRegistered);

	// Resolve early -> null.
	TestNull(TEXT("Early resolve null"),
		UMCore_ServiceRegistryLibrary::Resolve<IMCoreTest_PingInterface>(WC).GetInterface());

	// Register -> listener fires once with matching args and re-resolved non-null inside the handler.
	UMCoreTest_PingProvider* Provider = NewObject<UMCoreTest_PingProvider>(Fixture.GI);
	FMCore_ServiceHandle Handle =
		UMCore_ServiceRegistryLibrary::RegisterService<IMCoreTest_PingInterface>(WC, Provider, NoTag);
	TestTrue(TEXT("Register valid"), Handle.IsValid());
	TestEqual(TEXT("Listener fired once"), Listener->FireCount, 1);
	TestTrue(TEXT("Listener interface matches"), Listener->LastInterface == UMCoreTest_PingInterface::StaticClass());
	TestTrue(TEXT("Listener discriminator matches"), Listener->LastDiscriminator == NoTag);
	TestTrue(TEXT("Handler re-resolved non-null"), Listener->bReResolvedNonNull);

	// Resolve now -> non-null.
	TestNotNull(TEXT("Resolve non-null after register"),
		UMCore_ServiceRegistryLibrary::Resolve<IMCoreTest_PingInterface>(WC).GetInterface());

	// Unbind, then a fresh successful register must NOT fire the listener again.
	Global->OnServiceRegistered.RemoveDynamic(Listener, &UMCoreTest_Listener::HandleServiceRegistered);
	UMCore_ServiceRegistryLibrary::UnregisterService(Handle);   // free the (interface, NoTag) slot for an Added
	UMCoreTest_PingProvider* Provider2 = NewObject<UMCoreTest_PingProvider>(Fixture.GI);
	FMCore_ServiceHandle Handle2 =
		UMCore_ServiceRegistryLibrary::RegisterService<IMCoreTest_PingInterface>(WC, Provider2, NoTag);
	TestTrue(TEXT("Second register valid"), Handle2.IsValid());
	TestEqual(TEXT("Listener did not fire after unbind"), Listener->FireCount, 1);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
