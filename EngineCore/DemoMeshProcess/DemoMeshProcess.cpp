#include "DemoMeshProcess.h"

#include "ScenePrivate.h"
#include "SceneRendering.h"

#include "Shader.h"
#include "GlobalShader.h"
#include "MeshMaterialShader.h"
#include "MeshPassProcessor.h"
#include "MeshPassProcessor.inl"
#include "DeferredShadingRenderer.h"

bool IsSupportedVertexFactoryType(const FVertexFactoryType* VertexFactoryType);

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyPassUniformParameters, "MyPassUniformParametersParams");

class FMyPassVS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FMyPassVS, MeshMaterial);

	FMyPassVS() {}
public:

	FMyPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		PassUniformBuffer.Bind(Initializer.ParameterMap, FMyPassUniformParameters::StaticStructMetadata.GetShaderVariableName());
	}

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		//return Parameters.VertexFactoryType->SupportsPositionOnly() && IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	void GetShaderBindings(const FScene* Scene, ERHIFeatureLevel::Type FeatureLevel, const FPrimitiveSceneProxy* PrimitiveSceneProxy, const FMaterialRenderProxy& MaterialRenderProxy, const FMaterial& Material, const FMeshPassProcessorRenderState& DrawRenderState, const FMeshMaterialShaderElementData& ShaderElementData, FMeshDrawSingleShaderBindings& ShaderBindings)
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
	}
};


class FMyPassPS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FMyPassPS, MeshMaterial);

public:
	FMyPassPS() { }
	FMyPassPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FMeshMaterialShader(Initializer)
	{
		PassUniformBuffer.Bind(Initializer.ParameterMap, FMyPassUniformParameters::StaticStructMetadata.GetShaderVariableName());
	}

	static bool ShouldCompilePermutation(const FMeshMaterialShaderPermutationParameters& Parameters)
	{
		//return Parameters.VertexFactoryType->SupportsPositionOnly() && IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	void GetShaderBindings(const FScene* Scene, ERHIFeatureLevel::Type FeatureLevel, const FPrimitiveSceneProxy* PrimitiveSceneProxy, const FMaterialRenderProxy& MaterialRenderProxy, const FMaterial& Material, const FMeshPassProcessorRenderState& DrawRenderState, const FMeshMaterialShaderElementData& ShaderElementData, FMeshDrawSingleShaderBindings& ShaderBindings)
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
	}
};

IMPLEMENT_MATERIAL_SHADER_TYPE(, FMyPassVS, TEXT("/Engine/Private/MyPass.usf"), TEXT("Main"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FMyPassPS, TEXT("/Engine/Private/MyPass.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_SHADERPIPELINE_TYPE_VSPS(MyShaderPipeline, FMyPassVS, FMyPassPS, true);

FMyPassProcessor::FMyPassProcessor(
	const FScene* Scene,
	const FSceneView* InViewIfDynamicMeshCommand,
	const FMeshPassProcessorRenderState& InPassDrawRenderState,
	const bool InbRespectUseAsOccluderFlag,
	const bool InbEarlyZPassMoveabe,
	FMeshPassDrawListContext* InDrawListContext
)
	:FMeshPassProcessor(
		Scene
		, Scene->GetFeatureLevel()
		, InViewIfDynamicMeshCommand
		, InDrawListContext
	)
	, bRespectUseAsOccluderFlag(InbRespectUseAsOccluderFlag)
	, bEarlyZPassMoveable(InbEarlyZPassMoveabe)
{
	PassDrawRenderState = InPassDrawRenderState;
	PassDrawRenderState.SetViewUniformBuffer(Scene->UniformBuffers.ViewUniformBuffer);
	PassDrawRenderState.SetInstancedViewUniformBuffer(Scene->UniformBuffers.InstancedViewUniformBuffer);
}

void FMyPassProcessor::AddMeshBatch(const FMeshBatch& RESTRICT MeshBatch, uint64 BatchElementMask, const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy, int32 StaticMeshId)
{
	const FMaterialRenderProxy* FallbackMaterialRenderProxyPtr = nullptr;
	const FMaterial& Material = MeshBatch.MaterialRenderProxy->GetMaterialWithFallback(Scene->GetFeatureLevel(), FallbackMaterialRenderProxyPtr);
	const FMaterialRenderProxy& MaterialRenderProxy = FallbackMaterialRenderProxyPtr ? *FallbackMaterialRenderProxyPtr : *MeshBatch.MaterialRenderProxy;

	const EBlendMode BlendMode = Material.GetBlendMode();
	const bool bIsTranslucent = IsTranslucentBlendMode(BlendMode);

	if (
		!bIsTranslucent
		&& (!PrimitiveSceneProxy || PrimitiveSceneProxy->ShouldRenderInMainPass())
		&& ShouldIncludeDomainInMeshPass(Material.GetMaterialDomain())
		)
	{
		if (BlendMode == BLEND_Opaque)
		{
			//const FMaterialRenderProxy& DefualtProxy = *UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();
			//const FMaterial& DefaltMaterial = *DefualtProxy.GetMaterial(Scene->GetFeatureLevel());

			Process(
				MeshBatch,
				BatchElementMask,
				StaticMeshId,
				PrimitiveSceneProxy,
				MaterialRenderProxy,
				Material
			);
		}
	}

}

static FORCEINLINE bool UseShaderPipelines(ERHIFeatureLevel::Type InFeatureLevel)
{
	static const auto* CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.ShaderPipelines"));
	return RHISupportsShaderPipelines(GShaderPlatformForFeatureLevel[InFeatureLevel]) && CVar && CVar->GetValueOnAnyThread() != 0;
}

void GetMyPassShaders(
	const FMaterial& material,
	FVertexFactoryType* VertexFactoryType,
	ERHIFeatureLevel::Type FeatureLevel,
	TShaderRef<FBaseHS>& HullShader,
	TShaderRef<FBaseDS>& DomainShader,
	TShaderRef<FMyPassVS>& VertexShader,
	TShaderRef<FMyPassPS>& PixleShader,
	FShaderPipelineRef& ShaderPipeline
)
{
	ShaderPipeline = UseShaderPipelines(FeatureLevel) ? material.GetShaderPipeline(&MyShaderPipeline, VertexFactoryType) : FShaderPipelineRef();

	VertexShader = ShaderPipeline.IsValid() ? ShaderPipeline.GetShader<FMyPassVS>() : material.GetShader<FMyPassVS>(VertexFactoryType);
	PixleShader = ShaderPipeline.IsValid() ? ShaderPipeline.GetShader<FMyPassPS>() : material.GetShader<FMyPassPS>(VertexFactoryType);
}

void FMyPassProcessor::Process(
	const FMeshBatch& MeshBatch,
	uint64 BatchElementMask,
	int32 StaticMeshId,
	const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
	const FMaterialRenderProxy& RESTRICT MaterialRenderProxy,
	const FMaterial& RESTRICT MaterialResource
)
{
	const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;

	TMeshProcessorShaders<
		FMyPassVS,
		FBaseHS,
		FBaseDS,
		FMyPassPS
	>MyPassShaders;

	FShaderPipelineRef ShaderPipeline;

	GetMyPassShaders(
		MaterialResource,
		VertexFactory->GetType(),
		FeatureLevel,
		MyPassShaders.HullShader,
		MyPassShaders.DomainShader,
		MyPassShaders.VertexShader,
		MyPassShaders.PixelShader,
		ShaderPipeline
	);

	const FMeshDrawingPolicyOverrideSettings OverrideSettings = ComputeMeshOverrideSettings(MeshBatch);
	const ERasterizerFillMode MeshFillMode = ComputeMeshFillMode(MeshBatch, MaterialResource, OverrideSettings);
	ERasterizerCullMode MeshCullMode = ComputeMeshCullMode(MeshBatch, MaterialResource, OverrideSettings);

	switch (MeshCullMode)
	{
	case CM_CCW:
		MeshCullMode = CM_CW;
		break;
	case CM_CW:
		MeshCullMode = CM_CCW;
		break;
	default:
		break;
	}

	FMeshMaterialShaderElementData ShaderElementData;
	ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, StaticMeshId, true);

	const FMeshDrawCommandSortKey SortKey = CalculateMeshStaticSortKey(MyPassShaders.VertexShader, MyPassShaders.PixelShader);

	BuildMeshDrawCommands(
		MeshBatch,
		BatchElementMask,
		PrimitiveSceneProxy,
		MaterialRenderProxy,
		MaterialResource,
		PassDrawRenderState,
		MyPassShaders,
		MeshFillMode,
		MeshCullMode,
		SortKey,
		//EMeshPassFeatures::PositionOnly,
		EMeshPassFeatures::Default,
		ShaderElementData
	);

}

FMeshPassProcessor* CreateMyPassProcessor(
	const FScene* Scene,
	const FSceneView* InViewIfDynamicMeshCommand,
	FMeshPassDrawListContext* InDrawListContext
)
{
	FMeshPassProcessorRenderState MyPassState;
	MyPassState.SetInstancedViewUniformBuffer(Scene->UniformBuffers.InstancedViewUniformBuffer);
	MyPassState.SetBlendState(TStaticBlendStateWriteMask<CW_RGBA>::GetRHI());
	MyPassState.SetDepthStencilAccess(Scene->DefaultBasePassDepthStencilAccess);
	MyPassState.SetDepthStencilState(TStaticDepthStencilState<true, CF_DepthNearOrEqual>::GetRHI());

	const FBasePassMeshProcessor::EFlags Flags = FBasePassMeshProcessor::EFlags::CanUseDepthStencil;

	return new(FMemStack::Get()) FMyPassProcessor(
		Scene,
		InViewIfDynamicMeshCommand,
		MyPassState,
		true,
		Scene->bEarlyZPassMovable,
		InDrawListContext
	);
}

FRegisterPassProcessorCreateFunction RegisterMyPass(
	&CreateMyPassProcessor,
	EShadingPath::Deferred,
	EMeshPass::MyPass,
	EMeshPassFlags::CachedMeshCommands | EMeshPassFlags::MainView
);

void FDeferredShadingSceneRenderer::RenderMyMeshProcessPass(FRDGBuilder& GraphBuilder)
{
	//RenderEditorPrimitives(GraphBuilder, PassParameters, View, DrawRenderState);
	//if (View.ParallelMeshDrawCommandPasses[EMeshPass::MyPass].HasAnyDraw())
	//{
	//	GraphBuilder.AddPass(
	//		RDG_EVENT_NAME("ToonOutline"),
	//		PassParameters,
	//		ERDGPassFlags::Raster,
	//		[this, &View, PassParameters](FRHICommandList& RHICmdList)
	//	{
	//		View.ParallelMeshDrawCommandPasses[EMeshPass::MyPass].DispatchDraw(nullptr, RHICmdList);
	//	});
	//}
}