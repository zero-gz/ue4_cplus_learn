// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PandaTools.h"
#include "PandaToolsStyle.h"
#include "PandaToolsCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName PandaToolsTabName("PandaTools");

#define LOCTEXT_NAMESPACE "FPandaToolsModule"

void FPandaToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FPandaToolsStyle::Initialize();
	FPandaToolsStyle::ReloadTextures();

	FPandaToolsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPandaToolsCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FPandaToolsModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FPandaToolsModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FPandaToolsModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PandaToolsTabName, FOnSpawnTab::CreateRaw(this, &FPandaToolsModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FPandaToolsTabTitle", "PandaTools"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FPandaToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FPandaToolsStyle::Shutdown();

	FPandaToolsCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PandaToolsTabName);
}

TSharedRef<SDockTab> FPandaToolsModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SBox)
				.HeightOverride(200.0f)
				.WidthOverride(200.0f)
				[
					SNew(SButton)
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SBox)
				.HeightOverride(200.0f)
				.WidthOverride(200.0f)
				[
					SNew(SButton)
				]
			]
		];
}

void FPandaToolsModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(PandaToolsTabName);
}

void FPandaToolsModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FPandaToolsCommands::Get().OpenPluginWindow);
}

void FPandaToolsModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FPandaToolsCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPandaToolsModule, PandaTools)