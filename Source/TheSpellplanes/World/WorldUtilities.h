// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "WorldUtilities.generated.h"

#define EnumToString(EnumClassName, ValueOfEnum) GetEnumValueAsString<EnumClassName>(FString(TEXT(#EnumClassName)), (ValueOfEnum))
#define EnumToText(EnumClassName, ValueOfEnum) GetEnumValueAsText<EnumClassName>(FString(TEXT(#EnumClassName)), (ValueOfEnum))
#define EnumValueName(EnumClassName, ValueOfEnum) GetEnumValueAsName<EnumClassName>(FString(TEXT(#EnumClassName)), (ValueOfEnum))

template<typename TEnum>
static FORCEINLINE FString GetEnumValueAsString(const FString& Name, TEnum Value) {
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	if (!enumPtr) return FString("Invalid");
	return enumPtr->GetDisplayNameTextByValue((int64)Value).ToString();
}

template<typename TEnum>
static FORCEINLINE FText GetEnumValueAsText(const FString& Name, TEnum Value) {
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	if (!enumPtr) return FText::GetEmpty();
	return enumPtr->GetDisplayNameTextByValue((int64)Value);
}

template<typename TEnum>
static FORCEINLINE FName GetEnumValueAsName(const FString& Name, TEnum Value) {
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	if (!enumPtr) return NAME_None;
	return enumPtr->GetNameByValue((int64)Value);
}

UENUM(BlueprintType)
enum class EWorldRegionType : uint8
{
	WRT_Savana UMETA(DisplayName = "Savana Region"),
};

UENUM(BlueprintType)
enum class EZoneType : uint8
{
	PZT_LR_Base UMETA(DisplayName = "Left and Right"),
	PZT_LRT_Base UMETA(DisplayName = "Left, Right, and Top"),
	PZT_LRTB_Base UMETA(DisplayName = "Left, Right, Top and Bottom"),
	PZT_LRB_Base UMETA(DisplayName = "Left, Right, Bottom"),
	PZT_Fill_Base UMETA(DisplayName = "Fill"),
};

UENUM(BlueprintType)
enum class ESpecialZoneType : uint8
{
	SZT_Start UMETA(DisplayName = "Start Zone"),
	SZT_End UMETA(DisplayName = "End Zone"),
};

USTRUCT(BlueprintType)
struct FZoneTemplate : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	FZoneTemplate() {};

	/** The Block ID. This is a Signed Int16 so we can have negative values to will be cool as a Opposite block type (183 versus -183)  */
	UPROPERTY()
	TArray<int32> BlockIdLayout;
};

USTRUCT(BlueprintType)
struct FZoneTemplateVariations : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	FZoneTemplateVariations() {};

	/** An Array of all the different variations for a Zone Type */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Type Templates")
	TArray<FZoneTemplate> LayoutVariations;
};

USTRUCT(BlueprintType)
struct FZoneTypeTemplates : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	FZoneTypeTemplates() {};

	/** An Map of all the different Zone Type to their variations Array */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Type Templates")
	TMap<EZoneType, FZoneTemplateVariations> ZoneTypeTemplates;
};


UCLASS()
class THESPELLPLANES_API UWorldUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UWorldUtilities();

	/** Block Constants */
	static const uint8 BLOCK_SIZE = 100;

	/** Zone Constants */
	static const uint8 ZONE_LENGTH_IN_BLOCKS = 24;
	static const int32 ZONE_SIZE = BLOCK_SIZE * ZONE_LENGTH_IN_BLOCKS;
	static const int32 ZONE_BLOCK_AREA = ZONE_LENGTH_IN_BLOCKS * ZONE_LENGTH_IN_BLOCKS;

	/** This is for destroying Zones once the player/camera has reached this set distance (number of Zones away). */
	static const int32 MAX_ZONE_DISTANCE = 2;

	/** Fog Of War Constants */
	static const int32 FOG_DISTANCE = 4; // Must divisible by 2
	static const int32 FOG_FADE_STAGE_ONE = 40; // Percent 50% opacity
	static const int32 FOG_FADE_STAGE_TWO = 25; // Percent 35% opacity

	/** World Constants */

	/** IMPORTANT VALUE DO NOT CHANGE
	* This is the total number of Zones the game will allow across the map (both directions)
	* This Number is used along with the Terrain Generator SEED to determine the terrain layout for Zones.
	* That way we don't need to store all the Procedural/Random Noise zones, we just need the SEED.
	* An Int32 was chosen as it is plenty big enough.
	* Max unsigned Integer for uint32 is 4,294,967,295. (We use 4000 for our initial Seed max value.)
	* Seed Value Equation is (4000 * 1000000) + (y * WORLD_MAX_WIDTH_IN_ZONES + x)
	* Where y & x can have Max Values of (WORLD_MAX_WIDTH_IN_ZONES-1)
	* We can work out the largest seed value can be (4000000000 + (17149 * 17150) + 17149) = 4,294,122,499
	* This is used in the equation to determine array index ( i = y * width + x; )
	*/
	static const uint32 WORLD_MAX_WIDTH_IN_ZONES = 17150;


	/**
	* This is the Data Table that holds all the Special Zone Variation Templates.
	* These templates are use to Adjust the Procedurally generated Path to have Special Zones the level designer can setup.
	* It is a way of controlling the design and experience while still having Procedural Levels.
	*/
	//static class UDataTable* SpecialZoneTemplates;

	//static class UDataTable* ZoneTemplates;

	/** Given a Zone Type it will try and find a Template from the DataTable */
	static void GetBaseZoneTemplate(EZoneType ZoneType, TArray<int32, TInlineAllocator<400>>& ZoneTemplate);

	/** Will modify the existing Zone Template to match the Special Zone Template */
	static void AdjustZoneTemplateForSpecialtyZone(ESpecialZoneType SpecialZoneType, TArray<int32, TInlineAllocator<400>>& ZoneTemplate);

	/** Will Get the Zone's Center location from a specified Location inside that Zone */
	static FVector GetZoneCenterFromLocation(FVector FloatLocation);

	/** Will Get the Zone's Grid Coordinate from a specified Location inside that Zone and a Current Elevation Level */
	static FIntVector GetZoneGridCoordFromLocation(FVector Location, int32 CurrentElevation);

	/** Will take a Location Vector and Round it to snap to a Grid Coordinate */
	static FVector RoundLocationToGridSize(FVector FloatLocation, bool bUseRotation, int8 rotationDegrees, int BlockSize);

	/** This is a fun algorithm to create rooms ("zones") that connect and form a path of a certain length.  */
	static void CreateSolutionPath(const uint8 LengthOfSolutionPath, TMap<FIntVector, EZoneType>& SolutionPathData, TMap<FIntVector, ESpecialZoneType>& SpecialZoneLocations);


};