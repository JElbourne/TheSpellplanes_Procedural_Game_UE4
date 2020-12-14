// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "World/WorldUtilities.h"
#include "Blocks/ResourceBlock.h"
#include "Blocks/GroundTerrain.h"
#include "SpellPlanesSaveGame.generated.h"


/**
 * GRID SERIALIZATION CONCEPT FOR SAVING WORLD GRID DATA
 * 
 * Each GRID location will be stored as an signed int32 (eg. -1,999,999,999) - 4 BYTES
 * This will be a basic serialization of all the information on the grid.
 *
 * This information will be stored as follows:
 *
 * -/+ This acts as a switch and usually doubles the possibilities of each ID
 * xxxxxxxxx9 - Ground Layer (ID render 0-9). [20 possibilities]
 * xxxxxxxx9x - Ground Modifier ID (ID render 0-9). [20 possibilities] [0 being nothing]
 * xxxxxx99xx - Block Type (ID render 0-99). [200 possibilities] [0 being nothing]
 * xxx999xxxx - Pickup Item Type (ID render 0-999). [2000 possibilities] [0 being nothing]
 * x99xxxxxxx - Pickup Item Quantity (ID render 0-99). [200 max, the -/+ acts as a doubler for this] [starts counting at 0, +1]
 * 1xxxxxxxxx - This can only be set to either 1 or 0. It will act as a secondary switch/flag on the block.
 *
 * There will be a requirement for the provided value to be LESS than 2,000,000,000

 // Example program
#include <iostream>
#include <string>

int num = 1573965734;
int desirednum = 1572555734;

int Modulus = 0;
int PlaceModifier = 1;

void SetModulusAndPlaceModifier(int Index, int &Modulus, int &PlaceModifier)
{
	switch (Index)
	{
		case 6:
		{
			Modulus = 10;
			PlaceModifier = 1;
		}
		break;
		case 5:
		{
			Modulus = 100;
			PlaceModifier = 10;
		}
		break;
		case 4:
		{
			Modulus = 10000;
			PlaceModifier = 100;
		}
		break;
		case 3:
		{
			Modulus = 10000000;
			PlaceModifier = 10000;
		}
		break;
		case 2:
		{
			Modulus = 1000000000;
			PlaceModifier = 10000000;
		}
		break;
		case 1:
		{
			Modulus = 10000000000;
			PlaceModifier = 1000000000;
		}
		break;
		default:
		{
			Modulus = 0;
			PlaceModifier = 1;
		}
	}
}


//
int GetBlockIdFromGridId(int GridId, int Modulus, int PlaceModifier)
{
	return (GridId % Modulus) / PlaceModifier;
}

//
void SetBlockIdInGridId(int &GridId, int ValueToSet[7])
{
	// Manual Clamp Values
	ValueToSet[0] = (ValueToSet[0] < -1) ? -1 : (1 < ValueToSet[0]) ? 1 : ValueToSet[0];
	ValueToSet[1] = (ValueToSet[1] < 0) ? 0 : (1 < ValueToSet[1]) ? 1 : ValueToSet[1];
	ValueToSet[2] = (ValueToSet[2] < 0) ? 0 : (99 < ValueToSet[2]) ? 99 : ValueToSet[2];
	ValueToSet[3] = (ValueToSet[3] < 0) ? 0 : (999 < ValueToSet[3]) ? 999 : ValueToSet[3];
	ValueToSet[4] = (ValueToSet[4] < 0) ? 0 : (99 < ValueToSet[4]) ? 99 : ValueToSet[4];
	ValueToSet[5] = (ValueToSet[5] < 0) ? 0 : (9 < ValueToSet[5]) ? 9 : ValueToSet[5];
	ValueToSet[6] = (ValueToSet[6] < 0) ? 0 : (9 < ValueToSet[6]) ? 9 : ValueToSet[6];


	int NumsToAdd = 0;

	for (int i = 1; i < 7 ; i++)
	{
	   std::cout << "Value to set: " << ValueToSet[i] << "!\n";
	   SetModulusAndPlaceModifier(i, Modulus, PlaceModifier);
	   if (ValueToSet[i] < 0)
	   {
			int val = GetBlockIdFromGridId(GridId, Modulus, PlaceModifier);
			NumsToAdd += (val * PlaceModifier);
	   }
	   else
	   {
			NumsToAdd += (ValueToSet[i] * PlaceModifier);
	   }
	}

	if (ValueToSet[0] != 0) NumsToAdd *= ValueToSet[0];

	GridId = NumsToAdd;
}


int main()
{
	int GridId = 1000000009;
	// Ensure no leading Zero which tells combiler the number is Octal....this will break the ID system.
	// Order Of Data: { +/- Switch, 0/1 Flag, Pickup Item Qnt., Pickup Item ID, Block ID, Ground Modifier, Ground Layer ID }
	int ValueToSet[7] = { -1, 1, 10, 945, 33, 0, 1 };

	SetBlockIdInGridId(GridId, ValueToSet);
	std::cout << "New Number: " << GridId << "!\n";
}
 */

UENUM(BlueprintType)
enum class ESaveSlots : uint8
{
	SS_Slot1 UMETA(DisplayName = "Save Slot 1"),
	SS_Slot2 UMETA(DisplayName = "Save Slot 2"),
	SS_Slot3 UMETA(DisplayName = "Save Slot 3"),
};

USTRUCT(BlueprintType)
struct THESPELLPLANES_API FPlayerCharacterData
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	FVector PlayerLocation;

	UPROPERTY()
	EWorldRegionType CurrentRegion;

};

USTRUCT(BlueprintType)
struct THESPELLPLANES_API FZoneGridData
{
	GENERATED_BODY()
public:
	/**
	* All the saved grid data.
	* Refer to the GRID SERIALIZATION CONCEPT written above for how this works
	* This Array is initialized as an Inline Allocator or an initial size of Zone Grid Spaces
	*/
	TArray<int32, TInlineAllocator<400>> GridData;
};

USTRUCT(BlueprintType)
struct THESPELLPLANES_API FRegionData
{
	GENERATED_BODY()
public:
	/** Human friendly name for the Region.  */
	UPROPERTY()
	FString RegionName;
	
	/**
	* A Solution Path is used to procedurally generate a start of the world and a path that works to various goal zones.
	* This ensures that we generate a random world of blocks that has a clear path through it
	* so the player is not stuck at the beginning of a new world.
	*/
	UPROPERTY()
	TMap<FIntVector, EZoneType> SolutionPathData;

	/**
	* All the saved Zone data.
	*/
	UPROPERTY()
	TMap<FIntVector, FZoneGridData> ZoneData;

};

USTRUCT(BlueprintType)
struct THESPELLPLANES_API FWorldData
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY()
	FString WorldName = "The SpellPlanes";

	UPROPERTY()
	uint8 SolutionPathLength = 10;

	UPROPERTY()
	TMap<EWorldRegionType, FRegionData> BlockDataByRegion;

	FWorldData(): WorldName(TEXT("SpellPlanes")), SolutionPathLength(10) {}
};

/**
 * 
 */
UCLASS()
class THESPELLPLANES_API USpellPlanesSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	USpellPlanesSaveGame();

	UPROPERTY(EditAnywhere)
	FWorldData WorldData;

	UPROPERTY(EditAnywhere)
	FPlayerCharacterData PlayerCharacterData;

};
