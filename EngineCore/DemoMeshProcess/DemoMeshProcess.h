#pragma once

#include "MeshPassProcessor.h"

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyPassUniformParameters, )
END_GLOBAL_SHADER_PARAMETER_STRUCT()

typedef TUniformBufferRef<FMyPassUniformParameters> FMyPassUniformBufferRef;

class FPrimitiveSceneProxy;
class FScene;
class FStaticMeshBatch;
class FViewInfo;

class FMyPassProcessor : public FMeshPassProcessor
{

public:

	FMyPassProcessor(
		const FScene* Scene,
		const FSceneView* InViewIfDynamicMeshCommand,
		const FMeshPassProcessorRenderState& InPassDrawRenderState,
		const bool InbRespectUseAsOccluderFlag,
		const bool InbEarlyZPassMoveabe,
		FMeshPassDrawListContext* InDrawListContext
	);

	virtual void AddMeshBatch(
		const FMeshBatch& RESTRICT MeshBatch,
		uint64 BatchElementMask,
		const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
		int32 StaticMeshId
	)override final;

private:

	void Process(
		const FMeshBatch& MeshBatch,
		uint64 BatchElementMask,
		int32 StaticMeshId,
		const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
		const FMaterialRenderProxy& RESTRICT MaterialRenderProxy,
		const FMaterial& RESTRICT MaterialResource
	);

	FMeshPassProcessorRenderState PassDrawRenderState;

	const bool bRespectUseAsOccluderFlag;
	const bool bEarlyZPassMoveable;

};