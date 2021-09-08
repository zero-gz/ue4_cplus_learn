#include "DeferredShadingRenderer.h"
#include "SceneTextureParameters.h"
#include "PixelShaderUtils.h"
#include "RenderTargetPool.h"
#include "RenderGraphEvent.h"
#include "CommonRenderResources.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "Shader.h"

#define LOCTEXT_NAMESPACE "RDGTestShader"  

DEFINE_LOG_CATEGORY_STATIC(DemoRDGPassLog, Log, All)
DECLARE_GPU_STAT_NAMED(DemoRDGPass, TEXT("DemoRDGPass"));


BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyColorUniformStruct, RENDERER_API)
SHADER_PARAMETER(FVector4, ColorOne)
SHADER_PARAMETER(FVector4, ColorTwo)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyColorUniformStruct, "FMyColorUniform");

BEGIN_SHADER_PARAMETER_STRUCT(FDemoRDGParameters, )
SHADER_PARAMETER(FVector4, TintColor)
SHADER_PARAMETER_RDG_TEXTURE(Texture2D, RDGTex)
SHADER_PARAMETER_SAMPLER(SamplerState, RDGTexSampler)
END_SHADER_PARAMETER_STRUCT()

class FDemoRDGPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FDemoRDGPS)
	SHADER_USE_PARAMETER_STRUCT(FDemoRDGPS, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FDemoRDGParameters, DemoRDG)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};

IMPLEMENT_SHADER_TYPE(, FDemoRDGPS, TEXT("/Engine/Private/DemoRDGPS.usf"), TEXT("MainPS"), SF_Pixel)

class FDemoRDG_SECPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FDemoRDG_SECPS)
	SHADER_USE_PARAMETER_STRUCT(FDemoRDG_SECPS, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector4, SecondTintColor)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTex)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputTexSampler)
		RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()
};

IMPLEMENT_SHADER_TYPE(, FDemoRDG_SECPS, TEXT("/Engine/Private/DemoRDGPS.usf"), TEXT("SecondPS"), SF_Pixel)


class FMyVertexBuffer : public FVertexBuffer
{
public:
	/** Initialize the RHI for this rendering resource */
	void InitRHI() override
	{
		TResourceArray<FFilterVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(6);

		Vertices[0].Position = FVector4(1, 1, 0, 1);
		Vertices[0].UV = FVector2D(1, 1);

		Vertices[1].Position = FVector4(0, 1, 0, 1);
		Vertices[1].UV = FVector2D(0, 1);

		Vertices[2].Position = FVector4(1, 0, 0, 1);
		Vertices[2].UV = FVector2D(1, 0);

		Vertices[3].Position = FVector4(0, 0, 0, 1);
		Vertices[3].UV = FVector2D(0, 0);

		// Create vertex buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(&Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};
TGlobalResource<FMyVertexBuffer> GMyVertexBuffer;


class FMyIndexBuffer : public FIndexBuffer
{
public:
	/** Initialize the RHI for this rendering resource */
	void InitRHI() override
	{
		// Indices 0 - 5 are used for rendering a quad.
		const uint16 Indices[] = { 0, 1, 2, 2, 1, 3};

		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = UE_ARRAY_COUNT(Indices);
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		// Create index buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(&IndexBuffer);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

TGlobalResource<FMyIndexBuffer> GMyIndexBuffer;

/** The filter vertex declaration resource type. */
class FMyVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FMyVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FFilterVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FFilterVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FFilterVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FMyVertexDeclaration> GMyVertexDeclaration;

class FDemoRDGVS :public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FDemoRDGVS)

	FDemoRDGVS() = default;
	FDemoRDGVS(const ShaderMetaType::CompiledShaderInitializerType & Initializer)
		: FGlobalShader(Initializer)
	{}
};
IMPLEMENT_SHADER_TYPE(, FDemoRDGVS, TEXT("/Engine/Private/DemoRDGPS.usf"), TEXT("MainVS"), SF_Vertex)

// update texture function
bool WriteDataToTexture2D(FRHICommandListImmediate& RHICmdList, FRHITexture2D* tex)
{
	uint32 Stride = 0;
	char* TextureDataPtr = (char*)RHICmdList.LockTexture2D(tex, 0, EResourceLockMode::RLM_WriteOnly, Stride, false);

	for(uint32 Row = 0; Row < tex->GetSizeY(); ++Row)
	{
		uint32 * PixelPtr = (uint32*)TextureDataPtr;
		for(uint32 Col = 0; Col < tex->GetSizeX(); ++Col)
		{
			uint8 r = 255;
			uint8 g = 0;
			uint8 b = 255;
			uint8 a = 255;

			*PixelPtr = r|(g<<8) | (b<<16) | (a<<24);
			PixelPtr++;
		}

		TextureDataPtr += Stride;
	}

	RHICmdList.UnlockTexture2D(tex, 0, false);
	return true;
}

void FDeferredShadingSceneRenderer::RenderDemoRDGPass(
	FRDGBuilder& GraphBuilder,
	FRDGTextureRef SceneColorTexture,
	FRDGTextureRef SceneDepthTexture
)
{
	check(IsInRenderingThread());
	{
		RDG_GPU_STAT_SCOPE(GraphBuilder, DemoRDGPass);
		RDG_EVENT_SCOPE(GraphBuilder, "DemoRDGPass Draw");

		FPooledRenderTargetDesc Desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(800, 600), PF_B8G8R8A8, FClearValueBinding::Green, TexCreate_None, TexCreate_RenderTargetable | TexCreate_ShaderResource, false));
		static TRefCountPtr<IPooledRenderTarget> OutputRT;
		GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, Desc, OutputRT, TEXT("OutputRT"), ERenderTargetTransience::NonTransient);

		FRDGTextureDesc InputRTDesc = FRDGTextureDesc::Create2D(
			FIntPoint(800, 600),
			PF_B8G8R8A8,
			FClearValueBinding::White,
			TexCreate_ShaderResource | TexCreate_RenderTargetable | TexCreate_UAV
		);
		FRDGTexture* InputRT = GraphBuilder.CreateTexture(InputRTDesc, TEXT("InputRT"));
		// mrt second rt
		FRDGTexture* OutputRT2 = GraphBuilder.CreateTexture(InputRTDesc, TEXT("OutputRT2"));
		
		FPooledRenderTargetDesc DescDepth(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(800, 600), PF_DepthStencil, FClearValueBinding(0.2, 0x2), TexCreate_None, TexCreate_DepthStencilTargetable | TexCreate_ShaderResource | TexCreate_InputAttachmentRead, false));
		// Desc.NumSamples = 1;
		// Desc.Flags |= GFastVRamConfig.SceneDepth;
		// Desc.ArraySize = 1;
		// Desc.bIsArray = false;
		//Desc.TargetableFlags |= TexCreate_Memoryless;
		static TRefCountPtr<IPooledRenderTarget> OutputDepth;
		GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, DescDepth, OutputDepth, TEXT("OutputDepth"), ERenderTargetTransience::Transient);


		for (FViewInfo& View : Views)
		{
			FDemoRDGParameters RDGParam;
			RDGParam.TintColor = FVector4(0.0, 0.0, 1.0, 1.0);
			RDGParam.RDGTex = InputRT;
			RDGParam.RDGTexSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();

			FDemoRDGPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FDemoRDGPS::FParameters>();
			PassParameters->DemoRDG = RDGParam;
			FRDGTexture* OutputRDGRT = RegisterExternalOrPassthroughTexture(&GraphBuilder, OutputRT);
			PassParameters->RenderTargets[0] = FRenderTargetBinding(
				OutputRDGRT, ERenderTargetLoadAction::EClear);

			PassParameters->RenderTargets[1] = FRenderTargetBinding(OutputRT2, ERenderTargetLoadAction::EClear);

			FRDGTexture* OutputRDGDepth = RegisterExternalOrPassthroughTexture(&GraphBuilder, OutputDepth);
			PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(
					OutputRDGDepth,
					//ERenderTargetLoadAction::ELoad, 
					//ERenderTargetLoadAction::ELoad,
					ERenderTargetLoadAction::EClear, 
		ERenderTargetLoadAction::EClear, 
					FExclusiveDepthStencil::DepthWrite_StencilWrite);

			TShaderMapRef<FDemoRDGPS> PixelShader(View.ShaderMap);

			TShaderMapRef<FDemoRDGVS> VertexShader(View.ShaderMap);
			ClearUnusedGraphResources(PixelShader, PassParameters);

			// Add Clear RenderTarget Pass, can control LinearColor and view port(for some part).
			AddClearRenderTargetPass(GraphBuilder, PassParameters->DemoRDG.RDGTex, FLinearColor(1.0, 0.0, 1.0),  FIntRect(0, 0, 400, 300));
			
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("DemoRDGPass Render"),
				PassParameters,
				ERDGPassFlags::Raster,
				[PassParameters, &View, VertexShader, PixelShader](FRHICommandListImmediate& RHICmdList)
			{
					// this is RHI LockTexture for update data
					//FRHITexture2D* tex_tmp = static_cast<FRHITexture2D*>(PassParameters->DemoRDG.RDGTex->GetRHI());
					//WriteDataToTexture2D(RHICmdList, tex_tmp);
					
				//RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 0.0);
				RHICmdList.SetViewport(0, 0, 0, 800, 600, 0);

				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.BlendState =  TStaticBlendState<CW_BA, BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_Zero,BF_One>::GetRHI(); //TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_CW>::GetRHI();
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GMyVertexDeclaration.VertexDeclarationRHI;
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParameters);

					FMyColorUniformStruct struct_test;
					struct_test.ColorOne = FVector4(1.0, 1.0, 0.0, 1.0);
					struct_test.ColorTwo = FVector4(0.0, 1.0, 0.0, 1.0);
					SetUniformBufferParameterImmediate(RHICmdList, PixelShader.GetPixelShader(), PixelShader->GetUniformBufferParameter<FMyColorUniformStruct>(), struct_test);

				//FPixelShaderUtils::DrawFullscreenTriangle(RHICmdList);
				RHICmdList.SetStreamSource(0, GMyVertexBuffer.VertexBufferRHI, 0);

				RHICmdList.DrawIndexedPrimitive(
					GMyIndexBuffer.IndexBufferRHI,
					/*BaseVertexIndex=*/ 0,
					/*MinIndex=*/ 0,
					/*NumVertices=*/ 4,
					/*StartIndex=*/ 0,
					/*NumPrimitives=*/ 2,
					/*NumInstances=*/ 1);
			});

			// ------------------------------------------------------------------------------------------

			FDemoRDG_SECPS::FParameters* SecPassParameters = GraphBuilder.AllocParameters<FDemoRDG_SECPS::FParameters>();
			SecPassParameters->SecondTintColor = FVector4(0.0, 1.0, 1.0, 1.0);
			SecPassParameters->InputTex = OutputRDGRT;
			SecPassParameters->InputTexSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
			SecPassParameters->RenderTargets[0] = FRenderTargetBinding(
				SceneColorTexture, ERenderTargetLoadAction::ELoad);

			TShaderMapRef<FDemoRDG_SECPS> SecPixelShader(View.ShaderMap);
			ClearUnusedGraphResources(SecPixelShader, SecPassParameters);

			GraphBuilder.AddPass(
				RDG_EVENT_NAME("DemoRDG SecondPass Render"),
				SecPassParameters,
				ERDGPassFlags::Raster,
				[SecPassParameters, &View, VertexShader, SecPixelShader](FRHICommandList& RHICmdList)
			{
				// 这里要实验一下
				RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 0.0);
				//RHICmdList.SetViewport(0, 0, 0, 800, 600, 0);

				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GMyVertexDeclaration.VertexDeclarationRHI;
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = SecPixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				SetShaderParameters(RHICmdList, SecPixelShader, SecPixelShader.GetPixelShader(), *SecPassParameters);

					FMyColorUniformStruct struct_test;
				struct_test.ColorOne = FVector4(1.0, 1.0, 0.0, 1.0);
				struct_test.ColorTwo = FVector4(0.0, 1.0, 0.0, 1.0);
				SetUniformBufferParameterImmediate(RHICmdList, SecPixelShader.GetPixelShader(), SecPixelShader->GetUniformBufferParameter<FMyColorUniformStruct>(), struct_test);

				//FPixelShaderUtils::DrawFullscreenTriangle(RHICmdList);
				RHICmdList.SetStreamSource(0, GMyVertexBuffer.VertexBufferRHI, 0);

				RHICmdList.DrawIndexedPrimitive(
					GMyIndexBuffer.IndexBufferRHI,
					/*BaseVertexIndex=*/ 0,
					/*MinIndex=*/ 0,
					/*NumVertices=*/ 4,
					/*StartIndex=*/ 0,
					/*NumPrimitives=*/ 2,
					/*NumInstances=*/ 1);
			});
		}
	}
}

#undef LOCTEXT_NAMESPACE