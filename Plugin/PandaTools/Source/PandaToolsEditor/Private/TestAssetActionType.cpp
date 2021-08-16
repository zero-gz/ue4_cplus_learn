
#include "PandaToolsEditor/Public/TestAssetActionType.h"
#include "PandaToolAssets/Public/MyTestAsset.h"
#include "PandaToolsEditor/Private/Toolkit/TestAssetEditor.h"


#define LOCTEXT_NAMESPACE "MyTestAssetActions"

FMyTestAssetActions::FMyTestAssetActions(EAssetTypeCategories::Type InAssetCategory)
	:AssetCategory(InAssetCategory)
{

}

FText FMyTestAssetActions::GetName() const
{
	return NSLOCTEXT("MyTestAssetActions", "MyTestAssetActions", "MyTestAsset");
}

UClass* FMyTestAssetActions::GetSupportedClass() const
{
	return UMyTestAsset::StaticClass();
}

FColor FMyTestAssetActions::GetTypeColor() const
{
	return FColor::Red;
}

uint32 FMyTestAssetActions::GetCategories()
{
	return AssetCategory;
}

bool FMyTestAssetActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return false;
}

void FMyTestAssetActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /* = TSharedPtr<IToolkitHost>() */)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto Object = InObjects.CreateConstIterator(); Object; Object++)
	{
		auto Graph = Cast<UMyTestAsset>(*Object);
		if (Graph != nullptr)
		{
			TSharedRef<FMyTestAssetEditorToolKit>EditorToolkit = MakeShareable(new FMyTestAssetEditorToolKit());
			EditorToolkit->InitGraphAssetEditor(Mode, EditWithinLevelEditor, Graph);
		}
	}
}

#undef LOCTEXT_NAMESPACE