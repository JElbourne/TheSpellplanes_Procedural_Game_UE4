// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "JsonUtilities.h"
#include "World/WorldUtilities.h"
#include "Blocks/BlockUtilities.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blocks/Block.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULevelEditorUtilities::ULevelEditorUtilities()
{
	
}

FZoneTypeTemplates ULevelEditorUtilities::RefreshZoneTemplatesFromFile(EJsonFileName FileName)
{
	// Set complete file path.
	FString m_FileName = EnumToString(EJsonFileName, FileName);
	FString JsonFilePath = FPaths::ProjectContentDir() + "/TheSpellplanes/JSON/" + m_FileName + ".json";

	FZoneTypeTemplates ZoneTypeTemplatesStruct;

	// Load the existing file (if any)
	FString JSONPayload;
	if (FFileHelper::LoadFileToString(JSONPayload, *JsonFilePath))
	{
		GLog->Log("File found and loaded");
		if (FJsonObjectConverter::JsonObjectStringToUStruct(JSONPayload, &ZoneTypeTemplatesStruct, 0, 0))
		{
			GLog->Log("ZoneTemplateVariations loaded!");
		}
		else
		{
			GLog->Log("ZoneTemplateVariations could not be loaded from JSON.");
		}
	}
	return ZoneTypeTemplatesStruct;
}


void ULevelEditorUtilities::CreateZoneGridDataFromWorld(UObject* WorldContextObject, EJsonFileName FileName, FZoneTypeTemplates& ZoneTypeTemplatesStruct, EZoneType ZoneType, bool bEnsureUniqueTemplate /*= true*/)
{
	TArray<int32> m_ZoneTemplateData;
	m_ZoneTemplateData.Init(0, UWorldUtilities::ZONE_BLOCK_AREA);

	TArray<AActor*> FoundBlockActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject->GetWorld(), ABlock::StaticClass(), FoundBlockActors);

	UE_LOG(LogTemp, Warning, TEXT("Number of Blocks Found: %i"), FoundBlockActors.Num());

	bool Save = true;
	for (AActor* BlockActor : FoundBlockActors)
	{
		ABlock* Block = Cast<ABlock>(BlockActor);
		if (Block != nullptr)
		{
			GLog->Log("Block Found and Cast to ABlock.");

			if (IsBlockOutOfBounds(Block->GetActorLocation()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Block found is out of bounds so Canceling the Save."));
				Save = false;
				break;
			}

			int16 ArrayIndex = GetZoneGridArrayIndex(Block->GetActorLocation());
			UE_LOG(LogTemp, Warning, TEXT("GetZoneGridArrayIndex: %i"), ArrayIndex);

			if (m_ZoneTemplateData.IsValidIndex(ArrayIndex))
			{
				//UE_LOG(LogTemp, Warning, TEXT("Setting Value in in m_ZoneTemplateData, which has length of: %i"), m_ZoneTemplateData.Num());
				//UE_LOG(LogTemp, Warning, TEXT("Value Before: %i"), m_ZoneTemplateData[ArrayIndex]);
				//UE_LOG(LogTemp, Warning, TEXT("m_BlockId: %i"), Block->m_BlockId);
				//UE_LOG(LogTemp, Warning, TEXT("m_BlockIdIndex: %i"), Block->m_BlockIdIndex);
				UBlockUtilities::SetValueInGridId(m_ZoneTemplateData[ArrayIndex], Block->m_BlockId, Block->BlockIdIndex);
				//UE_LOG(LogTemp, Warning, TEXT("Value After: %i"), m_ZoneTemplateData[ArrayIndex]);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Index not found in m_ZoneTemplateData, which has length of: %i"), m_ZoneTemplateData.Num());
			}
		}
	}
	// After going through all the Blocks in the Level, we save to the file.
	if (Save)
	{
		SaveZoneGridIdToJsonObject(FileName, m_ZoneTemplateData, ZoneTypeTemplatesStruct, ZoneType, bEnsureUniqueTemplate);
	}
}

void ULevelEditorUtilities::ClearAllZoneActorsFromWorld(UObject* WorldContextObject)
{
	TArray<AActor*> FoundBlockActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject->GetWorld(), ABlock::StaticClass(), FoundBlockActors);

	UE_LOG(LogTemp, Warning, TEXT("Number of Blocks Found: %i"), FoundBlockActors.Num());

	bool Save = true;
	for (AActor* BlockActor : FoundBlockActors)
	{
		if (!BlockActor->IsValidLowLevel()) return;
		BlockActor->Destroy();
	}

	//GC
	GEngine->ForceGarbageCollection(true);
}

FText ULevelEditorUtilities::GetZoneTypeAtText(EZoneType ZoneType)
{
	return EnumToText(EZoneType, ZoneType);
}

bool ULevelEditorUtilities::DeleteTemplateVariation(EJsonFileName FileName, EZoneType ZoneType, int32 VariationIndex, FZoneTypeTemplates& ZoneTypeTemplatesStruct)
{
	FZoneTemplateVariations TemplateVariationsStruct;

	// Find the Templates associated with the given ZoneType
	if (ZoneTypeTemplatesStruct.ZoneTypeTemplates.Contains(ZoneType))
	{
		TemplateVariationsStruct = *ZoneTypeTemplatesStruct.ZoneTypeTemplates.Find(ZoneType);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DeleteTemplateVariation: Could not find ZoneType in ZoneTypeTemplatesStruct.ZoneTypeTemplates."));
		return false;
	}

	// If Index given is Valid we will remove the Template Variation from the Array at that index.
	if (TemplateVariationsStruct.LayoutVariations.IsValidIndex(VariationIndex))
	{
		TemplateVariationsStruct.LayoutVariations.RemoveAt(VariationIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DeleteTemplateVariation: Not a valid index: %i."), VariationIndex);
		return false;
	}

	// We add the ZoneTemplateVariationsStruct to the Map of ZoneTypes in ZoneTypeTemplatesStruct
	ZoneTypeTemplatesStruct.ZoneTypeTemplates.Add(ZoneType, TemplateVariationsStruct);

	return SaveJsonObjectToFile(FileName, ZoneTypeTemplatesStruct);
}

bool ULevelEditorUtilities::LoadTemplateVariation(UObject* WorldContextObject, EJsonFileName FileName, EZoneType ZoneType, int32 VariationIndex, FZoneTypeTemplates& ZoneTypeTemplatesStruct)
{
	FZoneTemplateVariations TemplateVariationsStruct;

	// Find the Templates associated with the given ZoneType
	if (ZoneTypeTemplatesStruct.ZoneTypeTemplates.Contains(ZoneType))
	{
		TemplateVariationsStruct = *ZoneTypeTemplatesStruct.ZoneTypeTemplates.Find(ZoneType);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadTemplateVariation: Could not find ZoneType in ZoneTypeTemplatesStruct.ZoneTypeTemplates."));
		return false;
	}

	FZoneTemplate ZoneTemplateData;
	// If Index given is Valid we will remove the Template Variation from the Array at that index.
	if (TemplateVariationsStruct.LayoutVariations.IsValidIndex(VariationIndex))
	{
		ZoneTemplateData = TemplateVariationsStruct.LayoutVariations[VariationIndex];
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadTemplateVariation: Not a valid index: %i."), VariationIndex);
		return false;
	}

	//UWorldUtilities::DrawBlocks(WorldContextObject, FIntVector(0,0,0), ZoneTemplateData.BlockIdLayout);

	return true;
}

//void ULevelEditorUtilities::SetValueInGridIdBP(int32& GridIdIn, int32 IdValue, EBlockIdIndex Index, int32& GridIdOut)
//{
//	UBlockUtilities::SetValueInGridId(GridIdIn, IdValue, Index);
//	GridIdOut = GridIdIn;
//}

bool ULevelEditorUtilities::SaveZoneGridIdToJsonObject(EJsonFileName FileName, TArray<int32> ZoneTemplateData, FZoneTypeTemplates& ZoneTypeTemplatesStruct, EZoneType ZoneType, bool bEnsureUniqueTemplate)
{
	ZoneTypeTemplatesStruct = RefreshZoneTemplatesFromFile(FileName);

	// Initialize the ZoneTypeTemplates Struct
	FZoneTemplateVariations TemplateVariationsStruct;

	if (ZoneTypeTemplatesStruct.ZoneTypeTemplates.Contains(ZoneType))
	{
		TemplateVariationsStruct = *ZoneTypeTemplatesStruct.ZoneTypeTemplates.Find(ZoneType);
	}

	// We check to see if we want to ensure a unique template.
	if (bEnsureUniqueTemplate)
	{
		GLog->Log("bEnsureUniqueTemplate set to true, we will check for uniqueness.");
		// We check if the ZoneTemplate is unique, this is a big process of looping through every single block id.
		if (!CheckIfUniqueZoneTemplate(TemplateVariationsStruct.LayoutVariations, ZoneTemplateData))
		{
			// If not unique then we don't save the file.
			GLog->Log("The template was not Unique, so we did not save.");
			return false;
		}
	}
	
	// Creates The BlockIdLayout
	FZoneTemplate ZoneTemplateStruct;
	ZoneTemplateStruct.BlockIdLayout = ZoneTemplateData;

	// We add the Zone Template to the Array in the Struct
	TemplateVariationsStruct.LayoutVariations.Add(ZoneTemplateStruct);
	// We add the ZoneTemplateVariationsStruct to the Map of ZoneTypes in ZoneTypeTemplatesStruct

	ZoneTypeTemplatesStruct.ZoneTypeTemplates.Add(ZoneType, TemplateVariationsStruct);
	

	/////////////////////////////////////////////
	// Try Something
	////////////////////////////////////////////

	//TArray< TSharedPtr<FJsonValue> > ObjArray;
	////TSharedRef< FJsonObject > JsonObj = MakeShareable(new FJsonObject);

	//// Loop through all the Zone Types
	//// Make JsonObjects out of the Structs
	//TSharedPtr< FJsonObject > JsonObj = FJsonObjectConverter::UStructToJsonObject(TemplateVariationsStruct, 0, 0);
	//// Add the Zone Type Json Object to a JsonValueObject so it can be added to an array
	//TSharedRef< FJsonValueObject > JsonValue = MakeShareable(new FJsonValueObject(JsonObj));
	//// Add the Json Value to an array
	//ObjArray.Add(JsonValue);
	////Exit Loop

	//// Create a final Json Object to add the array to
	//TSharedPtr< FJsonObject > FinalJsonObj = MakeShareable(new FJsonObject);
	//FinalJsonObj->SetArrayField("array", ObjArray);

	//FString OutputString;
	//TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	//FJsonSerializer::Serialize(FinalJsonObj.ToSharedRef(), Writer);

	//UE_LOG(LogTemp, Warning, TEXT("resulting jsonString -> %s"), *OutputString);

	return SaveJsonObjectToFile(FileName, ZoneTypeTemplatesStruct);

}

bool ULevelEditorUtilities::IsBlockOutOfBounds(FVector BlockLocation)
{
	float GridBounds = (float)(UWorldUtilities::BLOCK_SIZE * (UWorldUtilities::ZONE_LENGTH_IN_BLOCKS - 1));
	if (BlockLocation.X < 0 || BlockLocation.X > GridBounds || BlockLocation.Y < 0 || BlockLocation.Y > GridBounds)
	{
		//m_Message = "BLOCK OUT OF GRID BOUNDS";
		return true;
	}
	return false;
}

int16 ULevelEditorUtilities::GetZoneGridArrayIndex(FVector BlockLocation)
{
	return UKismetMathLibrary::Abs(UKismetMathLibrary::Round(BlockLocation.X / UWorldUtilities::BLOCK_SIZE) + ((UKismetMathLibrary::Round(BlockLocation.Y / UWorldUtilities::BLOCK_SIZE)) * UWorldUtilities::ZONE_LENGTH_IN_BLOCKS));
}

bool ULevelEditorUtilities::SaveJsonObjectToFile(EJsonFileName FileName, FZoneTypeTemplates& ZoneTypeTemplatesStruct)
{
	FString OutputString;
	FJsonObjectConverter::UStructToJsonObjectString(ZoneTypeTemplatesStruct, OutputString, 0, 0);
	GLog->Log(OutputString);
	// Set complete file path.
	FString m_FileName = EnumToString(EJsonFileName, FileName);
	FString JsonFilePath = FPaths::ProjectContentDir() + "/TheSpellplanes/JSON/" + m_FileName + ".json";
	return FFileHelper::SaveStringToFile(OutputString, *JsonFilePath);
}

bool ULevelEditorUtilities::CheckIfUniqueZoneTemplate(TArray<FZoneTemplate> TemplateVariations, TArray<int32> NewZoneTemplate)
{
	if (TemplateVariations.Num() < 1)
	{
		// If there are no template variations to start we are done.
		return true;
	}

	for (FZoneTemplate &TemplateVariation : TemplateVariations)
	{
		bool isUnique = false;
		if (TemplateVariation.BlockIdLayout.Num() != NewZoneTemplate.Num())
		{
			isUnique = true;
		}
			
		for (int16 i = 0; i < TemplateVariation.BlockIdLayout.Num(); i++)
		{
			if (TemplateVariation.BlockIdLayout[i] != NewZoneTemplate[i])
			{
				isUnique = true;
			}	
		}
		
		if (!isUnique)
		{
			// If the flag isUnique was never switched to true, that mean we have a duplicate and we can immediately return false.
			return false;
		}
	}
	// Returns true if was never discovered to be not unique while looping.
	return true;
}
