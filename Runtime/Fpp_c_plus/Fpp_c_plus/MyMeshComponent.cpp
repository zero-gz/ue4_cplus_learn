// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMyComponent, Warning, All);

class FMyVertexBuffer :public FVertexBuffer
{
	public:
		TArray<FDynamicMeshVertex> Vertices;

		virtual void InitRHI() override
		{
			FRHIResourceCreateInfo CreateInfo;
			void* VertexBufferData = nullptr;
			VertexBufferRHI = RHICreateAndLockVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex), BUF_Static, CreateInfo, VertexBufferData);
			FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FDynamicMeshVertex));
			RHIUnlockVertexBuffer(VertexBufferRHI);
		}
};

class FMyIndexBuffer :public FIndexBuffer
{
public:
	TArray<int32> Indices;
	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo);
		void* IndexBuffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(IndexBuffer, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};

class FMyFactory :public FLocalVertexFactory
{
public:
	FMyFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName):FLocalVertexFactory(InFeatureLevel, "FMyVertexFactory")
	{};
};

class FTestMyProxy : public FPrimitiveSceneProxy
{
public:
	FTestMyProxy(UPrimitiveComponent* Component) :FPrimitiveSceneProxy(Component), VertexFactory(GetScene().GetFeatureLevel(), "FMySceneProxy") {


		const int32 NumLineVertices = 4;
		const int32 NumLineIndices = 6;
		const int32 NumTextureCoordinates = 1;

		VertexBuffers.PositionVertexBuffer.Init(NumLineVertices);
		VertexBuffers.StaticMeshVertexBuffer.Init(NumLineVertices, NumTextureCoordinates);
		VertexBuffers.ColorVertexBuffer.Init(NumLineVertices);
		IndexBuffer.Indices.SetNumUninitialized(NumLineIndices);

		int32 VertexBufferIndex = 0;
		int32 IndexBufferIndex = 0;

		// Initialize lines.
		// Lines are represented as two tris of zero thickness. The UV's stored at vertices are actually (lineThickness, depthBias), 
		// which the material unpacks and uses to thicken the polygons and set the pixel depth bias.
		if (1)
		{
				VertexBuffers.PositionVertexBuffer.VertexPosition(VertexBufferIndex + 0) = FVector(0,0,0);
				VertexBuffers.PositionVertexBuffer.VertexPosition(VertexBufferIndex + 1) = FVector(0, 100, 0);
				VertexBuffers.PositionVertexBuffer.VertexPosition(VertexBufferIndex + 2) = FVector(0, 0, 100);
				VertexBuffers.PositionVertexBuffer.VertexPosition(VertexBufferIndex + 3) = FVector(0, 100, 100);

				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(VertexBufferIndex + 0, FVector::ForwardVector, FVector::ZeroVector, FVector::ZeroVector);
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(VertexBufferIndex + 1, FVector::ForwardVector, FVector::ZeroVector, FVector::ZeroVector);
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(VertexBufferIndex + 2, FVector::ForwardVector, FVector::ZeroVector, FVector::ZeroVector);
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(VertexBufferIndex + 3, FVector::ForwardVector, FVector::ZeroVector, FVector::ZeroVector);

				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(VertexBufferIndex + 0, 0, FVector2D(0, 0));
				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(VertexBufferIndex + 1, 0, FVector2D(1, 0));
				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(VertexBufferIndex + 2, 0, FVector2D(0, 1));
				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(VertexBufferIndex + 3, 0, FVector2D(1, 1));

				// The color stored in the vertices actually gets interpreted as a linear color by the material,
				// whereas it is more convenient for the user of the LineSet to specify colors as sRGB. So we actually
				// have to convert it back to linear. The ToFColor(false) call just scales back into 0-255 space.
				FColor color = FLinearColor::FromSRGBColor(FColor::Red).ToFColor(false);
				VertexBuffers.ColorVertexBuffer.VertexColor(VertexBufferIndex + 0) = color;
				VertexBuffers.ColorVertexBuffer.VertexColor(VertexBufferIndex + 1) = color;
				VertexBuffers.ColorVertexBuffer.VertexColor(VertexBufferIndex + 2) = color;
				VertexBuffers.ColorVertexBuffer.VertexColor(VertexBufferIndex + 3) = color;

				IndexBuffer.Indices[IndexBufferIndex + 0] = VertexBufferIndex + 0;
				IndexBuffer.Indices[IndexBufferIndex + 1] = VertexBufferIndex + 1;
				IndexBuffer.Indices[IndexBufferIndex + 2] = VertexBufferIndex + 2;
				IndexBuffer.Indices[IndexBufferIndex + 3] = VertexBufferIndex + 2;
				IndexBuffer.Indices[IndexBufferIndex + 4] = VertexBufferIndex + 3;
				IndexBuffer.Indices[IndexBufferIndex + 5] = VertexBufferIndex + 0;

				VertexBufferIndex += 4;
				IndexBufferIndex += 6;
		}

		ENQUEUE_RENDER_COMMAND(LineSetVertexBuffersInit)(
			[this](FRHICommandListImmediate& RHICmdList)
		{
			VertexBuffers.PositionVertexBuffer.InitResource();
			VertexBuffers.StaticMeshVertexBuffer.InitResource();
			VertexBuffers.ColorVertexBuffer.InitResource();

			FLocalVertexFactory::FDataType Data;
			VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&VertexFactory, Data);
			VertexBuffers.StaticMeshVertexBuffer.BindTexCoordVertexBuffer(&VertexFactory, Data);
			VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(&VertexFactory, Data);
			VertexFactory.SetData(Data);

			VertexFactory.InitResource();
			IndexBuffer.InitResource();
		});
	
	};

	virtual uint32 GetMemoryFootprint(void) const override
	{
		return (sizeof(*this) + GetAllocatedSize());
	};

	virtual void GetDynamicMeshElements(const TArray<const FSceneView *>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override
	{
		FBox TestBox = FBox(FVector(-100.0f), FVector(100.0f));

		DrawWireBox(Collector.GetPDI(0), GetLocalToWorld(), TestBox, FLinearColor(1.0, 0.0,0.0), ESceneDepthPriorityGroup::SDPG_Foreground);

		FMaterialRenderProxy* mtl = UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();
		
		FPrimitiveDrawInterface* PDI = Collector.GetPDI(0);
		if (PDI->View)
		{
			DrawSphere(Collector.GetPDI(0), FVector(0, 0, 0), FRotator::ZeroRotator, FVector(100, 100, 100), 12, 2, mtl, ESceneDepthPriorityGroup::SDPG_Foreground);
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
					const FSceneView* View = Views[ViewIndex];
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &IndexBuffer;
					Mesh.bWireframe = false;
					Mesh.VertexFactory = &VertexFactory;
					Mesh.MaterialRenderProxy = UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, false, DrawsVelocity(), false);
					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives =2;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = 5;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;
					Collector.AddMesh(ViewIndex, Mesh);
			}
		}
	};

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override
	{
		//FBox TestBox = FBox(FVector(-20.0f), FVector(20.0f));
		//FMaterialRenderProxy* mtl = UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();
		//DrawWireBox(PDI, GetLocalToWorld(), TestBox, FLinearColor(0.0, 1.0, 0.0), ESceneDepthPriorityGroup::SDPG_Foreground);

		//DrawSphere(PDI, FVector(0, 0, 0), FRotator::ZeroRotator, FVector(100, 100, 100), 12, 0, NULL, ESceneDepthPriorityGroup::SDPG_Foreground);
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	};

	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	};

	virtual ~FTestMyProxy()
	{
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	FLocalVertexFactory VertexFactory;
	FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;
};


UMyMeshComponent::UMyMeshComponent()
{
	GunOffset = FVector(0, 0, 1);
}

void UMyMeshComponent::test_func()
{
	UE_LOG(LogMyComponent, Warning, TEXT("just a test"));
}

FPrimitiveSceneProxy* UMyMeshComponent::CreateSceneProxy()
{
	return new FTestMyProxy(this);
}
