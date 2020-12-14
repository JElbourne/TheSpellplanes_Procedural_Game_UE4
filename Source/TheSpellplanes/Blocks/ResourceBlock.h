// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blocks/Block.h"
#include "Items/Item.h"
#include "Engine/EngineTypes.h"
#include "ResourceBlock.generated.h"

UENUM(BlueprintType)
enum class EResourceBlockTypes : uint8
{
	RBT_Wild_Grass UMETA(DisplayName = "Wild Grass"),

	// 10 Stone Blocks
	RBT_Basalt UMETA(DisplayName = "Basalt Block"),
	RBT_Gabbro UMETA(DisplayName = "Gabbro Block"),
	RBT_Granite UMETA(DisplayName = "Granite Block"),
	RBT_Limestone UMETA(DisplayName = "Limestone Block"),
	RBT_Lodestone  UMETA(DisplayName = "Lodestone Block"),
	RBT_Quartzite UMETA(DisplayName = "Quartzite Block"),
	RBT_Rhyolite UMETA(DisplayName = "Rhyolite Block"),
	RBT_Sandstone UMETA(DisplayName = "Sandstone Block"),
	RBT_Shale UMETA(DisplayName = "Shale Block"),
	RBT_Slate UMETA(DisplayName = "Slate Block"),

	// 10 Tree Blocks
	RBT_Oak UMETA(DisplayName = "Oak Tree Block"),
	RBT_Walnut UMETA(DisplayName = "Walnut Tree Block"),
	RBT_Redwood UMETA(DisplayName = "Redwood Tree Block"),
	RBT_Cedar UMETA(DisplayName = "Cedar Tree Block"),
	RBT_Baobob UMETA(DisplayName = "Baobob Tree Block"),
	RBT_RedMangrove UMETA(DisplayName = "Red Mangrove Tree Block"),
	RBT_Teak UMETA(DisplayName = "Teak Tree Block"),
	RBT_Ash UMETA(DisplayName = "Ash Tree Block"),
	RBT_Sugi UMETA(DisplayName = "Sugi Tree Block"),
	RBT_Pine UMETA(DisplayName = "Pine Tree Block"),
};

/** Naming of Spell Levels adapted from reddit commenter u/Kitten_of_Death */
UENUM(BlueprintType)
enum class ESpellAttackLevel : uint8
{
	SAL_Cantrip UMETA (DisplayName = "Lvl 0 Cantripr"),
	SAL_Novice UMETA(DisplayName = "Lvl 1 Novice"),
	SAL_Apprentice UMETA(DisplayName = "Lvl 2 Apprentice"),
	SAL_Journeyman UMETA(DisplayName = "Lvl 3 Journeyman"),
	SAL_Adept UMETA(DisplayName = "Lvl 4 Adept"),
	SAL_Expert UMETA(DisplayName = "Lvl 5 Expert"),
	SAL_Master UMETA(DisplayName = "Lvl 6 Master"),
	SAL_Grandmaster UMETA(DisplayName = "Lvl 7 Grandmaster"),
	SAL_Legendary UMETA(DisplayName = "Lvl 8 Legendary"),
	SAL_Mythic UMETA(DisplayName = "Lvl 9 Mythic"),
};

USTRUCT(BlueprintType)
struct FResourceBlockType : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	FResourceBlockType() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	EResourceBlockTypes BlockType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	TSubclassOf<class APickup> PickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	TSubclassOf<class UItem> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	float FullHealthTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	ESpellAttackLevel RequiredSpellLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	FColor WorldMapColour;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Terrain Info")
	int32 MaterialIndex;

	int32 InstancedStaticMeshIndex;
};

/**
 * 
 */
UCLASS(Blueprintable)
class THESPELLPLANES_API AResourceBlock : public ABlock
{
	GENERATED_BODY()
	
public:
	AResourceBlock();

	// Member Variables

	/** Type of resource block */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Block")
	EResourceBlockTypes m_BlockType;

	/** This is the class of the Blueprint Pickup we will place in the world after block destroyed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<class APickup> PickupClass;

	/** This is the type of Item this block could drop. (Random drop quantity) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<class UItem> ItemClass;

	/** The Total Amount of health the block starts with */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth;

	/** The amount of time until the block resets its health, after last being attacked. */
	UPROPERTY(EditAnywhere, Category = "Health")
	float FullHealthTimer;

	/** This is the required Level to do damage to the block (must be greater than or equal)  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spells")
	ESpellAttackLevel RequiredSpellLevel;

private:
	/** This is the timer handle we will use for setting black back to full health */
	FTimerHandle TimerHandle_RegenBlockHelath;

	/** This is the current health of the block */
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float m_CurrentHealth;

	/** This is the range of int's to determine how much items to drop when block destroyed. */
	FIntPoint m_ItemDropQuantity;


public:
	// Functions
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	float GetCurrentHealth();

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthModified(const float HealthDelta);


private:
	// Modify the players health by either a negative or positive amount. Return the amount of health actually removed.
	float ModifyHealth(const float Delta);

	UFUNCTION()
	void RestoreFullHealth();

	UFUNCTION()
	void DestroyAndSpawnItem();
};
