// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/WorldUtilities.h"
#include "Engine/GameInstance.h"
#include "SpellPlanesSaveGame.h"
#include "World/WorldUtilities.h"
#include "SpellPlanesGameInstance.generated.h"

/**
 * Initialize the Game from startup
 * The SpellPlanesGameInstance will setup the core game data from the MainMenu including the Character Creation process.
 * It will set the SpellPlanes world data from a Save or create new data to use in the game such as:
 *  - The levels procedurally generated map, or more specifically a solution path through a dynamic map.
 *  - The Player data.
 *
 * After it is done it will load the Game Level.
 */
UCLASS()
class THESPELLPLANES_API USpellPlanesGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	USpellPlanesGameInstance();

protected:

	UPROPERTY()
	uint8 m_StartSolutionLength;

	UPROPERTY()
	FString m_StartWorldName;

	UPROPERTY()
	FString m_SeedText;

	UPROPERTY()
	FVector m_StartPlayerLocation;

	UPROPERTY()
	int32 m_StartPlayerElevation;

	/** This is the current region. Used to setup the correct world block data. */
	UPROPERTY(VisibleAnywhere, Category = "GameInstance")
	EWorldRegionType m_CurrentRegion;

	/** This is the Player data. Their character stats, current settings/values for use in game play. */
	UPROPERTY(VisibleAnywhere, Category = "GameInstance")
	FPlayerCharacterData m_CurrentPlayerData;

	/** This is the Map of all saved Region Data to a specific Region Name. Essentially the entire World. */
	UPROPERTY(VisibleAnywhere, Category = "GameInstance")
	TMap<EWorldRegionType, FRegionData> m_BlockDataByRegion;

	/** This is the struct of all the current Region Data. */
	UPROPERTY(VisibleAnywhere, Category = "GameInstance")
	FRegionData m_CurrentRegionData;

	/** This is the struct of all the current Region Data. */
	UPROPERTY(VisibleAnywhere, Category = "GameInstance")
	FWorldData  m_CurrentWorldData = FWorldData();

	/** This is the current Save Slot the player is using to save games. This is setup at the Main Menu. */
	UPROPERTY(VisibleAnywhere, Category = "GameInstance")
	ESaveSlots m_CurrentSaveSlot;

public:

	UFUNCTION()
	uint8 GetSolutionLength();

	UFUNCTION()
	FVector	GetPlayerSavedLocation();

	UFUNCTION()
	int32	GetPlayerStartElevation();

	UFUNCTION()
	FString	GetSeedText();

	UFUNCTION()
	void SetSolutionPathInCurrentRegion(TMap<FIntVector, EZoneType> SolutionPathData);

	/** Will start a brand new game, should only be called from the Character Creation Menu */
	UFUNCTION()
	void StartNewCleanGame(ESaveSlots SaveSlot, FString WorldName, EWorldRegionType StartingRegionType, FPlayerCharacterData NewPlayerData, uint8 SolutionLength);

	/** This method will create all new region data. */
	UFUNCTION()
	void CreateNewRegionData();

	/** This method will create all new Player Character data. */
	UFUNCTION()
	void CreateNewPlayerCharacterData(FPlayerCharacterData NewPlayerData);

	/** Will start a Saved game after selecting a Save Slot. Should only be called from the Main Menu. */
	UFUNCTION()
	void StartSavedGame(ESaveSlots SaveSlot);

	/**
	* This is the public method to Save the game at any time. The method will get the data needed from the Game Instance before saving.
	* Currently this will save the game in 1 Slot. Perhaps this will need to change in future to break up data. Smaller chunks to save.
	*/
	UFUNCTION()
	bool SaveDataToSaveSlot();

	/** The method will load data from the current Save Slot Name and set the member variables. */
	UFUNCTION()
	bool LoadDataFromSaveSlot();

	/** This method will create all new region data, mainly the Solution Path. */
	UFUNCTION()
	void SetRegionDataFromWorldData(EWorldRegionType RegionToSet);



	/** This method is called to open the World Level after all the variable have been setup in this GameInstance */
	UFUNCTION()
	void OpenTheWorldLevel();


};
