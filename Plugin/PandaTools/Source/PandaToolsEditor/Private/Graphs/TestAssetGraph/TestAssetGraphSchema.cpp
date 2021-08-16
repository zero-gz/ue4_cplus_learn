
#include "PandaToolsEditor/Private/Graphs/TestAssetGraph/TestAssetGraphSchema.h"
#include "Rendering/DrawElements.h"

#define LOCTEXT_NAMESPACE "MyTestAssetModule"

UEdNode_MyAssetTestNode::UEdNode_MyAssetTestNode()
{
	bCanRenameNode = true;
}
UEdNode_MyAssetTestNode::~UEdNode_MyAssetTestNode()
{

}

UEdGraphNode* FMyTestAssetSchemaAction::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode /* = true */)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("GenericGraphEditorNewNode", "Generic Graph Editor: New Node"));
		ParentGraph->Modify();
		if (FromPin != nullptr)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		//NodeTemplate->GenericGraphNode->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;
	}

	return ResultNode;
}

void UEdNode_MyAssetTestNode::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, "MultipleNodes", FName(), TEXT("In"));
	CreatePin(EGPD_Output, "MultipleNodes", FName(), TEXT("Out"));
}

FText UEdNode_MyAssetTestNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText(FText::FromString("MyTest"));
}

const FPinConnectionResponse UMyTestAssetGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (!(A && B))
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both pins must be available."));

	if (A->GetOwningNode() == B->GetOwningNode())
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect a node to itself."));

	if (A->Direction == EGPD_Input && B->Direction == EGPD_Input)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an input pin to another input pin."));

	if (A->Direction == EGPD_Output && B->Direction == EGPD_Output)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an output pin to another output pin"));

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

void UMyTestAssetGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const FText AddToolTip = LOCTEXT("NewGenericGraphNodeTooltip", "Add node here");
	FText Desc = FText(FText::FromString("MyTestDesc"));

	TSharedPtr<FMyTestAssetSchemaAction> NewNodeAction(new FMyTestAssetSchemaAction(LOCTEXT("GenericGraphNodeAction", "Generic Graph Node"), Desc, AddToolTip, 0));
	NewNodeAction->NodeTemplate = NewObject<UEdNode_MyAssetTestNode>(ContextMenuBuilder.OwnerOfTemporaries);
	//NewNodeAction->NodeTemplate->GenericGraphNode = NewObject<UGenericGraphNode>(NewNodeAction->NodeTemplate, Graph->NodeType);
	//NewNodeAction->NodeTemplate->GenericGraphNode->Graph = Graph;
	ContextMenuBuilder.AddAction(NewNodeAction);
}

FConnectionDrawingPolicy* UMyTestAssetGraphSchema::CreateConnectionDrawingPolicy(
	int32 InBackLayerID,
	int32 InFrontLayerID,
	float InZoomFactor,
	const FSlateRect& InClippingRect,
	class FSlateWindowElementList& InDrawElements,
	class UEdGraph* InGraphObj
) const
{
	return new FMyTestAssetConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

#undef LOCTEXT_NAMESPACE

//****************************************



FMyTestAssetConnectionDrawingPolicy::FMyTestAssetConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	, GraphObj(InGraphObj)
{
}


void FMyTestAssetConnectionDrawingPolicy::DetermineWiringStyle
(
	UEdGraphPin* OutputPin, 
	UEdGraphPin* InputPin, /*inout*/ 
	FConnectionParams& Params
)
{
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness = 1.5f;

	const bool bDeemphasizeUnhoveredPins = HoveredPins.Num() > 0;
	if (bDeemphasizeUnhoveredPins)
	{
		ApplyHoverDeemphasis(OutputPin, InputPin, /*inout*/ Params.WireThickness, /*inout*/ Params.WireColor);
	}
}

void FMyTestAssetConnectionDrawingPolicy::Draw(
	TMap<TSharedRef<SWidget>, 
	FArrangedWidget>& InPinGeometries, 
	FArrangedChildren& ArrangedNodes
)
{
	// Build an acceleration structure to quickly find geometry for the nodes
	NodeWidgetMap.Empty();
	for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex)
	{
		FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
		TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
		NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
	}

	// Now draw
	FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FMyTestAssetConnectionDrawingPolicy::DrawPreviewConnector(
	const FGeometry& PinGeometry, 
	const FVector2D& StartPoint, 
	const FVector2D& EndPoint, 
	UEdGraphPin* Pin
)
{
	FConnectionParams Params;
	DetermineWiringStyle(Pin, nullptr, /*inout*/ Params);

	if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
	{
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, EndPoint), EndPoint, Params);
	}
	else
	{
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, StartPoint), StartPoint, Params);
	}
}

void FMyTestAssetConnectionDrawingPolicy::DrawSplineWithArrow(
	const FVector2D& StartAnchorPoint, 
	const FVector2D& EndAnchorPoint, 
	const FConnectionParams& Params
)
{
	// bUserFlag1 indicates that we need to reverse the direction of connection (used by debugger)
	const FVector2D& P0 = Params.bUserFlag1 ? EndAnchorPoint : StartAnchorPoint;
	const FVector2D& P1 = Params.bUserFlag1 ? StartAnchorPoint : EndAnchorPoint;

	Internal_DrawLineWithArrow(P0, P1, Params);
}

void FMyTestAssetConnectionDrawingPolicy::Internal_DrawLineWithArrow(
	const FVector2D& StartAnchorPoint, 
	const FVector2D& EndAnchorPoint, 
	const FConnectionParams& Params
)
{
	//@TODO: Should this be scaled by zoom factor?
	const float LineSeparationAmount = 4.5f;

	const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
	const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	// Come up with the final start/end points
	const FVector2D DirectionBias = Normal * LineSeparationAmount;
	const FVector2D LengthBias = ArrowRadius.X * UnitDelta;
	const FVector2D StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
	const FVector2D EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

	// Draw a line/spline
	DrawConnection(WireLayerID, StartPoint, EndPoint, Params);

	// Draw the arrow
	const FVector2D ArrowDrawPos = EndPoint - ArrowRadius;
	const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

	FSlateDrawElement::MakeRotatedBox(
		DrawElementsList,
		ArrowLayerID,
		FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
		ArrowImage,
		ESlateDrawEffect::None,
		AngleInRadians,
		TOptional<FVector2D>(),
		FSlateDrawElement::RelativeToElement,
		Params.WireColor
	);
}

void FMyTestAssetConnectionDrawingPolicy::DrawSplineWithArrow(
	const FGeometry& StartGeom, 
	const FGeometry& EndGeom, 
	const FConnectionParams& Params
)
{
	// Get a reasonable seed point (halfway between the boxes)
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

	// Find the (approximate) closest points between the two boxes
	const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

FVector2D FMyTestAssetConnectionDrawingPolicy::ComputeSplineTangent(const FVector2D& Start, const FVector2D& End) const
{
	const FVector2D Delta = End - Start;
	const FVector2D NormDelta = Delta.GetSafeNormal();

	return NormDelta;
}

