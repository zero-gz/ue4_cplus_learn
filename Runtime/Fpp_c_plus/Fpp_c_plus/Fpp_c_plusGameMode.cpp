// Copyright Epic Games, Inc. All Rights Reserved.

#include "Fpp_c_plusGameMode.h"
#include "Fpp_c_plusHUD.h"
#include "Fpp_c_plusCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFpp_c_plusGameMode::AFpp_c_plusGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFpp_c_plusHUD::StaticClass();
}
