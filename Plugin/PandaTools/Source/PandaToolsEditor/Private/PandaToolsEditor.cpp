// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PandaToolsEditor.h"
#include "Modules/ModuleManager.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#include "PandaToolsEditor/Public/TestAssetActionType.h"

#define LOCTEXT_NAMESPACE "FPandaToolsEditorModule"

void FPandaToolsEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	//ArtTools assets category
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("MyDefineAsset")), LOCTEXT("MyDefineAssetCategory", "MyDefineAsset"));
	//PandaTools PandaToolsAssetCategory
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FMyTestAssetActions(AssetCategoryBit)));
	//->add here

	//find Asset icon logic
	MaterialDataEffectIconSyleSet = MakeShareable(new FSlateStyleSet("MaterialEffectAssetStyle"));
	FString MaterialEffectAssetIconAssetDir = IPluginManager::Get().FindPlugin("PandaTools")->GetBaseDir();
	MaterialDataEffectIconSyleSet->SetContentRoot(MaterialEffectAssetIconAssetDir);
	FSlateImageBrush* ThumbnailBrush = new FSlateImageBrush
	(
		//add in future
		MaterialDataEffectIconSyleSet->RootToContentDir(TEXT("Resources/MaterialEffectAssetIcon") , TEXT(".png"))
		, FVector2D(128.0f, 128.0f)
	);
	if (ThumbnailBrush != nullptr)
	{
		MaterialDataEffectIconSyleSet->Set("ClassThumbnail.MaterialEffectDataDriver", ThumbnailBrush);
		FSlateStyleRegistry::RegisterSlateStyle(*MaterialDataEffectIconSyleSet);
	}
}

void FPandaToolsEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//delete our Action
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	FSlateStyleRegistry::UnRegisterSlateStyle(MaterialDataEffectIconSyleSet->GetStyleSetName());
}

void FPandaToolsEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPandaToolsEditorModule, PandaToolsEditor)