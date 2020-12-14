// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBlock.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "World/Pickup.h"
#include "Player/SpellPlanesCharacter.h"

AResourceBlock::AResourceBlock()
	: m_ItemDropQuantity(FIntPoint(10, 30))
	
{
	SetReplicates(true);
	BlockIdIndex = EBlockIdIndex::BII_BlockType;
}


void AResourceBlock::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AResourceBlock, m_CurrentHealth);
	DOREPLIFETIME(AResourceBlock, m_BlockType);
}

float AResourceBlock::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	const float DamageDelt = ModifyHealth(-Damage);

	if (m_CurrentHealth <= 0.f)
	{
		if (ASpellPlanesCharacter* KillerCharacter = Cast<ASpellPlanesCharacter>(DamageCauser->GetOwner()))
		{
			// TODO: Create a game Stats Log System to keep track of everything in the game. 
			// KillerCharacter->StatsLogEvent(DamageEvent, self);
		}
	}

	return DamageDelt;
}


float AResourceBlock::GetCurrentHealth()
{
	return m_CurrentHealth;
}

void AResourceBlock::OnRep_Health(float OldHealth)
{
	OnHealthModified(m_CurrentHealth - OldHealth);
}

float AResourceBlock::ModifyHealth(const float Delta)
{
	const float OldHealth = m_CurrentHealth;

	m_CurrentHealth = FMath::Clamp<float>(m_CurrentHealth + Delta, 0.f, MaxHealth);

	return m_CurrentHealth - OldHealth;
}

void AResourceBlock::RestoreFullHealth()
{
	if (HasAuthority())
	{
		m_CurrentHealth = MaxHealth;
	}
}

void AResourceBlock::DestroyAndSpawnItem()
{
	if (HasAuthority())
	{
		if (ItemClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			const int32 ItemQuantity = FMath::RandRange(m_ItemDropQuantity.GetMin(), m_ItemDropQuantity.GetMax());
			if (ItemQuantity > 0)
			{
				FTransform SpawnTransform = GetActorTransform();

				APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
				Pickup->InitializePickup(ItemClass, ItemQuantity);
			}
		}

		Destroy();
	}
}
