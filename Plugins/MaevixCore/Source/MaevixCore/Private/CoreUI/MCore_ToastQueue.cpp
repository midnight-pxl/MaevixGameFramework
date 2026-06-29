// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/MCore_ToastQueue.h"

void FMCore_ToastQueue::SetMaxVisible(int32 InMax)
{
	/* Floor of 1: zero would wedge an anchor with nothing able to show. */
	MaxVisible = FMath::Max(1, InMax);
}

EMCore_ToastAdmission FMCore_ToastQueue::Admit(const FMCore_ToastQueueEntry& Entry, uint32& OutExistingHandleID)
{
	/* Dedup first: a live entry sharing the key is refreshed in place rather than stacked. */
	if (!Entry.DedupKey.IsNone())
	{
		uint32 ExistingID = 0;
		if (FindByDedupKey(Entry.DedupKey, ExistingID))
		{
			OutExistingHandleID = ExistingID;
			return EMCore_ToastAdmission::DedupRefreshed;
		}
	}

	if (Active.Num() < MaxVisible)
	{
		Active.Add(Entry);
		return EMCore_ToastAdmission::ShowNow;
	}

	InsertPendingByPriority(Entry);
	return EMCore_ToastAdmission::Queued;
}

void FMCore_ToastQueue::InsertPendingByPriority(const FMCore_ToastQueueEntry& Entry)
{
	/* Insert before the first existing entry of strictly lower priority. Equal priority
	   lands after existing equals, so the order stays FIFO within a priority band. */
	int32 InsertIndex = 0;
	for (; InsertIndex < Pending.Num(); ++InsertIndex)
	{
		if (Pending[InsertIndex].Priority < Entry.Priority)
		{
			break;
		}
	}
	Pending.Insert(Entry, InsertIndex);
}

bool FMCore_ToastQueue::PromoteNext(FMCore_ToastQueueEntry& OutPromoted)
{
	if (Pending.Num() == 0 || Active.Num() >= MaxVisible)
	{
		return false;
	}

	OutPromoted = Pending[0];
	Pending.RemoveAt(0);
	Active.Add(OutPromoted);
	return true;
}

bool FMCore_ToastQueue::RemoveActive(uint32 HandleID)
{
	return Active.RemoveAll([HandleID](const FMCore_ToastQueueEntry& E) { return E.HandleID == HandleID; }) > 0;
}

bool FMCore_ToastQueue::RemovePending(uint32 HandleID)
{
	return Pending.RemoveAll([HandleID](const FMCore_ToastQueueEntry& E) { return E.HandleID == HandleID; }) > 0;
}

bool FMCore_ToastQueue::FindByDedupKey(FName Key, uint32& OutHandleID) const
{
	if (Key.IsNone())
	{
		return false;
	}

	for (const FMCore_ToastQueueEntry& E : Active)
	{
		if (E.DedupKey == Key)
		{
			OutHandleID = E.HandleID;
			return true;
		}
	}
	for (const FMCore_ToastQueueEntry& E : Pending)
	{
		if (E.DedupKey == Key)
		{
			OutHandleID = E.HandleID;
			return true;
		}
	}
	return false;
}

void FMCore_ToastQueue::Reset()
{
	Active.Reset();
	Pending.Reset();
}
