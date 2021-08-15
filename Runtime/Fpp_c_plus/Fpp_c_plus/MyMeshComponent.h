// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "MyMeshComponent.generated.h"

/**
 * 
 */
UCLASS(collapsecategories, hidecategories = (Object, LOD, Physics, Collision, Events), meta = (BlueprintSpawnableComponent), ClassGroup = "Custom", DisplayName = "MyMesh")
class FPP_C_PLUS_API UMyMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
		UMyMeshComponent();

public:
	UFUNCTION(BlueprintCallable, Category = "MyComponent")
	void test_func();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="TestShow")
	FVector GunOffset;

 	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
};
