// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTestActor.h"
//#include "Platform.h"
//#include "HAL\Platform.h"

#include "UObject\ConstructorHelpers.h"

DEFINE_LOG_CATEGORY(MyLogCategory);

// Sets default values
AMyTestActor::AMyTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	FString test_fs = TEXT("test_123");
	TEXT("test_123");
	FName test_fn = TEXT("TEst_123");
	
	FText test_ft = FText::FromString(TEXT("TEst_123"));

	FString output_str = "get output %s, %d, %d";
	//FString output_tmp = FString::Format(output_str.GetCharArray(), "123", 1, 2);

	//FText test_ft = TEXT("test 123");// FText::Format("test_123");

	FString test_fn_out;
	test_fn.ToString(test_fn_out);

	UE_LOG(MyLogCategory, Warning, TEXT("get output %s, %s, %d, %d"), *test_fn_out,*(test_ft.ToString()), false, test_fn.IsEqual(FName("test_123")));

	UE_LOG(MyLogCategory, Warning, TEXT("get bGenerateOverlapEventsDuringLevelStreaming:%d"), bGenerateOverlapEventsDuringLevelStreaming);
	bGenerateOverlapEventsDuringLevelStreaming = true;

	//UStaticMeshComponent* com = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("test"));
	//com->SetupAttachment(RootComponent);

	//static ConstructorHelpers::FObjectFinder TankStaticMesh(TEXT("/Game/StarterContent/Shapes/Shape_Torus"));
	//TankMesh = CreateDefaultSubobject(TEXT("TankMesh"));
	//TankMesh->SetStaticMesh(TankStaticMesh.Object);
	//TankMesh->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	//RootComponent = TankMesh;

}

// Called when the game starts or when spawned
void AMyTestActor::BeginPlay()
{
	//Super::BeginPlay();
	//UStaticMeshComponent* com1 = NewObject<UStaticMeshComponent>(this, TEXT("ffff"));
	//AddInstanceComponent(com1);
}

// Called every frame
void AMyTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

