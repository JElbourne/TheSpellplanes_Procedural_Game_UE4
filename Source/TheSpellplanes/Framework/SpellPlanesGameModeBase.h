// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpellPlanesGameModeBase.generated.h"


//enum to store the current state of gameplay
UENUM(BlueprintType)
enum class ESpellPlanesPlayState : uint8
{
	ELoading,
	EPlaying,
	EGameOver,
	EWon,
	EUnknown
};

/**
 * 
 */
UCLASS()
class THESPELLPLANES_API ASpellPlanesGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	ASpellPlanesGameModeBase();

	virtual void BeginPlay() override;

private:
	/**Keeps track of the current playing state */
	UPROPERTY(VisibleAnywhere, Category = "Game Play State")
	ESpellPlanesPlayState m_CurrentPlayState;


public:
	/** Returns the current playing state */
	UFUNCTION()
	ESpellPlanesPlayState GetCurrentState() const;

	/** Sets a new playing state */
	UFUNCTION()
	void SetCurrentState(ESpellPlanesPlayState NewState);

private:
	/**Handle any function calls that rely upon changing the playing state of our game */
	void HandleNewState(ESpellPlanesPlayState NewState);



};
