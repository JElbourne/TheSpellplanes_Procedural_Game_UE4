// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSpawn.h"
#include "World/Pickup.h"
#include "Items/Item.h"

AItemSpawn::AItemSpawn()
{
	PrimaryActorTick.bCanEverTick = false;
	// The clients never have to see a spawner, only the pickups that the server creates.
	// So this Actor (Spawner) never needs to Load on Client.
	bNetLoadOnClient = false;

	RespawnTimeRange = FIntPoint(10, 30);
}

void AItemSpawn::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnItem();
	}
}

void AItemSpawn::SpawnItem()
{
	if (HasAuthority() && LootTable)
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

		ensure(LootRow);

		float ProbabilityRoll = FMath::FRandRange(0.f, 1.f);

		while (ProbabilityRoll > LootRow->Probability)
		{
			LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
			ProbabilityRoll = FMath::FRandRange(0.f, 1.f);
		}

		if (LootRow && LootRow->Items.Num() && PickupClass)
		{

			for (auto& ItemClass : LootRow->Items)
			{
				if (ItemClass)
				{
					const FVector LocationOffset = FVector(0.f, 0.f, 0.f); // This should run a function to find a free grid location
					// TODO this should spawn the items in blocks that are open, within a range from this spawner.
					// I am thinking this Spawner will be for natural items like flowers and herbs etc. That will naturally grow up in the area.
					// Once the player picks the item it will grow back on a timer.

					FActorSpawnParameters SpawnParams;
					SpawnParams.bNoFail = true;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

					const int32 ItemQuantity = ItemClass->GetDefaultObject<UItem>()->GetQuantity();

					FTransform SpawnTransform = GetActorTransform();
					SpawnTransform.AddToTranslation(LocationOffset);

					APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
					Pickup->InitializePickup(ItemClass, ItemQuantity);
					Pickup->OnDestroyed.AddUniqueDynamic(this, &AItemSpawn::OnItemTaken);

					SpawnedPickups.Add(Pickup);
				}
			}
		}
	}
}

void AItemSpawn::OnItemTaken(AActor* DestroyedActor)
{
	if (HasAuthority())
	{
		SpawnedPickups.Remove(DestroyedActor);

		// If all pickups were taken queue a respawn
		if (SpawnedPickups.Num() <= 0)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnItem, this, &AItemSpawn::SpawnItem, FMath::RandRange(RespawnTimeRange.GetMin(), RespawnTimeRange.GetMax()), false);

		}
	}
}
