// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlockUtilities.generated.h"

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
*/

UENUM(BlueprintType)
enum class EBlockIdIndex : uint8
{
	BII_PosNeg = 0,
	BII_IsLocked = 1,
	BII_ItemQuantity = 2,
	BII_ItemType = 3,
	BII_BlockType = 4,
	BII_ModifierType = 5,
	BII_GroundType = 6,
};


UCLASS()
class THESPELLPLANES_API UBlockUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UBlockUtilities();

	/**
	* This will update the Static Variable Values for Modulus and PlaceModifier depending on the index with in the GridID you are looking for.
	* Serialization Technique has a Unique Pattern for the GridID so we must manually set unique Modulus and PlaceModifiers for the index of ID
	*/
	static void SetModulusAndPlaceModifier(EBlockIdIndex Index);

	/**
	* This will get the Value from the GridID give a Value Index.
	* It is important we don't use zero as Modulus with the compiler so we check, this happens with the First digit.
	*/
	//UFUNCTION(BlueprintCallable, Category = "Block Utilities")
	static int16 GetValueFromGridId(int32 GridId, EBlockIdIndex Index);

	/**
	* This will set the GridId based on an Array Of 7 Values supplied
	* Ensure no leading Zero which tells compiler the number is Octal....this will break the ID system.
	* Order Of Data: { +/- Switch, 0/1 Flag, Pickup Item Qnt., Pickup Item ID, Block ID, Ground Modifier, Ground Layer ID }
	*/
	static void SetValuesInGridId(int32& GridId, int16 ValuesToSet[7]);
	
	/** This is used to set an individual value. */
	static void SetValueInGridId(int32& GridIdIn, int16 IdValue, EBlockIdIndex Index);

private:
	/**
	* The Modulus is used to pull the right position of a Value from the ID
	* The PlaceModifier is to remove the trailing digits from the Value being extracted from the ID.
	*/
	static int64 s_IdModulus;
	static int32 s_IdPlaceModifier;


	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////

	/**
	* The below test code has been re-written into a Automated Test SPEC file called BlockUtilities.spec.cpp
	* It uses the in-engine Test Front end Plugin.
	* Code will remain here for now.
	*/
	/** Can be used to test the methods. */
	UFUNCTION(BlueprintCallable, Category = "Block Utilities")
	static void TestBlockUtilities();
	static void PrintTestResult(bool bIsSuccess);
	
};
