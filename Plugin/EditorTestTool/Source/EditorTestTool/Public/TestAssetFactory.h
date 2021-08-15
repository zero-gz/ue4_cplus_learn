#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Factories/Factory.h"
#include "TestAssetFactory.generated.h"


UCLASS()
class EDITORTESTTOOL_API UTestAssetFactory :public UFactory
{
	GENERATED_BODY()
public:
	UTestAssetFactory();

	virtual UObject* FactoryCreateNew(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn
	) override;
};