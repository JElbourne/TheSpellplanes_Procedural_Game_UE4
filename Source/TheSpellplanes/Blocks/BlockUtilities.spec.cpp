#include "BlockUtilities.h"

BEGIN_DEFINE_SPEC(BlockUtilitiesSpec, "SpellPlanes.BlockUtilitiesSpec", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
	//int GridId;
	//static class UBlockUtilities BlockUtilities;
END_DEFINE_SPEC(BlockUtilitiesSpec)

void BlockUtilitiesSpec::Define()
{
	
	Describe("Setting Values into GridId", [this]()
	{
		It("should be able to insert all new ID's with GridId starting at 0", [this]()
		{
			int32 GridId = 0;
			int16 ValueToSet[7] { 1, 1, 12, 945, 33, 7, 1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet);
			TestEqual("GridID == 1129453371", GridId, 1129453371);
		});
		It("should be able to insert all new ID's", [this]()
		{
			int32 GridId = 1000000009;
			int16 ValueToSet[7] = { 1, 1, 12, 945, 33, 7, 1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet);
			TestEqual("GridID == 1129453371", GridId, 1129453371);
		});
		It("should be able to convert to negative ID", [this]()
		{
			int32 GridId = 1000000009;
			int16 ValueToSet1[7] = { -1, 1, 10, 945, 33, 0, 1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet1);
			TestEqual("GridID == -1109453301", GridId, -1109453301);
		});

		It("should be able to bypass setting a value", [this]()
		{
			int32 GridId = 1000880009;
			int16 ValueToSet2[7] = { 1, 1, 10, -1, 33, 0, 1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet2);
			TestEqual("GridID == 1100883301", GridId, 1100883301);
		});
		It("should be able to set Values to stay the same if we wish", [this]()
		{
			int32 GridId = 1072840009;
			int16 ValueToSet3[7] = { 0, -1, -10, -1444, -33, -1, -1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet3);
			TestEqual("GridID == 1072840009", GridId, 1072840009);
		});
		It("should be able to adjust 1st Value, given an array", [this]()
		{
			int32 GridId = 1123456789;
			int16 ValueToSet5a[7] = { 0, -1, -1, -1, -1, -1, 3 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet5a);
			TestEqual("GridID == 1123456783", GridId, 1123456783);
		});
		It("should be able to adjust 2nd Value, given an array", [this]()
		{
			int32 GridId = 1123456789;
			int16 ValueToSet5b[7] = { 0, -1, -1, -1, -1, 2, -1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet5b);
			TestEqual("GridID == 1123456729", GridId, 1123456729);
		});
		It("should be able to adjust 3rd Value, given an array", [this]()
		{
			int32 GridId = 1123456789;
			int16 ValueToSet5c[7] = { 0, -1, -1, -1, 99, -1, -1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet5c);
			TestEqual("GridID == 1123459989", GridId, 1123459989);
		});
		It("should be able to adjust 4th Value, given an array", [this]()
		{
			int32 GridId = 1123456789;
			int16 ValueToSet5d[7] = { 0, -1, -1, 878, -1, -1, -1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet5d);
			TestEqual("GridID == 1128786789", GridId, 1128786789);
		});
		It("should be able to adjust 5th Value, given an array", [this]()
		{
			int32 GridId = 1123456789;
			int16 ValueToSet5e[7] = { 0, -1, 55, -1, -1, -1, -1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet5e);
			TestEqual("GridID == 1553456789", GridId, 1553456789);
		});
		It("should be able to adjust 6th Value, given an array", [this]()
		{
			int32 GridId = 1123456789;
			int16 ValueToSet5f[7] = { 0, 0, -1, -1, -1, -1, -1 };
			UBlockUtilities::SetValuesInGridId(GridId, ValueToSet5f);
			TestEqual("GridID == 123456789", GridId, 123456789);
		});
	});


	Describe("Test Set Value at Index", [this]()
	{
		It("should be able to adjust a Value individually with GridId at 0", [this]()
		{
			int32 GridId = 0;
			UBlockUtilities::SetValueInGridId(GridId, 888, EBlockIdIndex::BII_ItemType);
			TestEqual("GridID == 1008880000", GridId, 8880000);
		});
		It("should be able to adjust +/- Value individually", [this]()
		{
			int32 GridId = 1000000000;
			UBlockUtilities::SetValueInGridId(GridId, -1, EBlockIdIndex::BII_PosNeg);
			TestEqual("GridID == -1000000000", GridId, -1000000000);
		});
		It("should be able to adjust 1st Value individually", [this]()
		{
			int32 GridId = 1100000000;
			UBlockUtilities::SetValueInGridId(GridId, 0, EBlockIdIndex::BII_IsLocked);
			TestEqual("GridID == 100000000", GridId, 100000000);
		});
		It("should be able to adjust 2nd Value individually", [this]()
		{
			int32 GridId = 1000000000;
			UBlockUtilities::SetValueInGridId(GridId, 88, EBlockIdIndex::BII_ItemQuantity);
			TestEqual("GridID == 1880000000", GridId, 1880000000);
		});
		It("should be able to adjust 3rd Value individually", [this]()
		{
			int32 GridId = 1000000000;
			UBlockUtilities::SetValueInGridId(GridId, 888, EBlockIdIndex::BII_ItemType);
			TestEqual("GridID == 1008880000", GridId, 1008880000);
		});
		It("should be able to adjust 4th Value individually", [this]()
		{
			int32 GridId = 1000000000;
			UBlockUtilities::SetValueInGridId(GridId, 88, EBlockIdIndex::BII_BlockType);
			TestEqual("GridID == 1000008800", GridId, 1000008800);
		});
		It("should be able to adjust 5th Value individually", [this]()
		{
			int32 GridId = 1000000000;
			UBlockUtilities::SetValueInGridId(GridId, 8, EBlockIdIndex::BII_ModifierType);
			TestEqual("GridID == 1000000080", GridId, 1000000080);
		});
		It("should be able to adjust 6th Value individually", [this]()
		{
			int32 GridId = 1000000000;
			UBlockUtilities::SetValueInGridId(GridId, 8, EBlockIdIndex::BII_GroundType);
			TestEqual("GridID == 1000000008", GridId, 1000000008);
		});
	});


	Describe("Test Get Values", [this]()
	{
		It("should be able to get the +/- Value, as negative", [this]()
		{
			int32 GridId = -1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_PosNeg);
			TestEqual("Value == -1", val, -1);
		});
		It("should be able to get the +/- Value, as positive", [this]()
		{
			int32 GridId = 1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_PosNeg);
			TestEqual("Value == 1", val, 1);
		});
		It("should be able to get the 1st Value", [this]()
		{
			int32 GridId = 1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_IsLocked);
			TestEqual("Value == 1", val, 1);
		});
		It("should be able to get the 2nd Value", [this]()
		{
			int32 GridId = 1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_ItemQuantity);
			TestEqual("Value == 10", val, 10);
		});
		It("should be able to get the 3rd Value", [this]()
		{
			int32 GridId = 1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_ItemType);
			TestEqual("Value == 88", val, 88);
		});
		It("should be able to get the 4th Value", [this]()
		{
			int32 GridId = 1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_BlockType);
			TestEqual("Value == 33", val, 33);
		});
		It("should be able to get the 5th Value", [this]()
		{
			int32 GridId = 1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_ModifierType);
			TestEqual("Value == 0", val, 0);
		});
		It("should be able to get the 6th Value", [this]()
		{
			int32 GridId = 1100883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_GroundType);
			TestEqual("Value == 1", val, 1);
		});
		It("should get a 0 if position not in ID", [this]()
		{
			int32 GridId = 883301;
			int32 val = UBlockUtilities::GetValueFromGridId(GridId, EBlockIdIndex::BII_IsLocked);
			TestEqual("Value == 0", val, 0);
		});
	});
}