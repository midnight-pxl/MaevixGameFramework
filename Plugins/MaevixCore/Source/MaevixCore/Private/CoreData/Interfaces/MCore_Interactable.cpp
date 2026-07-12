// Copyright 2025, Midnight Pixel Studio LLC. All Rights Reserved

#include "CoreData/Interfaces/MCore_Interactable.h"

bool IMCore_Interactable::CanInteract_Implementation(const FMCore_InteractionContext& Context) const
{
	// Interactable by default; an implementer gates on state (locked, out of charges) by overriding this.
	return true;
}

void IMCore_Interactable::OnInteract_Implementation(const FMCore_InteractionContext& Context)
{
	// Default no-op; implementers commit their server-authoritative interaction result here.
}

FMCore_InteractionData IMCore_Interactable::GetInteractionData_Implementation(const FMCore_InteractionContext& Context) const
{
	// Empty payload by default; an implementer supplies prompt text and a type tag by overriding this.
	return FMCore_InteractionData();
}

int32 IMCore_Interactable::GetInteractionStateVersion_Implementation() const
{
	// 0 means static; an implementer bumps a monotonic counter when prompt, type, or interactability changes.
	return 0;
}
