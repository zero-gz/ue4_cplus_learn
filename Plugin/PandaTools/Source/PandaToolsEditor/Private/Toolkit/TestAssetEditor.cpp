

#include "TestAssetEditor.h"
#include "PropertyEditorModule.h"
#include "PandaToolAssets/Public/MyTestAsset.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "GraphEditorActions.h"
#include "Slate.h"

#include "PandaToolsEditor/Private/Graphs/TestAssetGraph/TestAssetGraph.h"
#include "PandaToolsEditor/Private/Graphs/TestAssetGraph/TestAssetGraphSchema.h"

#include "AssetEditorModeManager.h"

#define LOCTEXT_NAMESPACE "MyTestAssetEditorToolkit"

const FName FMyTestAssetEditorToolKit::MyAssetDetailsTabId(TEXT("TestAssetGraphEditorToolkitDetailsTabId"));
const FName FMyTestAssetEditorToolKit::GraphTabId(TEXT("TestAssetGraphEditorToolkitGraphTabId"));

const FName FMyTestAssetEditorToolKit::MyAssetViewportTabId(TEXT("TestAssetGraphEditorToolkitViewportTabId"));

FMyTestAssetEditorToolKit::FMyTestAssetEditorToolKit()
{
	//GEditor->OnBlueprintCompiled().AddRaw(this, &FMyTestAssetEditorToolKit::BlueprintCompiled);
}

FMyTestAssetEditorToolKit::~FMyTestAssetEditorToolKit()
{
	//GEditor->OnBlueprintCompiled().RemoveAll(this);
}

FLinearColor FMyTestAssetEditorToolKit::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FName FMyTestAssetEditorToolKit::GetToolkitFName() const
{
	return FName("TestAssetEditor");
}

FText FMyTestAssetEditorToolKit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "TestAsset Editor");
}

FString FMyTestAssetEditorToolKit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Graph").ToString();
}

TSharedRef<SDockTab> FMyTestAssetEditorToolKit::HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args)
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = true;
	DetailsViewArgs.bCustomNameAreaLocation = true;
	DetailsViewArgs.bLockable = false;
	//DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	DetailsWidget = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsWidget->SetObject(MyTestAssetObject);

	return SNew(SDockTab).TabRole(ETabRole::PanelTab)[DetailsWidget.ToSharedRef()];
}

TSharedRef<SDockTab> FMyTestAssetEditorToolKit::HandleTabManagerSpawnTabViewport(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		.Label(LOCTEXT("Viewport_TabTitle", "Viewport"))
		[
			TestViewport.ToSharedRef()
		];

	return SpawnedTab;
}

TSharedRef<SDockTab> FMyTestAssetEditorToolKit::HandleTabManagerSpawnTabGraph(const FSpawnTabArgs& Args)
{
	EdGraphEditor = SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.GraphToEdit(MyTestAssetObject->EdGraph);

	return SNew(SDockTab).TabRole(PanelTab)[EdGraphEditor.ToSharedRef()];
}

void FMyTestAssetEditorToolKit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("TestAssetToolkitWorkspaceMenu", "Test Asste Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	TabManager->RegisterTabSpawner(FMyTestAssetEditorToolKit::MyAssetDetailsTabId, FOnSpawnTab::CreateSP(this, &FMyTestAssetEditorToolKit::HandleTabManagerSpawnTabDetails))
		.SetDisplayName(LOCTEXT("MyAssetDetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef); 

	TabManager->RegisterTabSpawner(FMyTestAssetEditorToolKit::GraphTabId, FOnSpawnTab::CreateSP(this, &FMyTestAssetEditorToolKit::HandleTabManagerSpawnTabGraph))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph Editor"))
		.SetGroup(WorkspaceMenuCategoryRef); 

	TabManager->RegisterTabSpawner(FMyTestAssetEditorToolKit::MyAssetViewportTabId, FOnSpawnTab::CreateSP(this, &FMyTestAssetEditorToolKit::HandleTabManagerSpawnTabViewport))
		.SetDisplayName(LOCTEXT("MyAssetViewportTab", "My Asset Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef);

}

void FMyTestAssetEditorToolKit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	TabManager->UnregisterTabSpawner(MyAssetDetailsTabId);
}

void FMyTestAssetEditorToolKit::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();
}

void FMyTestAssetEditorToolKit::InitGraphAssetEditor(
	const EToolkitMode::Type InMode, 
	const TSharedPtr<class IToolkitHost>& InToolkitHost, 
	UMyTestAsset* InAsset
)
{

	MyTestAssetObject = InAsset;

	if (MyTestAssetObject->EdGraph == nullptr)
	{
		MyTestAssetObject->EdGraph = CastChecked<UTestAssteGraph>(FBlueprintEditorUtils::CreateNewGraph(MyTestAssetObject, NAME_None, UTestAssteGraph::StaticClass(), UMyTestAssetGraphSchema::StaticClass()));
		MyTestAssetObject->EdGraph->bAllowDeletion = false;
	}

	TestViewport = SNew(STestViewport);

	FGraphEditorCommands::Register();
	BindToolkitCommands();

	TSharedRef<FTabManager::FLayout>Layout = FTabManager::NewLayout("Layout_MytestAsset_V1")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->AddTab(FMyTestAssetEditorToolKit::GetToolbarTabId(), ETabState::OpenedTab)
			->SetHideTabWell(true)
			->SetSizeCoefficient(0.1f)
		)
		->Split
		(
			FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)
			->SetSizeCoefficient(0.9f)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(FMyTestAssetEditorToolKit::MyAssetViewportTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.5f)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(FMyTestAssetEditorToolKit::GraphTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.4f)
				->SetHideTabWell(true)
				->AddTab(FMyTestAssetEditorToolKit::MyAssetDetailsTabId, ETabState::OpenedTab)
			)
		)
	);

	FAssetEditorToolkit::InitAssetEditor(InMode, InToolkitHost, FName("TestAssetEditorIdentifier"), Layout, true, true, InAsset);
}

void FMyTestAssetEditorToolKit::BindToolkitCommands()
{
	if (!GraphEditorCommands.IsValid())
	{
		GraphEditorCommands = MakeShareable(new FUICommandList());

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FMyTestAssetEditorToolKit::OnCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FMyTestAssetEditorToolKit::CanDeleteNodes)
		);
	}
}

void FMyTestAssetEditorToolKit::OnCommandDelete()
{
	EdGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = EdGraphEditor->GetSelectedNodes();
	EdGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(SelectedNodes); It; ++It)
	{
		if (UEdGraphNode * Node = Cast<UEdGraphNode>(*It))
		{
			Node->Modify();
			Node->DestroyNode();
		}
	}
}

bool FMyTestAssetEditorToolKit::CanDeleteNodes()
{
	return true;
}

#undef LOCTEXT_NAMESPACE


/**
 * Viewport
 */

 //Just create the advnaced preview scene and initiate components
STestViewport::STestViewport() : PreviewScene(MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues())))
{

}

STestViewport::~STestViewport()
{
	if (TypedViewportClient.IsValid())
	{
		TypedViewportClient->Viewport = NULL;
	}
}

void STestViewport::AddReferencedObjects(FReferenceCollector& Collector)
{

}
TSharedRef<class SEditorViewport> STestViewport::GetViewportWidget()
{
	return SharedThis(this);
}
TSharedPtr<FExtender> STestViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}
void STestViewport::OnFloatingButtonClicked()
{
	// Nothing
}

void STestViewport::Construct(const FArguments& InArgs)
{
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> STestViewport::MakeEditorViewportClient()
{
	TypedViewportClient = MakeShareable(new FTestViewportClient(SharedThis(this), PreviewScene.ToSharedRef()));
	return TypedViewportClient.ToSharedRef();
}

/**
 * Client
 */


FTestViewportClient::FTestViewportClient(const TSharedRef<STestViewport>& InThumbnailViewport, const TSharedRef<FAdvancedPreviewScene>& InPreviewScene)
	: FEditorViewportClient(nullptr, &InPreviewScene.Get(), StaticCastSharedRef<SEditorViewport>(InThumbnailViewport))
	, ViewportPtr(InThumbnailViewport)
{
	AdvancedPreviewScene = static_cast<FAdvancedPreviewScene*>(PreviewScene);

	SetRealtime(true);

	// Hide grid, we don't need this.
	DrawHelper.bDrawGrid = false;
	DrawHelper.bDrawPivot = false;
	DrawHelper.AxesLineThickness = 5;
	DrawHelper.PivotSize = 5;

	//Initiate view
	SetViewLocation(FVector(75, 75, 75));
	SetViewRotation(FVector(-75, -75, -75).Rotation());

	EngineShowFlags.SetScreenPercentage(true);

	// Set the Default type to Ortho and the XZ Plane
	ELevelViewportType NewViewportType = LVT_Perspective;
	SetViewportType(NewViewportType);

	// View Modes in Persp and Ortho
	SetViewModes(VMI_Lit, VMI_Lit);
}