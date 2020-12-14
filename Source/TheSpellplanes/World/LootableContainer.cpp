// Fill out your copyright notice in the Description page of Project Settings.


#include "LootableContainer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Player/SpellPlanesCharacter.h"
#include "Items/Item.h"
#include "Engine/DataTable.h"

#define LOCTEXT_NAMESPACE "LootableContainer"

// Sets default values
ALootableContainer::ALootableContainer()
{
	LootContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>("LootContainerMesh");
	SetRootComponent(LootContainerMesh);

	LootInteraction = CreateDefaultSubobject<UInteractionComponent>("LootInteraction");
	LootInteraction->SetInteractableActionText(LOCTEXT("LootContainerText", "Loot"));
	LootInteraction->SetInteractableNameText(LOCTEXT("LootContainerName", "Chest"));
	LootInteraction->SetupAttachment(GetRootComponent());

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	Inventory->SetSlotCapacity(25);
	Inventory->SetWeightCapacity(60.f);

	LootRolls = FIntPoint(2, 8);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ALootableContainer::BeginPlay()
{
	Super::BeginPlay();
	
	LootInteraction->OnInteract.AddDynamic(this, &ALootableContainer::OnInteract);

	FillContainerWithLoot();
}

void ALootableContainer::FillContainerWithLoot()
{
	if (HasAuthority() && LootTable)
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		int32 Rolls = FMath::RandRange(LootRolls.GetMin(), LootRolls.GetMax());

		for (int32 i = 0; i < Rolls; ++i)
		{
			const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

			ensure(LootRow);

			float ProbabilityRoll = FMath::FRandRange(0.f, 1.f);

			while (ProbabilityRoll > LootRow->Probability)
			{
				LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
				ProbabilityRoll = FMath::FRandRange(0.f, 1.f);
			}

			if (LootRow && LootRow->Items.Num())
			{
				for (auto& ItemClass : LootRow->Items)
				{
					if (ItemClass)
					{
						const int32 Quantity = Cast<UItem>(ItemClass->GetDefaultObject())->GetQuantity();
						Inventory->TryAddItemFromClass(ItemClass, Quantity);
					}
				}
			}
		}
	}
}

void ALootableContainer::OnInteract(class ASpellPlanesCharacter* Character)
{
	if (Character)
	{
		Character->SetLootSource(Inventory);
	}
}

#undef LOCTEXT_NAMESPACE