// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"


#define LOCTEXT_NAMESPACE "Inventory"
// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

FItemAddResult UInventoryComponent::TryAddItem(class UItem* Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Quantity)
{
	UItem* Item = NewObject<UItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);
	return TryAddItem_Internal(Item);
}

int32 UInventoryComponent::ConsumeItem(class UItem* Item)
{
	return ConsumeItem(Item, Item->GetQuantity()); 
}

int32 UInventoryComponent::ConsumeItem(class UItem* Item, const int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority() && Item)
	{
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());

		// We should not have a negative amount of the item after the consume.
		ensure(!(Item->GetQuantity() - RemoveQuantity));

		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		// If we now have zero of this item, remove it from the inventory
		if (Item->GetQuantity() <= 0)
		{
			RemoveItem(Item);
		}
		else
		{
			ClientRefreshInventory();
		}

		return RemoveQuantity;
	}

	return 0;
}

bool UInventoryComponent::RemoveItem(class UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item)
		{
			Items.RemoveSingle(Item);

			ReplicatedItemsKey++;

			return true;
		}
	}
	return false;
}

bool UInventoryComponent::HasItem(TSubclassOf<class UItem> ItemClass, const int32 Quantity /*= 1*/) const
{
	if (UItem* ItemToFind = FindItemByClass(ItemClass))
	{
		return ItemToFind->GetQuantity() >= Quantity;
	}
	return false;
}

UItem* UInventoryComponent::FindItem(class UItem* Item) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == Item->GetClass())
		{
			return InvItem;
		}
	}
	return nullptr;
}

UItem* UInventoryComponent::FindItemByClass(TSubclassOf<class UItem> ItemClass) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Weight = 0.f;

	for (auto& Item : Items)
	{
		if (Item)
		{
			Weight += Item->GetStackWeight();
		}
	}
	return Weight;
}

void UInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetSlotCapacity(const int32 NewSlotCapacity)
{
	SlotCapacity = NewSlotCapacity;
	OnInventoryUpdated.Broadcast();
}

TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<class UItem> ItemClass) const
{
	TArray<UItem*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			ItemsOfClass.Add(InvItem);
		}
	}
	return ItemsOfClass;
}

void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}


void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey))
	{
		for (auto& Item : Items)
		{
			if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
			{
				bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}

	return bWroteSomething;
	
}

UItem* UInventoryComponent::AddItem(class UItem* Item, const int32 Quantity)
{

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UItem* NewItem = NewObject<UItem>(GetOwner(), Item->GetClass());
		NewItem->World = GetWorld();
		NewItem->SetQuantity(Quantity);
		NewItem->OwningInventory = this;
		NewItem->AddedToInventory(this);
		Items.Add(NewItem);
		NewItem->MarkDirtyForReplication();
		//OnItemAdded.Broadcast(NewItem);
		OnRep_Items();

		return NewItem;
	}

	return nullptr;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();

	for (auto& Item : Items)
	{
		if (Item && !Item->World)
		{
			Item->World = GetWorld();
		}
	}
}

FItemAddResult UInventoryComponent::TryAddItem_Internal(class UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item->bStackable)
		{
			//Somehow the items quantity went over the max stack size. This shouldn't ever happen
			ensure(Item->GetQuantity() <= Item->MaxStackSize);

			if (UItem* ExistingItem = FindItem(Item))
			{
				if (ExistingItem->IsStackFull())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackFullText", "Couldn't add {ItemName}. Tried adding items to a stack that was full."), Item->ItemDisplayName));
				}
				else
				{
					//Find the maximum amount of the item we could take due to weight
					const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
					const int32 QuantityMaxAddAmount = FMath::Min(ExistingItem->MaxStackSize - ExistingItem->GetQuantity(), Item->GetQuantity());
					const int32 AddAmount = Item->IsWeightless() ? QuantityMaxAddAmount : FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);

					if (AddAmount <= 0)
					{
						//Already did full stack check, must not have enough weight
						return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackWeightFullText", "Couldn't add {ItemName}, too much weight."), Item->ItemDisplayName));
					}
					else
					{
						ExistingItem->SetQuantity(ExistingItem->GetQuantity() + AddAmount);
						return AddAmount >= Item->GetQuantity() ? FItemAddResult::AddedAll(Item->GetQuantity()) : FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
					}
				}
			}
			else //we want to add a stackable item that doesn't exist in the inventory
			{
				if (Items.Num() + 1 > GetSlotCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->ItemDisplayName));
				}

				const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
				const int32 QuantityMaxAddAmount = FMath::Min(Item->MaxStackSize, Item->GetQuantity());

				//If the item is weightless then ignore the weight max add amount
				const int32 AddAmount = Item->IsWeightless() ? QuantityMaxAddAmount : FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);

				AddItem(Item, AddAmount);

				return AddAmount >= Item->GetQuantity() ? FItemAddResult::AddedAll(Item->GetQuantity()) : FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
			}
		}
		else //item isn't stackable
		{
			if (Items.Num() + 1 > GetSlotCapacity())
			{
				return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->ItemDisplayName));
			}

			//Items with a weight of zero don't require a weight check
			if (!FMath::IsNearlyZero(Item->Weight))
			{
				if (GetCurrentWeight() + Item->Weight > GetWeightCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackWeightFullText", "Couldn't add {ItemName}, too much weight."), Item->ItemDisplayName));
				}
			}

			//Non-stackable should always have a quantity of 1
			UE_LOG(LogTemp, Warning, TEXT("Item Quantity is: %i"), Item->GetQuantity());
			ensure(Item->GetQuantity() == 1);

			AddItem(Item, 1);

			return FItemAddResult::AddedAll(Item->GetQuantity());
		}
	}

	//AddItem should never be called on a client
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}

#undef LOCTEXT_NAMESPACE