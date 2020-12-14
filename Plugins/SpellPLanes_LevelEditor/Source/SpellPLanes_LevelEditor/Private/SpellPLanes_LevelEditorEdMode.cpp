// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellPLanes_LevelEditorEdMode.h"
#include "SpellPLanes_LevelEditorEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FSpellPLanes_LevelEditorEdMode::EM_SpellPLanes_LevelEditorEdModeId = TEXT("EM_SpellPLanes_LevelEditorEdMode");

FSpellPLanes_LevelEditorEdMode::FSpellPLanes_LevelEditorEdMode()
{

}

FSpellPLanes_LevelEditorEdMode::~FSpellPLanes_LevelEditorEdMode()
{

}

void FSpellPLanes_LevelEditorEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FSpellPLanes_LevelEditorEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FSpellPLanes_LevelEditorEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FSpellPLanes_LevelEditorEdMode::UsesToolkits() const
{
	return true;
}




