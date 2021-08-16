// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "PandaToolsStyle.h"

class FPandaToolsCommands : public TCommands<FPandaToolsCommands>
{
public:

	FPandaToolsCommands()
		: TCommands<FPandaToolsCommands>(TEXT("PandaTools"), NSLOCTEXT("Contexts", "PandaTools", "PandaTools Plugin"), NAME_None, FPandaToolsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};