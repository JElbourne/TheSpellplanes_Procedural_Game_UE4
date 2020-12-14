// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SpellPlanesGameInstance.h"
#include "Framework/SpellPlanesSaveGame.h"
#include "World/WorldUtilities.h"
#include "Kismet/GameplayStatics.h"
#include "SpellPlanesGameInstance.h"



USpellPlanesGameInstance::USpellPlanesGameInstance()
{

	m_StartSolutionLength = (uint8)10;
	m_StartPlayerLocation = FVector(0.f, 0.f, 0.f);
	m_StartPlayerElevation = 255;
	m_SeedText = TEXT("Apples");
}


uint8 USpellPlanesGameInstance::GetSolutionLength()
{
	return m_StartSolutionLength;
}

FVector USpellPlanesGameInstance::GetPlayerSavedLocation()
{
	return m_CurrentPlayerData.PlayerLocation;
}

int32 USpellPlanesGameInstance::GetPlayerStartElevation()
{
	return m_StartPlayerElevation;
}

FString USpellPlanesGameInstance::GetSeedText()
{
	return m_SeedText;
}

void USpellPlanesGameInstance::SetSolutionPathInCurrentRegion(TMap<FIntVector, EZoneType> SolutionPathData)
{
	m_CurrentRegionData.SolutionPathData = SolutionPathData;
}

void USpellPlanesGameInstance::StartNewCleanGame(ESaveSlots StartSaveSlot, FString StartWorldName, EWorldRegionType StartingRegionType, FPlayerCharacterData NewPlayerData, uint8 StartSolutionLength)
{
	m_StartSolutionLength = FMath::Clamp(StartSolutionLength, (uint8)10, (uint8)250);
	m_CurrentSaveSlot = StartSaveSlot;
	m_CurrentRegion = StartingRegionType;
	m_StartWorldName = StartWorldName;
	m_StartPlayerLocation = FVector(0.f, 0.f, 0.f);

	CreateNewRegionData();
	CreateNewPlayerCharacterData(NewPlayerData);
	SaveDataToSaveSlot();
}

void USpellPlanesGameInstance::CreateNewRegionData()
{
	m_CurrentRegionData.RegionName = EnumToString(EWorldRegionType, m_CurrentRegion);
	m_CurrentRegionData.ZoneData.Empty();
	m_CurrentRegionData.SolutionPathData.Empty();
}

void USpellPlanesGameInstance::CreateNewPlayerCharacterData(FPlayerCharacterData NewPlayerData)
{
	// TODO: the character creation menu system still needs to be done...this is where that info get set in the game instance.
	m_CurrentPlayerData = NewPlayerData;
}

void USpellPlanesGameInstance::StartSavedGame(ESaveSlots SaveSlot)
{
	m_CurrentSaveSlot = SaveSlot;
	bool SuccessLoad = LoadDataFromSaveSlot();

	if (!SuccessLoad)
	{
		// We could not get the Saved Data we expected to. So display error.
		UE_LOG(LogTemp, Warning, TEXT("We could not find the save data at teh slot specified."));

		// TODO: Create a Message on the UI so the player can see the issue.
	}
}


bool USpellPlanesGameInstance::SaveDataToSaveSlot()
{
	m_BlockDataByRegion.Add(m_CurrentRegion, m_CurrentRegionData);

	// Set the World Block Data
	m_CurrentWorldData.BlockDataByRegion = m_BlockDataByRegion;

	// Create instance of the save game class
	USpellPlanesSaveGame* SaveGameInstance = Cast<USpellPlanesSaveGame>(UGameplayStatics::CreateSaveGameObject(USpellPlanesSaveGame::StaticClass()));

	// Set the save game data
	SaveGameInstance->PlayerCharacterData = m_CurrentPlayerData;
	SaveGameInstance->WorldData = m_CurrentWorldData;

	FString SlotName = EnumToString(ESaveSlots, m_CurrentSaveSlot);
	if (SlotName == "Invalid")
	{
		return false;
	}
	//Save the save game instance
	return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}


bool USpellPlanesGameInstance::LoadDataFromSaveSlot()
{
	// Create instance of the save game class
	USpellPlanesSaveGame* SaveGameInstance = Cast<USpellPlanesSaveGame>(UGameplayStatics::CreateSaveGameObject(USpellPlanesSaveGame::StaticClass()));
	if (SaveGameInstance = Cast<USpellPlanesSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MySlot"), 0)))
	{
		// Set Game Instance Data from the Save Data
		m_CurrentPlayerData = SaveGameInstance->PlayerCharacterData;
		m_CurrentWorldData = SaveGameInstance->WorldData;

		m_StartPlayerLocation = m_CurrentPlayerData.PlayerLocation;
		m_StartSolutionLength = m_CurrentWorldData.SolutionPathLength;

		SetRegionDataFromWorldData(m_CurrentRegion);
		return true;
	}
	return false;
}


void USpellPlanesGameInstance::SetRegionDataFromWorldData(EWorldRegionType RegionToSet)
{
	if (m_CurrentWorldData.BlockDataByRegion.Contains(RegionToSet))
	{
		m_BlockDataByRegion = m_CurrentWorldData.BlockDataByRegion;
		m_CurrentRegionData = *m_BlockDataByRegion.Find(RegionToSet);
	}
	else
	{
		CreateNewRegionData();
	}
}


void USpellPlanesGameInstance::OpenTheWorldLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainWorldLevel");
}
