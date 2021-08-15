// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorTestTool.h"
//#include "TestAssetActionType.h"
//#include "TestAssetFactory.h"

#define LOCTEXT_NAMESPACE "FEditorTestToolModule"

void FEditorTestToolModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	//AssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("New Tools")), LOCTEXT("TestToolAssetCategory"));

}

void FEditorTestToolModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorTestToolModule, EditorTestTool)