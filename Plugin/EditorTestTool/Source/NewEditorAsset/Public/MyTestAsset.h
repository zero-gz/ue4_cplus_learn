// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "MyTestAsset.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(AssetLogCategory, Log, All);

UCLASS()
class NEWEDITORASSET_API UMyTestAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		int number;

	UPROPERTY(EditAnywhere)
		bool flag;

	// Sets default values for this actor's properties
	UMyTestAsset();
};