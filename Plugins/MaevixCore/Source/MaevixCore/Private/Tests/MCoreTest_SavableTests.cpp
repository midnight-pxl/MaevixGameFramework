// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/MCoreTest_SavableFixtures.h"
#include "CoreData/Interfaces/MCore_Savable.h"
#include "CoreData/Types/Persistence/MCore_SaveLoadContext.h"
#include "CoreData/Libraries/MCore_ServiceRegistryLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

namespace
{
	// Headless GameInstance stand-in (zero LocalPlayers -> facade routes to Global). Mirrors the service-registry
	// harness's helper; duplicated here to keep this file self-contained (no existing file touched).
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Savable_RegisterResolveByInterface,
	"Maevix.Persistence.Savable.RegisterResolveByInterface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Savable_RegisterResolveByInterface::RunTest(const FString& Parameters)
{
	FMCoreTest_StandaloneGI Fixture;
	UObject* WC = Fixture.World();
	TestNotNull(TEXT("Standalone world"), WC);
	if (!WC) { return false; }

	UMCoreTest_SavableProvider* Provider = NewObject<UMCoreTest_SavableProvider>(Fixture.GI);
	const FGuid Stamped = Provider->SaveGuid;
	TestTrue(TEXT("Stamped guid valid"), Stamped.IsValid());

	// Register and resolve BY the IMCore_Savable interface, through the facade (Global scope).
	FMCore_ServiceHandle Handle = UMCore_ServiceRegistryLibrary::RegisterService(
		WC, UMCore_Savable::StaticClass(), Provider, FGameplayTag());
	TestTrue(TEXT("Register handle valid"), Handle.IsValid());

	UObject* Out = nullptr;
	const bool bResolved = UMCore_ServiceRegistryLibrary::ResolveService(
		WC, UMCore_Savable::StaticClass(), FGameplayTag(), Out);
	TestTrue(TEXT("ResolveService returned true"), bResolved);
	TestTrue(TEXT("Resolved object is the provider"), Out == Provider);

	// Call the contract on the type-erased UObject via the interface (the orchestrator pattern).
	const bool bImplements = Out && Out->Implements<UMCore_Savable>();
	TestTrue(TEXT("Resolved implements IMCore_Savable"), bImplements);
	if (bImplements)
	{
		const FGuid ResolvedGuid = IMCore_Savable::Execute_GetSaveGuid(Out);
		TestTrue(TEXT("Execute_GetSaveGuid matches stamped guid"), ResolvedGuid == Stamped);
	}

	// Unregister invalidates the handle.
	UMCore_ServiceRegistryLibrary::UnregisterService(Handle);
	TestFalse(TEXT("Handle invalid after unregister"), Handle.IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Savable_ReflectedRoundTripWithHooks,
	"Maevix.Persistence.Savable.ReflectedRoundTripWithHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Savable_ReflectedRoundTripWithHooks::RunTest(const FString& Parameters)
{
	UMCoreTest_SavableProvider* Provider = NewObject<UMCoreTest_SavableProvider>(GetTransientPackage());
	Provider->RuntimeValue = 42;
	Provider->TransientNoise = 7;

	// Hook flushes the live value into the SaveGame field; stamp the context so we can prove it arrived intact.
	FMCore_SaveLoadContext SaveCtx;
	SaveCtx.Phase = EMCore_SaveLoadPhase::Saving;
	SaveCtx.SlotName = FString(TEXT("MaevixTestSlot"));
	SaveCtx.UserIndex = 0;
	IMCore_Savable::Execute_OnPreSave(Provider, SaveCtx);
	TestTrue(TEXT("OnPreSave fired"), Provider->bPreSaveFired);
	TestEqual(TEXT("SavedValue flushed from RuntimeValue"), Provider->SavedValue, 42);
	TestTrue(TEXT("OnPreSave saw Saving phase"), Provider->PreSavePhaseSeen == EMCore_SaveLoadPhase::Saving);
	TestTrue(TEXT("OnPreSave saw the slot name"), Provider->PreSaveSlotSeen == FString(TEXT("MaevixTestSlot")));

	// Serialize only the SaveGame fields to bytes.
	TArray<uint8> Bytes;
	{
		FMemoryWriter MemWriter(Bytes, /*bIsPersistent*/ true);
		FObjectAndNameAsStringProxyArchive Ar(MemWriter, /*bLoadIfFindFails*/ false);
		Ar.ArIsSaveGame = true;
		Ar.ArNoDelta = true;
		Provider->Serialize(Ar);
	}

	// Corrupt every field; only the SaveGame field should be restored by the read.
	Provider->SavedValue = 0;
	Provider->RuntimeValue = 0;
	Provider->TransientNoise = 999;

	{
		FMemoryReader MemReader(Bytes, /*bIsPersistent*/ true);
		FObjectAndNameAsStringProxyArchive Ar(MemReader, /*bLoadIfFindFails*/ true);
		Ar.ArIsSaveGame = true;
		Ar.ArNoDelta = true;
		Provider->Serialize(Ar);
	}

	TestEqual(TEXT("SaveGame field round-tripped"), Provider->SavedValue, 42);
	TestEqual(TEXT("Non-SaveGame field untouched by the ArIsSaveGame filter"), Provider->TransientNoise, 999);

	// Hook rehydrates the live value from the restored field; stamp the context for the same intact-arrival proof.
	FMCore_SaveLoadContext LoadCtx;
	LoadCtx.Phase = EMCore_SaveLoadPhase::Loading;
	LoadCtx.SlotName = FString(TEXT("MaevixTestSlot"));
	IMCore_Savable::Execute_OnPostLoad(Provider, LoadCtx);
	TestTrue(TEXT("OnPostLoad fired"), Provider->bPostLoadFired);
	TestEqual(TEXT("RuntimeValue rehydrated from SavedValue"), Provider->RuntimeValue, 42);
	TestTrue(TEXT("OnPostLoad saw Loading phase"), Provider->PostLoadPhaseSeen == EMCore_SaveLoadPhase::Loading);
	TestTrue(TEXT("OnPostLoad saw the slot name"), Provider->PostLoadSlotSeen == FString(TEXT("MaevixTestSlot")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
