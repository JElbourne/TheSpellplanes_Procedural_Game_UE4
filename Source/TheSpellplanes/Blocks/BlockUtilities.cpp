// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockUtilities.h"

int64 UBlockUtilities::s_IdModulus;
int32 UBlockUtilities::s_IdPlaceModifier;

UBlockUtilities::UBlockUtilities()
{
	s_IdModulus = 0;
	s_IdPlaceModifier = 1;
}

void UBlockUtilities::SetModulusAndPlaceModifier(EBlockIdIndex Index)
{
	switch (Index)
	{
	case 6:
	{
		s_IdModulus = 10;
		s_IdPlaceModifier = 1;
	}
	break;
	case 5:
	{
		s_IdModulus = 100;
		s_IdPlaceModifier = 10;
	}
	break;
	case 4:
	{
		s_IdModulus = 10000;
		s_IdPlaceModifier = 100;
	}
	break;
	case 3:
	{
		s_IdModulus = 10000000;
		s_IdPlaceModifier = 10000;
	}
	break;
	case 2:
	{
		s_IdModulus = 1000000000;
		s_IdPlaceModifier = 10000000;
	}
	break;
	case 1:
	{
		s_IdModulus = 10000000000;
		s_IdPlaceModifier = 1000000000;
	}
	break;
	default:
	{
		s_IdModulus = 0;
		s_IdPlaceModifier = 1;
	}
	}
}

int16 UBlockUtilities::GetValueFromGridId(int32 GridId, EBlockIdIndex Index)
{
	SetModulusAndPlaceModifier(Index);
	return (s_IdModulus > 0 && s_IdPlaceModifier > 0) ? (GridId % s_IdModulus) / s_IdPlaceModifier : (GridId > 0) ? 1 : -1;
}

void UBlockUtilities::SetValuesInGridId(int32& GridId, int16 ValuesToSet[7])
{
	// Manual Clamp Values
	ValuesToSet[0] = (ValuesToSet[0] < -1) ? -1 : (1 < ValuesToSet[0]) ? 1 : ValuesToSet[0];
	ValuesToSet[1] = (1 < ValuesToSet[1]) ? 1 : ValuesToSet[1];
	ValuesToSet[2] = (99 < ValuesToSet[2]) ? 99 : ValuesToSet[2];
	ValuesToSet[3] = (999 < ValuesToSet[3]) ? 999 : ValuesToSet[3];
	ValuesToSet[4] = (99 < ValuesToSet[4]) ? 99 : ValuesToSet[4];
	ValuesToSet[5] = (9 < ValuesToSet[5]) ? 9 : ValuesToSet[5];
	ValuesToSet[6] = (9 < ValuesToSet[6]) ? 9 : ValuesToSet[6];

	int32 NumsToAdd = 0;
	for (int8 i = 1; i < 7; i++)
	{
		SetModulusAndPlaceModifier(static_cast<EBlockIdIndex>(i));
		if (ValuesToSet[i] < 0)
		{
			int32 val = GetValueFromGridId(GridId, static_cast<EBlockIdIndex>(i));
			NumsToAdd += (val * s_IdPlaceModifier);
		}
		else
		{
			NumsToAdd += (ValuesToSet[i] * s_IdPlaceModifier);
		}
	}
	if (ValuesToSet[0] != 0) NumsToAdd *= ValuesToSet[0];

	GridId = NumsToAdd;
}


void UBlockUtilities::SetValueInGridId(int32& GridIdIn, int16 IdValue, EBlockIdIndex Index)
{
	int16 ValuesToSet[7] = { 0, -1, -1, -1, -1, -1, -1 };
	ValuesToSet[static_cast<int>(Index)] = IdValue;
	SetValuesInGridId(GridIdIn, ValuesToSet);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/**
* The below test code has been re-written into a Automated Test SPEC file called BlockUtilities.spec.cpp
* It uses the in-engine Test Front end Plugin.
* Code will remain here for now.
*/
void UBlockUtilities::TestBlockUtilities()
{
	int32 GridId;

	UE_LOG(LogTemp, Warning, TEXT("BlockUtilities: Starting the Tests:"));
	GridId = 1000000009;
	int16
		ValueToSet[7] = { 1, 1, 12, 945, 33, 7, 1 };
	SetValuesInGridId(GridId, ValueToSet);
	PrintTestResult((GridId == 1129453371));
	// Convert To negative Test
	GridId = 1000000009;
	int16
 ValueToSet1[7] = { -1, 1, 10, 945, 33, 0, 1 };
	SetValuesInGridId(GridId, ValueToSet1);
	PrintTestResult((GridId == -1109453301));
	// Test to make sure we can bypass setting a value.
	GridId = 1000880009;
	int16
 ValueToSet2[7] = { 1, 1, 10, -1, 33, 0, 1 };
	SetValuesInGridId(GridId, ValueToSet2);
	PrintTestResult((GridId == 1100883301));
	// Test to make sure we set Values to stay the same if we wish.
	GridId = 1072840009;
	int16
 ValueToSet3[7] = { 0, -1, -10, -1444, -33, -1, -1 };
	SetValuesInGridId(GridId, ValueToSet3);
	PrintTestResult((GridId == 1072840009));

	// Test to make sure we can adjust each Value individually.
	GridId = 1123456789;
	int16
 ValueToSet5a[7] = { 0, -1, -1, -1, -1, -1, 3 };
	SetValuesInGridId(GridId, ValueToSet5a);
	PrintTestResult((GridId == 1123456783));
	
	GridId = 1123456789;
	int16
 ValueToSet5b[7] = { 0, -1, -1, -1, -1, 2, -1 };
	SetValuesInGridId(GridId, ValueToSet5b);
	PrintTestResult((GridId == 1123456729));
	
	GridId = 1123456789;
	int16
 ValueToSet5c[7] = { 0, -1, -1, -1, 99, -1, -1 };
	SetValuesInGridId(GridId, ValueToSet5c);
	PrintTestResult((GridId == 1123459989));
	
	GridId = 1123456789;
	int16
 ValueToSet5d[7] = { 0, -1, -1, 878, -1, -1, -1 };
	SetValuesInGridId(GridId, ValueToSet5d);
	PrintTestResult((GridId == 1128786789));
	
	GridId = 1123456789;
	int16
 ValueToSet5e[7] = { 0, -1, 55, -1, -1, -1, -1 };
	SetValuesInGridId(GridId, ValueToSet5e);
	PrintTestResult((GridId == 1553456789));
	
	GridId = 1123456789;
	int16
 ValueToSet5f[7] = { 0, 0, -1, -1, -1, -1, -1 };
	SetValuesInGridId(GridId, ValueToSet5f);
	PrintTestResult((GridId == 123456789));


	// Test Get Modulus & Modifiers
	SetModulusAndPlaceModifier(EBlockIdIndex::BII_PosNeg);
	PrintTestResult(((s_IdModulus == 0) && (s_IdPlaceModifier == 1)));
	
	SetModulusAndPlaceModifier(EBlockIdIndex::BII_IsLocked);
	PrintTestResult(((s_IdModulus == 10000000000) && (s_IdPlaceModifier == 1000000000)));
	
	SetModulusAndPlaceModifier(EBlockIdIndex::BII_ItemQuantity);
	PrintTestResult(((s_IdModulus == 1000000000) && (s_IdPlaceModifier == 10000000)));
	
	SetModulusAndPlaceModifier(EBlockIdIndex::BII_ItemType);
	PrintTestResult(((s_IdModulus == 10000000) && (s_IdPlaceModifier == 10000)));
	
	SetModulusAndPlaceModifier(EBlockIdIndex::BII_BlockType);
	PrintTestResult(((s_IdModulus == 10000) && (s_IdPlaceModifier == 100)));
	
	SetModulusAndPlaceModifier(EBlockIdIndex::BII_ModifierType);
	PrintTestResult(((s_IdModulus == 100) && (s_IdPlaceModifier == 10)));
	
	SetModulusAndPlaceModifier(EBlockIdIndex::BII_GroundType);
	PrintTestResult(((s_IdModulus == 10) && (s_IdPlaceModifier == 1)));


	// Test Get Values
	int32 val;
	GridId = 1100883301;
	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_PosNeg);
	PrintTestResult((val == 1));
	std::cout << "Test 13 Pass: " << (val == 1) << "!\n";
	
	GridId = -1100883301;
	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_PosNeg);
	PrintTestResult((val == -1));
	std::cout << "Test 14 Pass: " << (val == -1) << "!\n";
	
	GridId = 1100883301;
	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_IsLocked);
	PrintTestResult((val == 1));

	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_ItemQuantity);
	PrintTestResult((val == 10));

	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_ItemType);
	PrintTestResult((val == 88));

	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_BlockType);
	PrintTestResult((val == 33));

	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_ModifierType);
	PrintTestResult((val == 0));

	val = GetValueFromGridId(GridId, EBlockIdIndex::BII_GroundType);
	PrintTestResult((val == 1));
}

void UBlockUtilities::PrintTestResult(bool bIsSuccess)
{
	static int64 s_TestNum;
	s_TestNum++;

	if (bIsSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlockUtilities:: Test %i: PASS"), s_TestNum);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BlockUtilities:: Test %i: FAIL"), s_TestNum);
	}
}
