// Fill out your copyright notice in the Description page of Project Settings.


#include "SpellPlanesGameModeBase.h"
#include "SpellPlanesGameStateBase.h"


ASpellPlanesGameModeBase::ASpellPlanesGameModeBase()
{

}

void ASpellPlanesGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	SetCurrentState(ESpellPlanesPlayState::ELoading);
}

ESpellPlanesPlayState ASpellPlanesGameModeBase::GetCurrentState() const
{
	return m_CurrentPlayState;
}

void ASpellPlanesGameModeBase::SetCurrentState(ESpellPlanesPlayState NewState)
{
	//set current state
	m_CurrentPlayState = NewState;
	// handle the new current state
	HandleNewState(m_CurrentPlayState);
}

void ASpellPlanesGameModeBase::HandleNewState(ESpellPlanesPlayState NewState)
{
	switch (NewState)
	{
		// When the game is loading
		case ESpellPlanesPlayState::ELoading:
		{
			
			// Start the Create Level Process
			if (GetGameState<ASpellPlanesGameStateBase>()->CreateLevel())
			{
				SetCurrentState(ESpellPlanesPlayState::EPlaying);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Error creating the Level"));
			}
		}
		break;
		// If the game is playing
		case ESpellPlanesPlayState::EPlaying:
		{
			UE_LOG(LogTemp, Warning, TEXT("We are now playing the game"));
		}
		break;
		// Unknown/default state
		default:
		case ESpellPlanesPlayState::EUnknown:
		{
			// do nothing
		}
		break;
	}
}
