// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Engine/DataTable.h"
#include "World/ItemSpawn.h"
#include "Player/SpellPlanesCharacter.h"
#include "LootableContainer.generated.h"

UCLASS()
class THESPELLPLANES_API ALootableContainer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALootableContainer();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* LootContainerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInteractionComponent* LootInteraction;

	/** The items in the lootable container are held in here */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot");
	class UDataTable* LootTable;

	// The number of times to roll the loot table. Random number between min and max will be used.
	FIntPoint LootRolls;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void FillContainerWithLoot();

public:	

	UFUNCTION()
	void OnInteract(class ASpellPlanesCharacter* Character);

};
