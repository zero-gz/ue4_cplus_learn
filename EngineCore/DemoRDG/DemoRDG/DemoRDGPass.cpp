#include "DeferredShadingRenderer.h"
#include "SceneTextureParameters.h"
#include "PixelShaderUtils.h"
#include "RenderTargetPool.h"
#include "RenderGraphEvent.h"
#include "CommonRenderResources.h"

#define LOCTEXT_NAMESPACE "RDGTestShader"  

DEFINE_LOG_CATEGORY_STATIC(DemoRDGPassLog, Log, All)

DECLARE_GPU_STAT_NAMED(DemoRDGPass, TEXT("DemoRDGPass"));




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

void FDeferredShadingSceneRenderer::RenderDemoRDGPass(
	FRDGBuilder& GraphBuilder,
	FRDGTextureRef SceneColorTexture
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

			//PassParameters->RenderTargets[0] = FRenderTargetBinding(
			//	SceneColorTexture, ERenderTargetLoadAction::ELoad);

			TShaderMapRef<FDemoRDGPS> PixelShader(View.ShaderMap);
			ClearUnusedGraphResources(PixelShader, PassParameters);

			GraphBuilder.AddPass(
				RDG_EVENT_NAME("DemoRDGPass Render"),
				PassParameters,
				ERDGPassFlags::Raster,
				[PassParameters, &View, VertexShader, PixelShader](FRHICommandList& RHICmdList)
			{
				// 这里要实验一下
				RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.0f, View.ViewRect.Max.X, View.ViewRect.Max.Y, 0.0);

				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				//FPixelShaderUtils::InitFullscreenPipelineState(RHICmdList, View.ShaderMap, PixelShader, /* out */ GraphicsPSOInit);

				//GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_SourceAlpha, BO_Add, BF_Zero, BF_SourceAlpha>::GetRHI();
				//GraphicsPSOInit.BlendState = FScreenPassPipelineState::FDefaultBlendState::GetRHI();

				TShaderMapRef<FScreenVertexShaderVS> VertexShader(GlobalShaderMap);

				//RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;


				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParameters);

				//FPixelShaderUtils::DrawFullscreenTriangle(RHICmdList);
				RHICmdList.SetStreamSource(0, GScreenRectangleVertexBuffer.VertexBufferRHI, 0);

				RHICmdList.DrawIndexedPrimitive(
					GScreenRectangleIndexBuffer.IndexBufferRHI,
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