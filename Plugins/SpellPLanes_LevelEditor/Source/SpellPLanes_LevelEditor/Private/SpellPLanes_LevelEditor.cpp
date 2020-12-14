// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpellPLanes_LevelEditor.h"
#include "SpellPLanes_LevelEditorEdMode.h"

#define LOCTEXT_NAMESPACE "FSpellPLanes_LevelEditorModule"

void FSpellPLanes_LevelEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FSpellPLanes_LevelEditorEdMode>(FSpellPLanes_LevelEditorEdMode::EM_SpellPLanes_LevelEditorEdModeId, LOCTEXT("SpellPLanes_LevelEditorEdModeName", "SpellPLanes_LevelEditorEdMode"), FSlateIcon(), true);
}

void FSpellPLanes_LevelEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FSpellPLanes_LevelEditorEdMode::EM_SpellPLanes_LevelEditorEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSpellPLanes_LevelEditorModule, SpellPLanes_LevelEditor)