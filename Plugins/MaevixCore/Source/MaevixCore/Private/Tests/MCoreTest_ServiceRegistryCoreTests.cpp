// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Tests/MCoreTest_ServiceRegistryFixtures.h"
#include "CoreServiceRegistry/MCore_ServiceRegistryCore.h"
#include "UObject/Package.h"

namespace
{
	// A fresh transient provider whose only references are non-keeping (raw local + the registry weak ptr),
	// so MarkAsGarbage + CollectGarbage deterministically kills it.
	UMCoreTest_PingProvider* MakeProvider() { return NewObject<UMCoreTest_PingProvider>(GetTransientPackage()); }

	constexpr int32 AsInt(EMCore_ServiceRegisterResult R) { return static_cast<int32>(R); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_SR_CoreRegisterResolveUnregister,
	"Maevix.ServiceRegistry.Core.RegisterResolveUnregister",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_SR_CoreRegisterResolveUnregister::RunTest(const FString& Parameters)
{
	FMCore_ServiceRegistryCore Core;
	UClass* Key = UMCoreTest_PingInterface::StaticClass();
	const FGameplayTag NoTag;
	EMCore_ServiceRegisterResult Result = EMCore_ServiceRegisterResult::InvalidArguments;

	// Register into an empty key -> Added, id != 0; Resolve hit -> the provider.
	UMCoreTest_PingProvider* A = MakeProvider();
	const uint32 IdA = Core.Register(Key, NoTag, A, Result);
	TestTrue(TEXT("Added id non-zero"), IdA != 0);
	TestEqual(TEXT("Added outcome"), AsInt(Result), AsInt(EMCore_ServiceRegisterResult::Added));
	TestTrue(TEXT("Resolve returns A"), Core.Resolve(Key, NoTag) == A);

	// Duplicate over a LIVE incumbent -> RejectedDuplicate, id 0, no mutation.
	UMCoreTest_PingProvider* B = MakeProvider();
	const uint32 IdDup = Core.Register(Key, NoTag, B, Result);
	TestTrue(TEXT("Duplicate id zero"), IdDup == 0);
	TestEqual(TEXT("Duplicate outcome"), AsInt(Result), AsInt(EMCore_ServiceRegisterResult::RejectedDuplicate));
	TestTrue(TEXT("Incumbent unchanged"), Core.Resolve(Key, NoTag) == A);

	// Unregister valid id -> true then Resolve null; unknown id -> false.
	TestTrue(TEXT("Unregister A true"), Core.Unregister(IdA));
	TestTrue(TEXT("Resolve null after unregister"), Core.Resolve(Key, NoTag) == nullptr);
	TestFalse(TEXT("Unregister unknown false"), Core.Unregister(987654));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_SR_CoreInvalidArguments,
	"Maevix.ServiceRegistry.Core.InvalidArguments",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_SR_CoreInvalidArguments::RunTest(const FString& Parameters)
{
	FMCore_ServiceRegistryCore Core;
	EMCore_ServiceRegisterResult Result = EMCore_ServiceRegisterResult::Added;

	// Null interface -> InvalidArguments, id 0.
	const uint32 Id1 = Core.Register(nullptr, FGameplayTag(), MakeProvider(), Result);
	TestTrue(TEXT("Null interface id zero"), Id1 == 0);
	TestEqual(TEXT("Null interface outcome"), AsInt(Result), AsInt(EMCore_ServiceRegisterResult::InvalidArguments));

	// Null provider -> InvalidArguments, id 0.
	const uint32 Id2 = Core.Register(UMCoreTest_PingInterface::StaticClass(), FGameplayTag(), nullptr, Result);
	TestTrue(TEXT("Null provider id zero"), Id2 == 0);
	TestEqual(TEXT("Null provider outcome"), AsInt(Result), AsInt(EMCore_ServiceRegisterResult::InvalidArguments));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_SR_CoreStaleReclaimVsPruneThenAdded,
	"Maevix.ServiceRegistry.Core.StaleReclaimVsPruneThenAdded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_SR_CoreStaleReclaimVsPruneThenAdded::RunTest(const FString& Parameters)
{
	UClass* Key = UMCoreTest_PingInterface::StaticClass();
	const FGameplayTag NoTag;

	// Stale reclaim WITHOUT an intervening resolve -> ReclaimedStale (dead entry was still present).
	{
		FMCore_ServiceRegistryCore Core;
		EMCore_ServiceRegisterResult Result = EMCore_ServiceRegisterResult::InvalidArguments;
		UMCoreTest_PingProvider* Dead = MakeProvider();
		Core.Register(Key, NoTag, Dead, Result);
		Dead->MarkAsGarbage();
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);             // entry's weak ptr goes stale
		const uint32 Id = Core.Register(Key, NoTag, MakeProvider(), Result);   // no Resolve in between
		TestTrue(TEXT("Reclaim id non-zero"), Id != 0);
		TestEqual(TEXT("Reclaim outcome"), AsInt(Result), AsInt(EMCore_ServiceRegisterResult::ReclaimedStale));
	}

	// Prune-then-Added: Resolve (prunes on access) before re-register -> Added, NOT ReclaimedStale.
	// Added here vs ReclaimedStale above is the proof prune-on-access removed the entry (no Num() needed).
	{
		FMCore_ServiceRegistryCore Core;
		EMCore_ServiceRegisterResult Result = EMCore_ServiceRegisterResult::InvalidArguments;
		UMCoreTest_PingProvider* Dead = MakeProvider();
		Core.Register(Key, NoTag, Dead, Result);
		Dead->MarkAsGarbage();
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		TestTrue(TEXT("Resolve prunes -> null"), Core.Resolve(Key, NoTag) == nullptr);
		const uint32 Id = Core.Register(Key, NoTag, MakeProvider(), Result);
		TestTrue(TEXT("Post-prune id non-zero"), Id != 0);
		TestEqual(TEXT("Post-prune outcome Added"), AsInt(Result), AsInt(EMCore_ServiceRegisterResult::Added));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_SR_CorePruneStaleEntriesSweep,
	"Maevix.ServiceRegistry.Core.PruneStaleEntriesSweep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_SR_CorePruneStaleEntriesSweep::RunTest(const FString& Parameters)
{
	FMCore_ServiceRegistryCore Core;
	UClass* Key = UMCoreTest_PingInterface::StaticClass();
	const FGameplayTag NoTag;
	EMCore_ServiceRegisterResult Result = EMCore_ServiceRegisterResult::InvalidArguments;

	UMCoreTest_PingProvider* Dead = MakeProvider();
	Core.Register(Key, NoTag, Dead, Result);
	Dead->MarkAsGarbage();
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	Core.PruneStaleEntries();                                    // sweep reclaims the dead entry
	const uint32 Id = Core.Register(Key, NoTag, MakeProvider(), Result);
	TestTrue(TEXT("Post-sweep id non-zero"), Id != 0);
	TestEqual(TEXT("Post-sweep outcome Added"), AsInt(Result), AsInt(EMCore_ServiceRegisterResult::Added));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
