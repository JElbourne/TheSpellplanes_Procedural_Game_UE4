// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "EquipableItem.generated.h"

UENUM(BlueprintType)
enum class EEquippableSlot : uint8
{
	EIS_Head UMETA(DisplayName = "Head"),
	EIS_Hair UMETA(DisplayName = "Hair"),
	EIS_Ears UMETA(DisplayName = "Ears"),
	EIS_Chest UMETA(DisplayName = "Chest"),
	EIS_Hands UMETA(DisplayName = "Hands"),
	EIS_Legs UMETA(DisplayName = "Legs"),
	EIS_Feet UMETA(DisplayName = "Feet"),
	EIS_Tail UMETA(DisplayName = "Tail"),
	EIS_Backpack UMETA(DisplayName = "Backpack"),
	EIS_Hat UMETA(DisplayName = "Hat"),
	EIS_L_Holding UMETA(DisplayName = "Left Holding"),
	EIS_R_Holding UMETA(DisplayName = "Right Holding"),
	EIS_Throwable UMETA(DisplayName = "Throwable")
};

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable)
class THESPELLPLANES_API UEquipableItem : public UItem
{
	GENERATED_BODY()
	
public:

	UEquipableItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipables")
	EEquippableSlot Slot;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Use(class ASpellPlanesCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool Equip(class ASpellPlanesCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool UnEquip(class ASpellPlanesCharacter* Character);

	virtual bool ShouldShowInInventory() const override;

	UFUNCTION(BlueprintPure, Category = "Equippables")
	bool IsEquipped() { return bEquipped; }

	/** Call this on the server to equip the item */
	void SetEquipped(bool bNewEquipped);

protected:

	UPROPERTY(ReplicatedUsing = EquipStatusChanged)
	bool bEquipped;

	UFUNCTION()
	void EquipStatusChanged();
};
