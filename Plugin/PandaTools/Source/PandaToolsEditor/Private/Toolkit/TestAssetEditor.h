#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Misc/NotifyHook.h"
#include "GraphEditor.h"
#include "IDetailsView.h"

#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "SlateFwd.h"
#include "Components/Viewport.h"
#include "Widgets/SViewport.h"
#include "Editor/AdvancedPreviewScene/Public/AdvancedPreviewScene.h"
#include "Editor/AdvancedPreviewScene/Public/AdvancedPreviewSceneModule.h"
#include "SLevelViewport.h"
#include "SEditorViewport.h"
#include "Editor/UnrealEd/Public/SCommonEditorViewportToolbarBase.h"

class UMyTestAsset;
class STestAssetViewport;

class FMyTestAssetEditorToolKit : public FAssetEditorToolkit, public FNotifyHook
{
public:

	FMyTestAssetEditorToolKit();
	~FMyTestAssetEditorToolKit();

	// Inherited via FAssetEditorToolkit
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void SaveAsset_Execute() override;

	virtual void InitGraphAssetEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UMyTestAsset* InAsset);
	//virtual void BlueprintCompiled();

private:

	static const FName MyAssetDetailsTabId;
	static const FName GraphTabId;
	static const FName MyAssetViewportTabId;

	UMyTestAsset* MyTestAssetObject;

	TSharedPtr<SGraphEditor> EdGraphEditor;
	TSharedPtr<IDetailsView> DetailsWidget;
	TSharedPtr<class STestViewport> TestViewport;

	TSharedPtr<FUICommandList> GraphEditorCommands;

	//FGraphPanelSelectionSet GetSelectedNodes();
	TSharedRef<SDockTab> HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabViewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabGraph(const FSpawnTabArgs& Args);
	void BindToolkitCommands();

	void OnCommandDelete();
	bool CanDeleteNodes();
};


/*
Viewport
*/

class STestViewport : public SEditorViewport, public FGCObject, public ICommonEditorViewportToolbarInfoProvider
{
public:

	SLATE_BEGIN_ARGS(STestViewport) {}
	SLATE_END_ARGS()

	/** The scene for this viewport. */
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;

	void Construct(const FArguments& InArgs);

	STestViewport();
	~STestViewport();

	void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;

	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

	TSharedPtr<class FTestViewportClient> GetViewportClient() { return TypedViewportClient; };
	//Shared ptr to the client
	TSharedPtr<class FTestViewportClient> TypedViewportClient;
};


class FTestViewportClient : public FEditorViewportClient
{
public:

	/** Pointer back to the Editor Viewport */
	TWeakPtr<class STestViewport> ViewportPtr;

	//Constructor and destructor
	FTestViewportClient(const TSharedRef<STestViewport>& InThumbnailViewport, const TSharedRef<FAdvancedPreviewScene>& InPreviewScene);

	/** Stored pointer to the preview scene in which the static mesh is shown */
	FAdvancedPreviewScene* AdvancedPreviewScene;

};