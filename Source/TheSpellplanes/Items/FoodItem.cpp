// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodItem.h"
#include "../Player/SpellPlanesCharacter.h"
#include "../Player/SpellPlanesPlayerController.h"
#include "../Components/InventoryComponent.h"


#define LOCTEXT_NAMESPACE "FoodItem"

UFoodItem::UFoodItem()
{
	HealAmount = 20.f;
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
}

void UFoodItem::Use(class ASpellPlanesCharacter* Character)
{
	if (Character)
	{
		const float ActualHealedAmount = Character->ModifyHealth(HealAmount);
		const bool bUsedFood = !FMath::IsNearlyZero(ActualHealedAmount);

		if (!Character->HasAuthority())
		{
			if (ASpellPlanesPlayerController* SpPC = Cast<ASpellPlanesPlayerController>(Character->GetController()))
			{
				if (bUsedFood)
				{
					SpPC->ShowNotification(FText::Format(LOCTEXT("AteFoodItem", "Ate {FoodName}, healed {HealAmount} health."), ItemDisplayName, ActualHealedAmount));
				}
				else
				{
					SpPC->ShowNotification(FText::Format(LOCTEXT("FullHealthText", "No need to eat {FoodName}, health is already full."), ItemDisplayName));
				}
			}
		}

		if (bUsedFood)
		{
			if (UInventoryComponent* Inventory = Character->PlayerInventory)
			{
				Inventory->ConsumeItem(this, 1);
			}
		}
	}
	
}

#undef LOCTEXT_NAMESPACE