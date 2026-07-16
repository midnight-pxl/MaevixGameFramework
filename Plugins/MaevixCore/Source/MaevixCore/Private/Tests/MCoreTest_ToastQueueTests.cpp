// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreUI/MCore_ToastQueue.h"
#include "CoreData/Types/UI/MCore_ToastTypes.h"

namespace
{
	FMCore_ToastQueueEntry MakeEntry(uint32 Id, int32 Priority = 0, FName DedupKey = NAME_None)
	{
		FMCore_ToastQueueEntry Entry;
		Entry.HandleID = Id;
		Entry.Priority = Priority;
		Entry.DedupKey = DedupKey;
		Entry.bPersistent = false;
		return Entry;
	}

	constexpr int32 AsInt(EMCore_ToastAdmission Admission) { return static_cast<int32>(Admission); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_AdmitUnderCapShowsNow,
	"Maevix.UI.Toast.Queue.AdmitUnderCapShowsNow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_AdmitUnderCapShowsNow::RunTest(const FString& Parameters)
{
	FMCore_ToastQueue Queue;
	Queue.SetMaxVisible(2);

	uint32 Existing = 0;
	TestEqual(TEXT("Admit below cap is ShowNow"), AsInt(Queue.Admit(MakeEntry(1), Existing)), AsInt(EMCore_ToastAdmission::ShowNow));
	TestEqual(TEXT("Active is 1"), Queue.NumActive(), 1);
	TestEqual(TEXT("Pending is 0"), Queue.NumPending(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_AdmitOverCapQueuesFIFO,
	"Maevix.UI.Toast.Queue.AdmitOverCapQueuesFIFO",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_AdmitOverCapQueuesFIFO::RunTest(const FString& Parameters)
{
	FMCore_ToastQueue Queue;
	Queue.SetMaxVisible(1);

	uint32 Existing = 0;
	Queue.Admit(MakeEntry(1), Existing);
	TestEqual(TEXT("Second admit at cap is Queued"), AsInt(Queue.Admit(MakeEntry(2), Existing)), AsInt(EMCore_ToastAdmission::Queued));
	TestEqual(TEXT("Third admit at cap is Queued"), AsInt(Queue.Admit(MakeEntry(3), Existing)), AsInt(EMCore_ToastAdmission::Queued));
	TestEqual(TEXT("Active is 1"), Queue.NumActive(), 1);
	TestEqual(TEXT("Pending is 2"), Queue.NumPending(), 2);

	/* Same priority: promotion order is submission order. */
	Queue.RemoveActive(1);
	FMCore_ToastQueueEntry Promoted;
	TestTrue(TEXT("PromoteNext succeeds"), Queue.PromoteNext(Promoted));
	TestEqual(TEXT("FIFO promotes 2 first"), static_cast<int32>(Promoted.HandleID), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_PriorityInsertionOrder,
	"Maevix.UI.Toast.Queue.PriorityInsertionOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_PriorityInsertionOrder::RunTest(const FString& Parameters)
{
	FMCore_ToastQueue Queue;
	Queue.SetMaxVisible(1);

	uint32 Existing = 0;
	Queue.Admit(MakeEntry(1, 0), Existing);   /* active */
	Queue.Admit(MakeEntry(2, 0), Existing);   /* pending, priority 0 */
	Queue.Admit(MakeEntry(3, 5), Existing);   /* pending, priority 5 */
	Queue.Admit(MakeEntry(4, 5), Existing);   /* pending, priority 5 (after 3) */

	FMCore_ToastQueueEntry Promoted;
	Queue.RemoveActive(1);
	Queue.PromoteNext(Promoted);
	TestEqual(TEXT("Highest priority promotes first"), static_cast<int32>(Promoted.HandleID), 3);
	Queue.RemoveActive(3);
	Queue.PromoteNext(Promoted);
	TestEqual(TEXT("Equal priority is FIFO"), static_cast<int32>(Promoted.HandleID), 4);
	Queue.RemoveActive(4);
	Queue.PromoteNext(Promoted);
	TestEqual(TEXT("Lowest priority promotes last"), static_cast<int32>(Promoted.HandleID), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_DedupKeyRefreshesNoNewSlot,
	"Maevix.UI.Toast.Queue.DedupKeyRefreshesNoNewSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_DedupKeyRefreshesNoNewSlot::RunTest(const FString& Parameters)
{
	FMCore_ToastQueue Queue;
	Queue.SetMaxVisible(2);

	uint32 Existing = 0;
	Queue.Admit(MakeEntry(1, 0, TEXT("save")), Existing);

	uint32 ExistingHandle = 0;
	const EMCore_ToastAdmission Admission = Queue.Admit(MakeEntry(2, 0, TEXT("save")), ExistingHandle);
	TestEqual(TEXT("Same DedupKey is DedupRefreshed"), AsInt(Admission), AsInt(EMCore_ToastAdmission::DedupRefreshed));
	TestEqual(TEXT("Existing handle is the live one"), static_cast<int32>(ExistingHandle), 1);
	TestEqual(TEXT("Active unchanged"), Queue.NumActive(), 1);
	TestEqual(TEXT("Pending unchanged"), Queue.NumPending(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_PromoteNextAfterSlotFreed,
	"Maevix.UI.Toast.Queue.PromoteNextAfterSlotFreed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_PromoteNextAfterSlotFreed::RunTest(const FString& Parameters)
{
	FMCore_ToastQueue Queue;
	Queue.SetMaxVisible(1);

	uint32 Existing = 0;
	Queue.Admit(MakeEntry(1), Existing);
	Queue.Admit(MakeEntry(2), Existing);   /* queued */

	FMCore_ToastQueueEntry Promoted;
	TestFalse(TEXT("No promotion while at cap"), Queue.PromoteNext(Promoted));
	TestTrue(TEXT("RemoveActive frees the slot"), Queue.RemoveActive(1));
	TestTrue(TEXT("Promotion succeeds after free"), Queue.PromoteNext(Promoted));
	TestEqual(TEXT("Promoted the queued entry"), static_cast<int32>(Promoted.HandleID), 2);

	FMCore_ToastQueueEntry None;
	TestFalse(TEXT("Empty queue promotes nothing"), Queue.PromoteNext(None));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_RemovePendingByHandle,
	"Maevix.UI.Toast.Queue.RemovePendingByHandle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_RemovePendingByHandle::RunTest(const FString& Parameters)
{
	FMCore_ToastQueue Queue;
	Queue.SetMaxVisible(1);

	uint32 Existing = 0;
	Queue.Admit(MakeEntry(1), Existing);   /* active */
	Queue.Admit(MakeEntry(2), Existing);   /* pending */
	Queue.Admit(MakeEntry(3), Existing);   /* pending */

	TestTrue(TEXT("Remove a queued entry"), Queue.RemovePending(2));
	TestFalse(TEXT("Removing an unknown handle is false"), Queue.RemovePending(99));
	TestEqual(TEXT("Pending is 1"), Queue.NumPending(), 1);

	Queue.RemoveActive(1);
	FMCore_ToastQueueEntry Promoted;
	Queue.PromoteNext(Promoted);
	TestEqual(TEXT("Removed entry never promotes"), static_cast<int32>(Promoted.HandleID), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_MaxVisibleSerializes,
	"Maevix.UI.Toast.Queue.MaxVisibleSerializes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_MaxVisibleSerializes::RunTest(const FString& Parameters)
{
	FMCore_ToastQueue Queue;
	Queue.SetMaxVisible(1);   /* explicit, not relying on the member initializer */

	uint32 Existing = 0;
	TestEqual(TEXT("First is ShowNow"), AsInt(Queue.Admit(MakeEntry(1), Existing)), AsInt(EMCore_ToastAdmission::ShowNow));
	TestEqual(TEXT("Second is Queued"), AsInt(Queue.Admit(MakeEntry(2), Existing)), AsInt(EMCore_ToastAdmission::Queued));
	TestEqual(TEXT("Third is Queued"), AsInt(Queue.Admit(MakeEntry(3), Existing)), AsInt(EMCore_ToastAdmission::Queued));
	TestEqual(TEXT("Only one active"), Queue.NumActive(), 1);

	Queue.RemoveActive(1);
	FMCore_ToastQueueEntry Promoted;
	TestTrue(TEXT("Promote one"), Queue.PromoteNext(Promoted));
	TestEqual(TEXT("Still only one active"), Queue.NumActive(), 1);
	TestFalse(TEXT("No second promotion while at cap"), Queue.PromoteNext(Promoted));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCoreTest_Toast_HandleValidityAndEquality,
	"Maevix.UI.Toast.Queue.HandleValidityAndEquality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMCoreTest_Toast_HandleValidityAndEquality::RunTest(const FString& Parameters)
{
	FMCore_ToastHandle Invalid;
	TestFalse(TEXT("Default handle is invalid"), Invalid.IsValid());
	TestEqual(TEXT("Default handle id is 0"), static_cast<int32>(Invalid.GetID()), 0);

	FMCore_ToastHandle Handle(7);
	TestTrue(TEXT("Explicit handle is valid"), Handle.IsValid());
	TestEqual(TEXT("Explicit handle id is 7"), static_cast<int32>(Handle.GetID()), 7);

	const FMCore_ToastHandle SameId(7);
	const FMCore_ToastHandle OtherId(8);
	TestTrue(TEXT("Equal when ids match"), Handle == SameId);
	TestTrue(TEXT("Not equal when ids differ"), Handle != OtherId);

	Handle.Invalidate();
	TestFalse(TEXT("Invalidated handle is invalid"), Handle.IsValid());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
