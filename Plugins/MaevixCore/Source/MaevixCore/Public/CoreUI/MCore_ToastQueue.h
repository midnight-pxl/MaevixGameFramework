// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#pragma once

#include "CoreMinimal.h"

/** Result of admitting a request to a per-anchor queue. */
enum class EMCore_ToastAdmission : uint8
{
	ShowNow,
	Queued,
	DedupRefreshed
};

/**
 * One queued or active toast, reduced to the fields the queue reasons about.
 * No widget, no world, no timer: those live in the subsystem record.
 */
struct FMCore_ToastQueueEntry
{
	uint32 HandleID = 0;
	FName  DedupKey = NAME_None;
	int32  Priority = 0;
	bool   bPersistent = false; // Duration <= 0
};

/**
 * Per-anchor admission and ordering math. Pure C++ (no UObject, no world, no
 * timers): the subsystem turns these decisions into widget actions. Constructed
 * with MaxVisible = 1 so a bare instance matches the settings default; the
 * subsystem always calls SetMaxVisible before use.
 */
struct MAEVIXCORE_API FMCore_ToastQueue
{
	/** Set the active-slot cap for this anchor. Clamped to a floor of 1. */
	void SetMaxVisible(int32 InMax);

	/** Admit an entry. On DedupRefreshed, OutExistingHandleID is the live entry to refresh in place. */
	EMCore_ToastAdmission Admit(const FMCore_ToastQueueEntry& Entry, uint32& OutExistingHandleID);

	/** Move the next pending entry (highest Priority, then FIFO) into an active slot. False if none or at cap. */
	bool PromoteNext(FMCore_ToastQueueEntry& OutPromoted);

	bool RemoveActive(uint32 HandleID);
	bool RemovePending(uint32 HandleID);

	/**
	 * Find a live entry by DedupKey, active first then pending, within THIS anchor's queue only.
	 * Dedup is anchor-local: the same key at another anchor is a separate toast.
	 */
	bool FindByDedupKey(FName Key, uint32& OutHandleID) const;

	int32 NumActive() const { return Active.Num(); }
	int32 NumPending() const { return Pending.Num(); }
	void Reset();

private:
	void InsertPendingByPriority(const FMCore_ToastQueueEntry& Entry);

	int32 MaxVisible = 1;
	TArray<FMCore_ToastQueueEntry> Active;
	TArray<FMCore_ToastQueueEntry> Pending; // priority-ordered, FIFO within equal priority
};
