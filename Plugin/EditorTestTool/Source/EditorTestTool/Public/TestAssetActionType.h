#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "AssetTypeActions_Base.h"

class FMyTestAssetActions :public FAssetTypeActions_Base
{
public:
	FMyTestAssetActions(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObject, TSharedPtr<class IToolkitHost> EditorWithinLevel);

private:
	EAssetTypeCategories::Type AssetCategory;
};