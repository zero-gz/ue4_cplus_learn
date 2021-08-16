// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"

#include "Styling/SlateStyle.h"

class FPandaToolsEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	TSharedPtr<FSlateStyleSet> MaterialDataEffectIconSyleSet;

	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);
	EAssetTypeCategories::Type AssetCategoryBit;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;
};
