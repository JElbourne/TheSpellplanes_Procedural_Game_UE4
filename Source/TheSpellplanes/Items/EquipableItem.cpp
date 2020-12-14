// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipableItem.h"
#include "Net/UnrealNetwork.h"
#include "Player/SpellPlanesCharacter.h"
#include "Components/InventoryComponent.h"

#define LOCTEXT_NAMESPACE "EquippableItem"

UEquipableItem::UEquipableItem()
{
	bStackable = false;
	bEquipped = false;
	UseActionText = LOCTEXT("ItemUseActionText", "Equip");
}

void UEquipableItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipableItem, bEquipped);
}

void UEquipableItem::Use(class ASpellPlanesCharacter* Character)
{
	if (Character && Character->HasAuthority())
	{
		if (Character->GetEquippedItems().Contains(Slot) && !bEquipped)
		{
			UEquipableItem* AlreadyEquippedItem = *Character->GetEquippedItems().Find(Slot);
			AlreadyEquippedItem->SetEquipped(false);
		}

		SetEquipped(!IsEquipped());
	}
}

bool UEquipableItem::Equip(class ASpellPlanesCharacter* Character)
{
	if (Character)
	{
		return Character->EquipItem(this);
	}
	return false;
}

bool UEquipableItem::UnEquip(class ASpellPlanesCharacter* Character)
{
	if (Character)
	{
		return Character->UnEquipItem(this);
	}
	return false;
}

bool UEquipableItem::ShouldShowInInventory() const
{
	return !bEquipped;
}

void UEquipableItem::SetEquipped(bool bNewEquipped)
{
	bEquipped = bNewEquipped;
	EquipStatusChanged();
	MarkDirtyForReplication();
}

void UEquipableItem::EquipStatusChanged()
{
	if (ASpellPlanesCharacter* Character = Cast<ASpellPlanesCharacter>(GetOuter()))
	{
		if (bEquipped)
		{
			Equip(Character);
		}
		else
		{
			UnEquip(Character);
		}
	}

	// Tell UI to update
	OnItemModified.Broadcast();
}

#undef LOCTEXT_NAMESPACE