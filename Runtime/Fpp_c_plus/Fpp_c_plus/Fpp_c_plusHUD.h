// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Fpp_c_plusHUD.generated.h"

UCLASS()
class AFpp_c_plusHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFpp_c_plusHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

