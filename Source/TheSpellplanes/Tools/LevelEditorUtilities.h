// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "World/WorldUtilities.h"
#include "LevelEditorUtilities.generated.h"


UENUM(BlueprintType)
enum class EJsonFileName : uint8
{
	JFI_SP_BaseZones UMETA(DisplayName = "Base_Zone_Templates"),
	JFI_SP_SpecialZones UMETA(DisplayName = "Special_Zone_Templates"),
};

/**
 * 
 */
UCLASS()
class THESPELLPLANES_API ULevelEditorUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	ULevelEditorUtilities();

	UFUNCTION(BlueprintCallable, Category = "SpellPlanes Level Editor")
	static FZoneTypeTemplates RefreshZoneTemplatesFromFile(EJsonFileName FileName);

	UFUNCTION(BlueprintCallable, Category = "SpellPlanes Level Editor", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static void CreateZoneGridDataFromWorld(UObject* WorldContextObject, EJsonFileName FileName, UPARAM(ref) FZoneTypeTemplates& ZoneTypeTemplatesStruct, EZoneType ZoneType, bool bEnsureUniqueTemplate = true);

	UFUNCTION(BlueprintCallable, Category = "SpellPlanes Level Editor", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static void ClearAllZoneActorsFromWorld(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "SpellPlanes Level Editor")
	static FText GetZoneTypeAtText(EZoneType ZoneType);

	UFUNCTION(BlueprintCallable, Category = "SpellPlanes Level Editor")
	static bool DeleteTemplateVariation(EJsonFileName FileName, EZoneType ZoneType, int32 VariationIndex, UPARAM(ref) FZoneTypeTemplates& ZoneTypeTemplatesStruct);

	UFUNCTION(BlueprintCallable, Category = "SpellPlanes Level Editor", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static bool LoadTemplateVariation(UObject* WorldContextObject, EJsonFileName FileName, EZoneType ZoneType, int32 VariationIndex, UPARAM(ref) FZoneTypeTemplates& ZoneTypeTemplatesStruct);

	/** This is used to set an individual value for the GridID. */
	//UFUNCTION(BlueprintCallable, Category = "SpellPlanes Level Editor")
	//static void SetValueInGridIdBP(UPARAM(ref) int32& GridIdIn, int32 IdValue, EBlockIdIndex Index, int32& GridIdOut);
	

private:
	
	static bool SaveZoneGridIdToJsonObject(EJsonFileName FileName, TArray<int32> ZoneTemplateData, FZoneTypeTemplates& ZoneTypeTemplatesStruct, EZoneType ZoneType, bool bEnsureUniqueTemplate);
	static bool IsBlockOutOfBounds(FVector BlockLocation);
	static int16 GetZoneGridArrayIndex(FVector BlockLocation);
	static bool SaveJsonObjectToFile(EJsonFileName FileName, FZoneTypeTemplates& ZoneTypeTemplatesStruct);

	/** Must be private - Costly Algorithm.
	* This function will loop through all Template Variations, then loop through all BlockId's to ensure NewZoneTemplate is unique.
	*/
	static bool CheckIfUniqueZoneTemplate(TArray<FZoneTemplate> TemplateVariations, TArray<int32> NewZoneTemplate);

};
