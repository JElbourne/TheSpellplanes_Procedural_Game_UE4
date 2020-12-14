// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "FoodItem.generated.h"

/**
 * 
 */
UCLASS()
class THESPELLPLANES_API UFoodItem : public UItem
{
	GENERATED_BODY()

public:

	UFoodItem();

	/** The amount for the food to cure hunger */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sustenance")
	float HealAmount;

	virtual void Use(class ASpellPlanesCharacter* Character) override;
	
};
