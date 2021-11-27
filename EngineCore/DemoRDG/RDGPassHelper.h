#pragma once

#include "DeferredShadingRenderer.h"
#include "RenderTargetPool.h"

/** The vertex data used to filter a texture. */
struct FOnlyPositionVertex
{
public:
	FVector4 Position;
};

/** The filter vertex declaration resource type. */
class FVertexOnlyPostionDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FVertexOnlyPostionDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FOnlyPositionVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FOnlyPositionVertex, Position), VET_Float4, 0, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FVertexOnlyPostionDeclaration> GVertexOnlyPostionDeclaration;


/** The vertex data used to filter a texture. */
struct FPostionUVVertex
{
public:
	FVector4 Position;
	FVector2D UV;
};


/** The filter vertex declaration resource type. */
class FVertexPostionUVDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FVertexPostionUVDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FPostionUVVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FPostionUVVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FPostionUVVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FVertexPostionUVDeclaration> GVertexPostionUVDeclaration;

/** The vertex data used to filter a texture. */
struct FPostionUVColorVertex
{
public:
	FVector4 Position;
	FVector2D UV;
	FVector4 Color;
};


/** The filter vertex declaration resource type. */
class FVertexPostionUVColorDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FVertexPostionUVColorDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FPostionUVColorVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FPostionUVColorVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FPostionUVColorVertex, UV), VET_Float2, 1, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FPostionUVColorVertex, Color), VET_Float4, 2, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FVertexPostionUVColorDeclaration> GVertexPostionUVColorDeclaration;

enum DECLARATION_ENUM
{
	ONLY_POSTION,
	POSTION_UV,
	POSTION_UV_COLOR
};

FRHIVertexDeclaration* GetVertexDeclaration(DECLARATION_ENUM InputEnum)
{
	switch (InputEnum)
	{
		case ONLY_POSTION:
			return GVertexOnlyPostionDeclaration.VertexDeclarationRHI;
		case POSTION_UV:
			return GVertexPostionUVDeclaration.VertexDeclarationRHI;
		case POSTION_UV_COLOR:
			return GVertexPostionUVColorDeclaration.VertexDeclarationRHI;
		default:
			return GVertexPostionUVDeclaration.VertexDeclarationRHI;
	}
}

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

FRDGTexture* CreateRenderTargetUtil(FRDGBuilder& GraphBuilder, FIntPoint RTSize, FString RTName, EPixelFormat PXFormat,
		ETextureCreateFlags TCreateFlag, FClearValueBinding ClearBindVar = FClearValueBinding::Black, bool IsPoolRT = false)
{
	FRDGTexture* OutputRDGRT = nullptr;
	if(IsPoolRT)
	{
		FPooledRenderTargetDesc Desc(FPooledRenderTargetDesc::Create2DDesc(RTSize, PXFormat, ClearBindVar, TexCreate_None, TCreateFlag, false));
		TRefCountPtr<IPooledRenderTarget> OutputRT;
		GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, Desc, OutputRT, *RTName, ERenderTargetTransience::NonTransient);
		OutputRDGRT = RegisterExternalOrPassthroughTexture(&GraphBuilder, OutputRT);
	}
	else
	{
		FRDGTextureDesc InputRTDesc = FRDGTextureDesc::Create2D(
			RTSize,
			PXFormat,
			ClearBindVar,
			TCreateFlag
		);
		OutputRDGRT = GraphBuilder.CreateTexture(InputRTDesc, *RTName);
	}

	return OutputRDGRT;
}

enum STATE_ENUM
{
	OpaqueDepthMode,
	TranslucentMode
};

void GetPSOState(STATE_ENUM InputStateEnum,
		FRHIBlendState*& BlendState, FRHIRasterizerState*& RasState, FRHIDepthStencilState*& DSState)
{
	switch (InputStateEnum)
	{
		case OpaqueDepthMode:
			BlendState = TStaticBlendState<>::GetRHI();
			RasState = TStaticRasterizerState<FM_Solid, CM_CW>::GetRHI();
			DSState = TStaticDepthStencilState<true, CF_Always>::GetRHI();
			break;

		case TranslucentMode:
			BlendState = TStaticBlendState<CW_RGBA, BO_Add,BF_SourceAlpha,BF_InverseSourceAlpha,BO_Add,BF_Zero,BF_One>::GetRHI();
			RasState = TStaticRasterizerState<FM_Solid, CM_CW>::GetRHI();
			DSState = TStaticDepthStencilState<true, CF_Always>::GetRHI();
			break;
	}
}

#define INIT_PSO_AND_PARAMETERS(INPUT_STATE_ENUM,VERTEX_DECLARATION_ENUM) 	FGraphicsPipelineStateInitializer GraphicsPSOInit; \
RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit); \
FRHIBlendState* BlendState=nullptr;FRHIRasterizerState* RasState=nullptr;FRHIDepthStencilState* DSState=nullptr;\
GetPSOState(INPUT_STATE_ENUM, BlendState, RasState, DSState);\
GraphicsPSOInit.BlendState =  BlendState; \
GraphicsPSOInit.RasterizerState = RasState;\
GraphicsPSOInit.DepthStencilState = DSState;\
GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclaration(VERTEX_DECLARATION_ENUM);\
GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();\
GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();\
GraphicsPSOInit.PrimitiveType = PT_TriangleList; \
SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit)

// --------------------------------------------------------------------------------------------------
class FPlanePositionUVVertexBuffer : public FVertexBuffer
{
public:
	/** Initialize the RHI for this rendering resource */
	void InitRHI() override
	{
		TResourceArray<FPostionUVVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
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
TGlobalResource<FPlanePositionUVVertexBuffer> GPlanePositionUVVertexBuffer;


class FPlaneIndexBuffer : public FIndexBuffer
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

TGlobalResource<FPlaneIndexBuffer> GPlaneIndexBuffer;