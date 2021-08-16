#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "MyTestAsset.generated.h"

UCLASS(BlueprintType)
class PANDATOOLASSETS_API UMyTestAsset : public UObject
{
	GENERATED_BODY()

public:

	UMyTestAsset()
	{

	}

	//This is a test val
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = MyTestAsset)
	FString TestString;


#if WITH_EDITORONLY_DATA

public:

	UPROPERTY()
	class UEdGraph* EdGraph;
#endif
};
