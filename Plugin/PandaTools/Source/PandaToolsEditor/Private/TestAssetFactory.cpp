#include "PandaToolsEditor/Public/TestAssetFactory.h"


#include "PandaToolAssets/Public/MyTestAsset.h"

UTestAssetFactory::UTestAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMyTestAsset::StaticClass();
}

UObject* UTestAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UMyTestAsset* EditorAsset = NewObject<UMyTestAsset>(InParent, InClass, InName, Flags);
	return EditorAsset;
}