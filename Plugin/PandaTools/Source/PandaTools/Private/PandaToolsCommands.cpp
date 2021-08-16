// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PandaToolsCommands.h"

#define LOCTEXT_NAMESPACE "FPandaToolsModule"

void FPandaToolsCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "PandaTools", "Bring up PandaTools window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
