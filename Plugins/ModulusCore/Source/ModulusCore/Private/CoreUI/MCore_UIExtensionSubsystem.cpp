// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreUI/MCore_UIExtensionSubsystem.h"

#include "CoreData/Types/UI/MCore_UIExtensionTypes.h"
#include "CoreData/Logging/LogModulusUI.h"

#include "Blueprint/UserWidget.h"

// ============================================================================
// INITIALIZATION
// ============================================================================

void UMCore_UIExtensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	NextHandleID = 1;
	
	UE_LOG(LogModulusUI, Verbose, TEXT("UIExtensionSubsystem::Initialize: initialized"));
}

void UMCore_UIExtensionSubsystem::Deinitialize()
{
	ExtensionPointMap.Empty();
	ExtensionMap.Empty();
	ExtensionPointHandles.Empty();
	ExtensionHandles.Empty();
	NextHandleID = 1;
	
	UE_LOG(LogModulusUI, Verbose, TEXT("UIExtensionSubsystem::Deinitialize: deinitialized"));
	
	Super::Deinitialize();
}

// ============================================================================
// EXTENSION POINT REGISTRATION
// ============================================================================

/**
 * @param ExtensionPointTag  GameplayTag identifying this slot. Must be valid.
 * @param ContextObject      Per-player filtering context (LocalPlayer), or nullptr for global.
 * @param ExtensionCallback  Called with Added/Removed action when extensions change.
 * @return                   Handle for later unregistration. Invalid if tag was empty.
 */
FMCore_UIExtensionPointHandle UMCore_UIExtensionSubsystem::RegisterExtensionPoint(
	const FGameplayTag& ExtensionPointTag,
	UObject* ContextObject,
	const FOnUIExtensionChanged& ExtensionCallback)
{
	if (!ExtensionPointTag.IsValid())
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("UIExtensionSubsystem::RegisterExtensionPoint: invalid ExtensionPointTag"));
		return FMCore_UIExtensionPointHandle();
	}

	if (!ExtensionCallback.IsBound())
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("UIExtensionSubsystem::RegisterExtensionPoint: callback not bound for Tag='%s'"),
			*ExtensionPointTag.ToString());
	}

	TSharedPtr<FMCore_UIExtensionPoint> ExtensionPoint = MakeShared<FMCore_UIExtensionPoint>();
	ExtensionPoint->ExtensionPointTag = ExtensionPointTag;
	ExtensionPoint->ContextObject = ContextObject;
	ExtensionPoint->OnChangeCallback = ExtensionCallback;

	const uint32 HandleID = NextHandleID++;
	
	ExtensionPointMap.FindOrAdd(ExtensionPointTag).Add(ExtensionPoint);
	ExtensionPointHandles.Add(HandleID, ExtensionPoint);

	NotifyExtensionPointOfExtensions(ExtensionPoint.Get());

	UE_LOG(LogModulusUI, Verbose,
		TEXT("UIExtensionSubsystem::RegisterExtensionPoint: registered: Tag='%s', Context=%s, HandleID=%u"),
		*ExtensionPointTag.ToString(),
		ContextObject ? *ContextObject->GetName() : TEXT("Global"),
		HandleID);

	return FMCore_UIExtensionPointHandle(HandleID);
}

void UMCore_UIExtensionSubsystem::UnregisterExtensionPoint(
	FMCore_UIExtensionPointHandle& ExtensionPointHandle)
{
	if (!ExtensionPointHandle.IsValid())
	{
		return;
	}

	const uint32 HandleID = ExtensionPointHandle.GetID();

	TSharedPtr<FMCore_UIExtensionPoint>* FoundPoint = ExtensionPointHandles.Find(HandleID);
	if (!FoundPoint)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("UIExtensionSubsystem::UnregisterExtensionPoint: Handle %u not found"), HandleID);
		ExtensionPointHandle.Invalidate();
		return;
	}

	const FGameplayTag Tag = (*FoundPoint)->ExtensionPointTag;

	if (TArray<TSharedPtr<FMCore_UIExtensionPoint>>* PointArray = ExtensionPointMap.Find(Tag))
	{
		PointArray->Remove(*FoundPoint);
		if (PointArray->Num() == 0)
		{
			ExtensionPointMap.Remove(Tag);
		}
	}

	ExtensionPointHandles.Remove(HandleID);
	ExtensionPointHandle.Invalidate();

	UE_LOG(LogModulusUI, Verbose,
		TEXT("UIExtensionSubsystem::UnregisterExtensionPoint: unregistered: HandleID=%u"), HandleID);
}

// ============================================================================
// EXTENSION REGISTRATION
// ============================================================================

/**
 * @param ExtensionPointTag  Target extension point tag. Must be valid.
 * @param ContextObject      Per-player filtering context, or nullptr for global.
 * @param WidgetClass        Widget class to instantiate. Must be non-null.
 * @param Priority           Higher values sort first (default: 0).
 * @return                   Handle for later unregistration.
 */
FMCore_UIExtensionHandle UMCore_UIExtensionSubsystem::RegisterExtension(
	const FGameplayTag& ExtensionPointTag,
	UObject* ContextObject,
	TSubclassOf<UUserWidget> WidgetClass,
	int32 Priority)
{
	if (!ExtensionPointTag.IsValid())
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("UIExtensionSubsystem::RegisterExtension: invalid ExtensionPointTag"));
		return FMCore_UIExtensionHandle();
	}

	if (!WidgetClass)
	{
		UE_LOG(LogModulusUI, Error,
			TEXT("UIExtensionSubsystem::RegisterExtension: invalid WidgetClass for Tag='%s'"),
			*ExtensionPointTag.ToString());
		return FMCore_UIExtensionHandle();
	}

	TSharedPtr<FMCore_UIExtension> Extension = MakeShared<FMCore_UIExtension>();
	Extension->ExtensionPointTag = ExtensionPointTag;
	Extension->ContextObject = ContextObject;
	Extension->WidgetClass = WidgetClass;
	Extension->Priority = Priority;

	const uint32 HandleID = NextHandleID++;
	Extension->HandleID = HandleID;

	ExtensionMap.FindOrAdd(ExtensionPointTag).Add(Extension);
	ExtensionHandles.Add(HandleID, Extension);

	NotifyExtensionPointsOfExtension(EMCore_UIExtensionAction::Added, Extension.Get());

	UE_LOG(LogModulusUI, Verbose,
		TEXT("UIExtensionSubsystem::RegisterExtension: registered: Tag='%s', Widget=%s, Priority=%d, Context=%s, HandleID=%u"),
		*ExtensionPointTag.ToString(),
		*WidgetClass->GetName(),
		Priority,
		ContextObject ? *ContextObject->GetName() : TEXT("Global"),
		HandleID);

	return FMCore_UIExtensionHandle(HandleID);
}

void UMCore_UIExtensionSubsystem::UnregisterExtension(FMCore_UIExtensionHandle& ExtensionHandle)
{
	if (!ExtensionHandle.IsValid())
	{
		return;
	}

	const uint32 HandleID = ExtensionHandle.GetID();

	TSharedPtr<FMCore_UIExtension>* FoundExtension = ExtensionHandles.Find(HandleID);
	if (!FoundExtension)
	{
		UE_LOG(LogModulusUI, Warning,
			TEXT("UIExtensionSubsystem::UnregisterExtension: Handle %u not found"), HandleID);
		ExtensionHandle.Invalidate();
		return;
	}

	const FGameplayTag Tag = (*FoundExtension)->ExtensionPointTag;

	NotifyExtensionPointsOfExtension(EMCore_UIExtensionAction::Removed, FoundExtension->Get());

	if (TArray<TSharedPtr<FMCore_UIExtension>>* ExtArray = ExtensionMap.Find(Tag))
	{
		ExtArray->Remove(*FoundExtension);
		if (ExtArray->Num() == 0)
		{
			ExtensionMap.Remove(Tag);
		}
	}

	ExtensionHandles.Remove(HandleID);
	ExtensionHandle.Invalidate();

	UE_LOG(LogModulusUI, Verbose,
		TEXT("UIExtensionSubsystem::UnregisterExtension: unregistered: HandleID=%u"), HandleID);
}

// ============================================================================
// QUERIES
// ============================================================================

TSubclassOf<UUserWidget> UMCore_UIExtensionSubsystem::GetWidgetClassForExtension(
	const FMCore_UIExtensionHandle& ExtensionHandle) const
{
	if (!ExtensionHandle.IsValid())
	{
		return nullptr;
	}

	const TSharedPtr<FMCore_UIExtension>* FoundExtension = ExtensionHandles.Find(ExtensionHandle.GetID());
	if (!FoundExtension)
	{
		return nullptr;
	}

	return (*FoundExtension)->WidgetClass;
}

bool UMCore_UIExtensionSubsystem::IsExtensionRegistered(
	const FMCore_UIExtensionHandle& ExtensionHandle) const
{
	if (!ExtensionHandle.IsValid())
	{
		return false;
	}

	return ExtensionHandles.Contains(ExtensionHandle.GetID());
}

bool UMCore_UIExtensionSubsystem::IsExtensionPointRegistered(
	const FMCore_UIExtensionPointHandle& ExtensionPointHandle) const
{
	if (!ExtensionPointHandle.IsValid())
	{
		return false;
	}

	return ExtensionPointHandles.Contains(ExtensionPointHandle.GetID());
}

// ============================================================================
// INTERNAL
// ============================================================================

void UMCore_UIExtensionSubsystem::NotifyExtensionPointsOfExtension(
	EMCore_UIExtensionAction Action,
	const FMCore_UIExtension* Extension)
{
	if (!Extension)
	{
		return;
	}

	const FGameplayTag Tag = Extension->ExtensionPointTag;

	const TArray<TSharedPtr<FMCore_UIExtensionPoint>>* Points = ExtensionPointMap.Find(Tag);
	if (!Points)
	{
		return;
	}

	FMCore_UIExtensionHandle Handle(Extension->HandleID);

	for (const TSharedPtr<FMCore_UIExtensionPoint>& Point : *Points)
	{
		if (Point->DoesExtensionPassContract(Extension))
		{
			Point->OnChangeCallback.ExecuteIfBound(Action, Handle);
		}
	}
}

void UMCore_UIExtensionSubsystem::NotifyExtensionPointOfExtensions(
	const FMCore_UIExtensionPoint* ExtensionPoint)
{
	if (!ExtensionPoint)
	{
		return;
	}

	const FGameplayTag Tag = ExtensionPoint->ExtensionPointTag;

	const TArray<TSharedPtr<FMCore_UIExtension>>* Extensions = ExtensionMap.Find(Tag);
	if (!Extensions || Extensions->Num() == 0)
	{
		return;
	}

	/* Sort by priority (higher first) */
	TArray<TSharedPtr<FMCore_UIExtension>> SortedExtensions = *Extensions;
	SortedExtensions.Sort([](const TSharedPtr<FMCore_UIExtension>& A,
	                         const TSharedPtr<FMCore_UIExtension>& B)
	{
		return A->Priority > B->Priority;
	});

	for (const TSharedPtr<FMCore_UIExtension>& Extension : SortedExtensions)
	{
		if (ExtensionPoint->DoesExtensionPassContract(Extension.Get()))
		{
			FMCore_UIExtensionHandle Handle(Extension->HandleID);
			ExtensionPoint->OnChangeCallback.ExecuteIfBound(EMCore_UIExtensionAction::Added, Handle);
		}
	}
}