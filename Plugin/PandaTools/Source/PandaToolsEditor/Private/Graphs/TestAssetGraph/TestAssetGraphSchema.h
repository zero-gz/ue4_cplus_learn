
#pragma once
#include "EdGraph/EdGraphNode.h"
#include "ConnectionDrawingPolicy.h"

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "Misc/NotifyHook.h"
#include "IDetailsView.h"

#include "TestAssetGraphSchema.generated.h"

UCLASS(MinimalAPI)
class UEdNode_MyAssetTestNode : public UEdGraphNode
{
	GENERATED_BODY()

public:

	UEdNode_MyAssetTestNode();
	virtual ~UEdNode_MyAssetTestNode();

	virtual void AllocateDefaultPins()override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString TestNodeString;
};

USTRUCT()
struct FMyTestAssetSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY()

public:

	FMyTestAssetSchemaAction() :NodeTemplate(nullptr) {}
	FMyTestAssetSchemaAction(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NodeTemplate(nullptr) {}

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	UEdNode_MyAssetTestNode* NodeTemplate;
};

UCLASS(MinimalAPI)
class UMyTestAssetGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
public:
	
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	/* returns new FConnectionDrawingPolicy from this schema */
	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;

};

class FMyTestAssetConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
protected:

	UEdGraph* GraphObj;
	TMap<UEdGraphNode*, int32> NodeWidgetMap;

public:

	FMyTestAssetConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);

	// FConnectionDrawingPolicy interface 
	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params) override;
	virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& PinGeometries, FArrangedChildren& ArrangedNodes) override;
	virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params) override;
	virtual void DrawSplineWithArrow(const FVector2D& StartPoint, const FVector2D& EndPoint, const FConnectionParams& Params) override;
	virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2D& StartPoint, const FVector2D& EndPoint, UEdGraphPin* Pin) override;
	virtual FVector2D ComputeSplineTangent(const FVector2D& Start, const FVector2D& End) const override;
	// End of FConnectionDrawingPolicy interface

protected:
	void Internal_DrawLineWithArrow(const FVector2D& StartAnchorPoint, const FVector2D& EndAnchorPoint, const FConnectionParams& Params);


};