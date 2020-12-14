// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldUtilities.h"
#include "Kismet/KismetMathLibrary.h"
#include "Framework/SpellPlanesGameStateBase.h"
#include "Engine/DataTable.h"
#include "Blocks/GroundTerrain.h"

#define KMath UKismetMathLibrary

UWorldUtilities::UWorldUtilities()
{

	//static ConstructorHelpers::FObjectFinder<UDataTable> SpecialZoneTemplatesObject(TEXT("DataTable'/Game/TheSpellplanes/Data/SpecialRoomTemplates.SpecialRoomTemplates'"));
	//if (SpecialZoneTemplatesObject.Succeeded())
	//{
	//	SpecialZoneTemplates = SpecialZoneTemplatesObject.Object;
	//}

	//static ConstructorHelpers::FObjectFinder<UDataTable> ZoneTemplatesObject(TEXT("DataTable'/Game/TheSpellplanes/Data/RoomTemplates.RoomTemplates'"));
	//if (ZoneTemplatesObject.Succeeded())
	//{
	//	ZoneTemplates = ZoneTemplatesObject.Object;
	//}
}

void UWorldUtilities::GetBaseZoneTemplate(EZoneType ZoneType, TArray<int32, TInlineAllocator<400>>& BlockIDLayout)
{
	UDataTable* ZoneTemplates = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, TEXT("DataTable'/Game/TheSpellplanes/Data/RoomTemplates.RoomTemplates'")));
	if (ZoneTemplates)
	{
		static const FString ContextString(TEXT("ZoneTemplateVariations"));
		if (FZoneTemplateVariations* ZoneTemplateVariations = ZoneTemplates->FindRow<FZoneTemplateVariations>(EnumValueName(EZoneType, ZoneType), ContextString, true))
		{
			TArray<FZoneTemplate> ZoneVariations = ZoneTemplateVariations->LayoutVariations;
			FZoneTemplate SPZoneTemplate = ZoneVariations[FMath::RandHelper(ZoneVariations.Num() - 1)];
			BlockIDLayout = SPZoneTemplate.BlockIdLayout;
		}
	}
}

void UWorldUtilities::AdjustZoneTemplateForSpecialtyZone(ESpecialZoneType SpecialZoneType, TArray<int32, TInlineAllocator<400>>& ZoneTemplate)
{
	UDataTable* SpecialZoneTemplates = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, TEXT("DataTable'/Game/TheSpellplanes/Data/SpecialRoomTemplates.SpecialRoomTemplates'")));
	if (SpecialZoneTemplates)
	{
		static const FString ContextString(TEXT("ZoneTemplateVariations"));
		if (FZoneTemplateVariations* ZoneTemplateVariations = SpecialZoneTemplates->FindRow<FZoneTemplateVariations>(EnumValueName(ESpecialZoneType, SpecialZoneType), ContextString, true))
		{
			TArray<FZoneTemplate> ZoneVariations = ZoneTemplateVariations->LayoutVariations;
			FZoneTemplate SPZoneTemplate = ZoneVariations[FMath::RandHelper(ZoneVariations.Num() - 1)];
			TArray<int32> BlockIDLayout = SPZoneTemplate.BlockIdLayout;
			for (uint8 i = 0; i < BlockIDLayout.Num(); i++)
			{
				int32 CurBlockID = BlockIDLayout[i];
				if (CurBlockID > 0 && ZoneTemplate.IsValidIndex(i))
				{
					ZoneTemplate.Insert(CurBlockID, i);
				}
			}
		}
	}
}

FVector UWorldUtilities::GetZoneCenterFromLocation(FVector Loc)
{
	return FVector((float)KMath::Round(Loc.X / ZONE_SIZE) * ZONE_SIZE, (float)KMath::Round(Loc.Y / ZONE_SIZE) * ZONE_SIZE, Loc.Z);
}

FIntVector UWorldUtilities::GetZoneGridCoordFromLocation(FVector Loc, int32 CurrentElevation)
{
	int16 Xval = KMath::Round(Loc.X / ZONE_SIZE);
	int16 Yval = KMath::Round(Loc.Y / ZONE_SIZE);
	int32 Zval = CurrentElevation;
	return FIntVector(Xval, Yval, Zval);
}

FVector UWorldUtilities::RoundLocationToGridSize(FVector Loc, bool bUseRotation, int8 RotationDegrees, int BlockSize)
{
	/** This Map is used to determine blocks around the player character that can be selected based on an angle.*/
	TMap<int32, FVector2D> OffsetDirection;
	OffsetDirection.Add(-45, FVector2D(1, -1));
	OffsetDirection.Add(-90, FVector2D(0, -1));
	OffsetDirection.Add(-135, FVector2D(-1, -1));
	OffsetDirection.Add(-180, FVector2D(-1, 0));
	OffsetDirection.Add(135, FVector2D(-1, 1));
	OffsetDirection.Add(90, FVector2D(0, 1));
	OffsetDirection.Add(45, FVector2D(1, 1));
	OffsetDirection.Add(0, FVector2D(1, 0));

	if (bUseRotation)
	{
		FVector2D Offset = *OffsetDirection.Find(RotationDegrees);
		int8 AdjustX = BlockSize / 2 * Offset.X;
		int8 AdjustY = BlockSize / 2 * Offset.Y;
		return FVector(((float)KMath::Round(Loc.X / BlockSize) * BlockSize) + AdjustX, ((float)KMath::Round(Loc.Y / BlockSize) * BlockSize) + AdjustY, Loc.Z);
	}
	return FVector((float)KMath::Round(Loc.X / BlockSize) * BlockSize, (float)KMath::Round(Loc.Y / BlockSize) * BlockSize, Loc.Z);
}

void UWorldUtilities::CreateSolutionPath(const uint8 LengthOfSolutionPath, TMap<FIntVector, EZoneType>& SolutionPathData, TMap<FIntVector, ESpecialZoneType>& SpecialZoneLocations)
{
	// Lets Clear the existing Solution Path Data
	SolutionPathData.Empty();

	// Set Variables
	int8 Direction = 1;
	int8 UpCounter = 0;
	bool bIsGenerating = true;
	EZoneType TempSelectedZoneType;
	FIntVector CurrentPosition = FIntVector(0, 0, 0);
	
	// Setup the Starting point or our path. (Rooms 1 and 2 are best for starting rooms.)
	SpecialZoneLocations.Add(CurrentPosition, ESpecialZoneType::SZT_Start);
	SolutionPathData.Add(CurrentPosition, (EZoneType)FMath::RandRange(0, 1));
	// Set initial direction to travel
	Direction = FMath::RandRange(1, 5);

	while (bIsGenerating)
	{
		// Check if the Current Length is greater than our goal...if it is then something went wring and exit the loop.
		if (SolutionPathData.Num() >= LengthOfSolutionPath)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateSolutionPath: Existing the loop early, reached Length ahead of schedule."));
			bIsGenerating = false;
			break;
		}
			
		if (Direction == 1) {
			// This is the rarest case for Direction and we use it to go up in direction
			UpCounter++;
			// We must ensure the current room has a Top opening
			if (!SolutionPathData.Contains(CurrentPosition))
			{
				bIsGenerating = false; // We can not find our current Zone. Something is wrong.
				UE_LOG(LogTemp, Warning, TEXT("CreateSolutionPath: Existing the loop early, Current Room Not in Room List and we have to move up.."));
				break;
			}
			// Next we need to ensure the current ZONE has top so IF NOT we will swap it for one that does have a top.
			// This is required because we are now moving UP a row and need a clear path to do so.
			EZoneType ZoneType = *SolutionPathData.Find(CurrentPosition);
			if (ZoneType == EZoneType::PZT_LR_Base || ZoneType == EZoneType::PZT_LRB_Base || ZoneType == EZoneType::PZT_Fill_Base)
			{
				if (UpCounter >= 2)
				{
					UpCounter = 0;
					SolutionPathData.Add(CurrentPosition, EZoneType::PZT_LRTB_Base);
				}
				else
				{
					EZoneType TempZoneTypes[2] = { EZoneType::PZT_LRT_Base, EZoneType::PZT_LRTB_Base };
					SolutionPathData.Add(CurrentPosition, TempZoneTypes[FMath::RandRange(0, 1)]);
				}
			}
			// That is now done, so we complete the move up by adjusting out CurrentPosition X value.
			CurrentPosition.X++;

			// We now pick our next random direction
			Direction = FMath::RandRange(1, 5);
			// Pick our next Zone Type to add to the path (It must have a Bottom opening to line up with our Top opening)
			EZoneType TempZoneTypes[2] = { EZoneType::PZT_LRB_Base, EZoneType::PZT_LRTB_Base };
			TempSelectedZoneType = TempZoneTypes[FMath::RandRange(0, 1)];
			// We are done so lets break and join the rest of the code.
		}
		else if (Direction == 2 || Direction == 3)
		{
			// 2 or 3 -> fallthrough is deliberate - Using as a tool for randomization
			// Move Left
			CurrentPosition.Y--;
			Direction = FMath::RandRange(3, 5);
			EZoneType TempZoneTypes[4] = { EZoneType::PZT_LRB_Base, EZoneType::PZT_LRTB_Base, EZoneType::PZT_LR_Base, EZoneType::PZT_LRT_Base };
			TempSelectedZoneType = TempZoneTypes[FMath::RandRange(0, 3)];
		}
		else
		{
			// 4 or 5
			// Move Right
			CurrentPosition.Y++;
			int8 TempDirection[3] = { 1, 2, 5 };
			Direction = TempDirection[FMath::RandRange(0, 2)];
			EZoneType TempZoneTypes[4] = { EZoneType::PZT_LRB_Base, EZoneType::PZT_LRTB_Base, EZoneType::PZT_LR_Base, EZoneType::PZT_LRT_Base };
			TempSelectedZoneType = TempZoneTypes[FMath::RandRange(0, 3)];
		}

		// Check if we are all done, if this is the last zone we will add.
		if (SolutionPathData.Num() >= LengthOfSolutionPath - 1)
		{
			// This is the last Zone
			// Add Special Zone
			SpecialZoneLocations.Add(CurrentPosition, ESpecialZoneType::SZT_Start);

			// Add to Path Map.
			SolutionPathData.Add(CurrentPosition, TempSelectedZoneType);

			UE_LOG(LogTemp, Warning, TEXT("Added the last Zone to the solution path. Length of: %i"), SolutionPathData.Num())
			// No more generating
			bIsGenerating = false;
			break;
		}
		else
		{
			// Add to Path Map.
			SolutionPathData.Add(CurrentPosition, TempSelectedZoneType);
			//On to the next one in the loop.
		}
	}
	// Finished
}