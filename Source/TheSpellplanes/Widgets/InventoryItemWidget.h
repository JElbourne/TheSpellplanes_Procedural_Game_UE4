// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Item.h"
#include "InventoryItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class THESPELLPLANES_API UInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Invetory Tiem Widget", meta = (ExposeOnSpawn = true))
	class UItem* Item;
	
};
