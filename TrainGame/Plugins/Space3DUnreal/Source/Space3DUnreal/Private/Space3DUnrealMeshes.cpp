#include "Space3DUnrealMeshes.h"
#include "PhysXPublic.h"
#include "SkeletalRenderPublic.h"
#include "Space3D.hpp"

FString LogText;

USpace3DUnrealMesh::USpace3DUnrealMesh(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  , bStatic(false)
  , LastMaterialIndex(-1)
  , NumVertices(0)
  , NumTriangles(0)
  , SkelComponent(nullptr)
  , SkelMesh(nullptr)
  , SkelPhysAsset(nullptr)
{
  //UE_LOG(LogSpace3DUnreal, Log, TEXT("USpace3DUnrealMesh::USpace3DUnrealMesh()"));
}

USpace3DUnrealMesh::~USpace3DUnrealMesh()
{
  for(int32 i=0; i<SkelSegments.Num(); ++i)
  {
    delete SkelSegments[i];
  }
}

void USpace3DUnrealMesh::CreateS3DObject()
{
  //UE_LOG(LogSpace3DUnreal, Log, TEXT("USpace3DUnrealMesh::CreateS3DObject()"));
}

void USpace3DUnrealMesh::DestroyS3DObject()
{
  //UE_LOG(LogSpace3DUnreal, Log, TEXT("USpace3DUnrealMesh::DestroyS3DObject()"));
  if(uuid != 0) Space3D::MeshRemove(uuid);
  bStatic = false;
  LastMaterialIndex = -1;
  NumVertices = NumTriangles = 0;
  SkelComponent = nullptr;
  SkelMesh = nullptr;
  SkelPhysAsset = nullptr;
  for(int32 i=0; i<SkelSegments.Num(); ++i)
  {
    delete SkelSegments[i];
  }
  SkelSegments.Empty();
}

bool USpace3DUnrealMesh::RequiresScaleScale()
{
    return true;
}

static void ImportMesh(int32& NumVertices, TArray<FVector>& Vertices, TArray<int32>& Indices, const FTransform& Transform, const TArray<FVector>& VerticesIn, const TArray<int32>& IndicesIn)
{
  check(VerticesIn.Num() > 0);
  check(IndicesIn.Num() > 0);
  check(IndicesIn.Num() % 3 == 0);
  for(int v=0; v<VerticesIn.Num(); ++v)
  {
    Vertices.Add(Transform.TransformPosition(VerticesIn[v]));
  }
  for(int i=0; i<IndicesIn.Num(); ++i)
  {
    check(IndicesIn[i] >= 0 && IndicesIn[i] < VerticesIn.Num());
    Indices.Add(IndicesIn[i] + NumVertices);
  }
  NumVertices += VerticesIn.Num();
}

static void ImportBox(int32& NumVertices, TArray<FVector>& Vertices, TArray<int32>& Indices, const FTransform& Transform, float SizeX, float SizeY, float SizeZ)
{
  TArray<FVector> VerticesIn;
  float SX2 = SizeX * 0.5f;
  float SY2 = SizeY * 0.5f;
  float SZ2 = SizeZ * 0.5f;
  VerticesIn.Add(FVector( SX2,  SY2,  SZ2));
  VerticesIn.Add(FVector(-SX2,  SY2,  SZ2));
  VerticesIn.Add(FVector( SX2, -SY2,  SZ2));
  VerticesIn.Add(FVector(-SX2, -SY2,  SZ2));
  VerticesIn.Add(FVector( SX2,  SY2, -SZ2));
  VerticesIn.Add(FVector(-SX2,  SY2, -SZ2));
  VerticesIn.Add(FVector( SX2, -SY2, -SZ2));
  VerticesIn.Add(FVector(-SX2, -SY2, -SZ2));
  TArray<int32> IndicesIn;
  IndicesIn.Append({0, 2, 1, 1, 2, 3});
  IndicesIn.Append({4, 5, 6, 6, 5, 7});
  IndicesIn.Append({0, 1, 4, 4, 1, 5});
  IndicesIn.Append({2, 6, 3, 3, 6, 7});
  IndicesIn.Append({0, 4, 2, 2, 4, 6});
  IndicesIn.Append({1, 3, 5, 5, 3, 7});
  ImportMesh(NumVertices, Vertices, Indices, Transform, VerticesIn, IndicesIn);
}

#define N_LONGITUDE 24
#define N_LATITUDE_HEMI 6
//Z direction is always major.

static void ImportHemisphere(int32& NumVertices, TArray<FVector>& Vertices, TArray<int32>& Indices, const FTransform& Transform, float Radius, float ZOffset, bool NegZ)
{
  TArray<FVector> VerticesIn;
  TArray<int32> IndicesIn;
  float BaseZ = NegZ ? -1.0f : 1.0f;
  for(int32 lati = 0; lati < N_LATITUDE_HEMI; ++lati)
  {
    float alti = PI * 0.5f * (float)lati / (float)N_LATITUDE_HEMI;
    int32 ib = lati*N_LONGITUDE;
    int32 nib = (lati+1)*N_LONGITUDE;
    for(int32 longi = 0; longi < N_LONGITUDE; ++longi)
    {
      float azi = PI * 2.0f * (float)longi / (float)N_LONGITUDE;
      float x = Radius * cos(alti) * cos(azi);
      float y = Radius * cos(alti) * sin(azi);
      float z = BaseZ * Radius * sin(alti) + ZOffset;
      VerticesIn.Add(FVector(x, y, z));
      int32 nx = (longi == N_LONGITUDE - 1) ? 0 : longi + 1;
      int32 i0 = ib + longi;
      int32 i1 = ib + nx;
      if(lati == N_LATITUDE_HEMI - 1)
      {
        int32 i2 = N_LATITUDE_HEMI * N_LONGITUDE;
        if(NegZ){
          IndicesIn.Append({i0, i1, i2});
        }else{
          IndicesIn.Append({i0, i2, i1});
        }
      }else{
        int32 i2 = nib + longi;
        int32 i3 = nib + nx;
        if(NegZ){
          IndicesIn.Append({i0, i1, i2});
          IndicesIn.Append({i2, i1, i3});
        }else{
          IndicesIn.Append({i0, i2, i1});
          IndicesIn.Append({i2, i3, i1});
        }
      }
    }
  }
  VerticesIn.Add(FVector(0.0f, 0.0f, BaseZ * Radius + ZOffset));
  ImportMesh(NumVertices, Vertices, Indices, Transform, VerticesIn, IndicesIn);
}

static void ImportSphere(int32& NumVertices, TArray<FVector>& Vertices, TArray<int32>& Indices, const FTransform& Transform, float Radius)
{
  ImportHemisphere(NumVertices, Vertices, Indices, Transform, Radius, 0.0f, false);
  ImportHemisphere(NumVertices, Vertices, Indices, Transform, Radius, 0.0f, true);
}

static void ImportTaperedCapsule(int32& NumVertices, TArray<FVector>& Vertices, TArray<int32>& Indices, const FTransform& Transform, float Length, float Radius1, float Radius2)
{
  int32 NV1 = NumVertices;
  ImportHemisphere(NumVertices, Vertices, Indices, Transform, Radius1, Length * 0.5f, false);
  int32 NV2 = NumVertices;
  ImportHemisphere(NumVertices, Vertices, Indices, Transform, Radius2, Length * -0.5f, true);
  //Connect existing hemisphere vertices
  for(int32 i=0; i<N_LONGITUDE; ++i)
  {
    int32 ip1 = (i == N_LONGITUDE - 1) ? 0 : i+1;
    Indices.Append({NV1 + i, NV1 + ip1, NV2 + i, NV2 + i, NV1 + ip1, NV2 + ip1});
  }
}

static void ImportConvex(int32& NumVertices, TArray<FVector>& Vertices, TArray<int32>& Indices, const FKConvexElem& Cvx)
{
  //Code from FKConvexElem::DrawElemWire
#if PHYSICS_INTERFACE_PHYSX
  PxConvexMesh* Mesh = Cvx.GetConvexMesh();
  if(!Mesh)
  {
    UE_LOG(LogSpace3DUnreal, Warning, TEXT("%s: PhysX ConvexMesh not set up correctly internally"));

    return;
  }
  
  PxU32 NumPxVertices = Mesh->getNbVertices();
  check(NumPxVertices > 0);
  const PxVec3* PxVertices = Mesh->getVertices();
  check(PxVertices);
  // Geometry is stored in body space, so no need to transform
  for(PxU32 i=0; i<NumPxVertices; ++i)
  {
    Vertices.Add(P2UVector(PxVertices[i]));
  }
  check(Vertices.Num() >= (int32)NumPxVertices);
    
  PxU32 NbPolygons = Mesh->getNbPolygons();
  check(NbPolygons > 0);
  const PxU8* PIndexBuffer = Mesh->getIndexBuffer();
  check(PIndexBuffer);
  for(PxU32 i=0;i<NbPolygons;i++)
  {
    PxHullPolygon Data;
    bool bStatus = Mesh->getPolygonData(i, Data);
    check(bStatus);

    const PxU8* PIndices = PIndexBuffer + Data.mIndexBase;
    check(Data.mNbVerts >= 3);
    check(PIndices[0] < NumPxVertices);
    check(PIndices[1] < NumPxVertices);
    
    // Polys bounding the convex hull are often higher-order (quads, etc.) so they must be triangulated. However, since they are convex, any trivial triangulation is guaranteed to be valid.
    for(PxU16 j=0; j<Data.mNbVerts-2; ++j)
    {
      check(PIndices[j+2] < NumPxVertices);
      Indices.Add(NumVertices + PIndices[0]);
      Indices.Add(NumVertices + PIndices[j+2]);
      Indices.Add(NumVertices + PIndices[j+1]);
    }
  }
  
  NumVertices += NumPxVertices;
#else
  UE_LOG(LogSpace3DUnreal, Warning, TEXT("%s: Collision mesh contains convex bodies, but this is only supported with the PhysX backend"));
#endif
}


static void TransformInto(TArray<FVector>& Vertices, int32 VStartIdx, const TArray<FVector>& VerticesIn, const FTransform& BoneTransform)
{
  for(int v=0; v<VerticesIn.Num(); ++v)
  {
    Vertices[VStartIdx+v] = BoneTransform.TransformPosition(VerticesIn[v]);
  }
}



void USpace3DUnrealMesh::PreTick()
{
  //UE_LOG(LogSpace3DUnreal, Log, TEXT("USpace3DUnrealMesh::PreTick()"));
  
  AActor *Owner = GetOwner();
  if(Owner == nullptr)
  {
    UE_LOG(LogSpace3DUnreal, Error, TEXT("Space3DUnrealMesh must be attached to an actor"));

    IntentionallyNotCreated = true;
    return;
  }

  USceneComponent *Parent = GetAttachParent();
  if(Parent == nullptr){
    UE_LOG(LogSpace3DUnreal, Error, TEXT("%s: Space3DUnrealMesh must be attached to some sort of mesh component within an actor"), *Owner->GetName());

    IntentionallyNotCreated = true;
    return;
  }
  
  if(bStatic)
  {
    check(uuid != 0);
    //No need to update the mesh again
    return;
  }
  
  bool bHaveLock = false;
  if(uuid == 0)
  {
    check(SkelComponent == nullptr);
    check(SkelMesh == nullptr);
    check(SkelPhysAsset == nullptr);
    
    //New object
    
    UStaticMeshComponent* StaticMeshC = Cast<UStaticMeshComponent>(Parent);
    if(StaticMeshC != nullptr)
    {
      // Create and submit data for static mesh
      UE_LOG(LogSpace3DUnreal, Log, TEXT("%s: Creating static mesh"), *Owner->GetName());
      
      IInterface_CollisionDataProvider* StaticColDataSrc = StaticMeshC->GetStaticMesh();
      bStatic = true;
      
      if(!StaticColDataSrc->ContainsPhysicsTriMeshData(true))
      {
        UE_LOG(LogSpace3DUnreal, Error, TEXT("%s: CollisionDataProvider does not have physics tri mesh data"), *Owner->GetName());

        IntentionallyNotCreated = true;
        return;
      }
    
      FTriMeshCollisionData ColData;
      if(!StaticColDataSrc->GetPhysicsTriMeshData(&ColData, true))
      {
        UE_LOG(LogSpace3DUnreal, Error, TEXT("%s: Could not get collision tri mesh from CollisionDataProvider"), *Owner->GetName());

        IntentionallyNotCreated = true;
        return;
      }
      //"Indices" is actually triangles
      NumVertices = ColData.Vertices.Num();
      NumTriangles = ColData.Indices.Num();
      const glm::vec3 *Normals = nullptr;
      if(ColData.Normals.Num() > 0)
      {
        Normals = (const glm::vec3*)ColData.Normals.GetData();
        /*
        UE_LOG(LogSpace3DUnreal, Log, TEXT("    Normals:"));
        for(int i=0; i<NumVertices && i<50; ++i)
        {
          UE_LOG(LogSpace3DUnreal, Log, TEXT("    (%f, %f, %f)"), Normals[i].x, Normals[i].y, Normals[i].z);
        }
        */
      }
      else
      {
        UE_LOG(LogSpace3DUnreal, Warning, TEXT("%s: Static mesh does not have normals"), *Owner->GetName());
      }
      Space3D::BeginAtomicAccess();
      uuid = Space3D::MeshAdd(NumVertices, NumTriangles, (const uint32_t*)ColData.Indices.GetData());
      Space3D::MeshSetVertices(uuid, (const glm::vec3*)ColData.Vertices.GetData(), Normals, false);
      Space3D::EndAtomicAccess();
      UE_LOG(LogSpace3DUnreal, Log, TEXT("%s: Added static mesh"), *Owner->GetName());

      return;
    }
    
    SkelComponent = Cast<USkinnedMeshComponent>(Parent);
    if(SkelComponent == nullptr)
    {
      // Must be skinned (usually skeletal) mesh
      
      UE_LOG(LogSpace3DUnreal, Error, TEXT("%s: Space3DUnrealMesh must be attached to an actor with a UStaticMeshComponent, or a USkinnedMeshComponent (including USkeletalMeshComponent) which has a UPhysicsAsset on its USkeletalMesh."), *Owner->GetName());

      IntentionallyNotCreated = true;
      return;
    }
    
    // Create skeletal mesh and submit indices
    
    UE_LOG(LogSpace3DUnreal, Log, TEXT("%s: Creating skeletal mesh"), *Owner->GetName());

    SkelMesh = SkelComponent->SkeletalMesh;
    if(SkelMesh != nullptr)
    {
      SkelPhysAsset = SkelMesh->PhysicsAsset;
    }
    if(SkelPhysAsset == nullptr)
    {
      UE_LOG(LogSpace3DUnreal, Error, TEXT("%s: USkinnedMeshComponent (usually USkeletalMeshComponent) which this USpace3DUnrealMesh is attached to must have a physics asset attached to its SkeletalMesh."), *Owner->GetName());

      IntentionallyNotCreated = true;
      return;
    }
      
    NumVertices = 0;
    TArray<int32> Indices;
    
    //Code adapted from UPhysicsAsset::GetCollisionMesh and FKAggregateGeom::GetAggGeom
    for(int32 j=0; j<SkelPhysAsset->SkeletalBodySetups.Num(); ++j)
    {
      UBodySetup* Setup = SkelPhysAsset->SkeletalBodySetups[j];
      //UE_LOG(LogSpace3DUnreal, Log, TEXT("Col mesh for %s"), *Setup->BoneName.ToString());
      if (Setup->bCreatedPhysicsMeshes)
      {
        SegmentInfo* SegInfo = new SegmentInfo();
        SegInfo->BodySetupIdx = j;
        SegInfo->BoneIndex = SkelMesh->RefSkeleton.FindBoneIndex(Setup->BoneName);
        check(SegInfo->BoneIndex >= 0);
        FKAggregateGeom* Agg = &Setup->AggGeom;

        for (int32 i = 0; i < Agg->BoxElems.Num(); i++)
        {
          //UE_LOG(LogSpace3DUnreal, Log, TEXT("--Importing box"));
          const FKBoxElem& Elem = Agg->BoxElems[i];
          ImportBox(NumVertices, SegInfo->Vertices, Indices, Elem.GetTransform(), Elem.X, Elem.Y, Elem.Z);
        }

        for (int32 i = 0; i < Agg->SphereElems.Num(); i++)
        {
          //UE_LOG(LogSpace3DUnreal, Log, TEXT("--Importing sphere"));
          const FKSphereElem& Elem = Agg->SphereElems[i];
          ImportSphere(NumVertices, SegInfo->Vertices, Indices, Elem.GetTransform(), Elem.Radius);
        }

        for (int32 i = 0; i < Agg->SphylElems.Num(); i++)
        {
          //UE_LOG(LogSpace3DUnreal, Log, TEXT("--Importing capsule"));
          const FKSphylElem& Elem = Agg->SphylElems[i];
          ImportTaperedCapsule(NumVertices, SegInfo->Vertices, Indices, Elem.GetTransform(), Elem.Length, Elem.Radius, Elem.Radius);
        }
        
        for (int32 i = 0; i < Agg->TaperedCapsuleElems.Num(); i++)
        {
          //UE_LOG(LogSpace3DUnreal, Log, TEXT("--Importing tapered capsule"));
          const FKTaperedCapsuleElem& Elem = Agg->TaperedCapsuleElems[i];
          ImportTaperedCapsule(NumVertices, SegInfo->Vertices, Indices, Elem.GetTransform(), Elem.Length, Elem.Radius0, Elem.Radius1);
        }
        
        for (int32 i = 0; i < Agg->ConvexElems.Num(); i++)
        {
          const FKConvexElem& Elem = Agg->ConvexElems[i];
          //UE_LOG(LogSpace3DUnreal, Log, TEXT("--Importing convex"));
          ImportConvex(NumVertices, SegInfo->Vertices, Indices, Elem);
        }
        
        if(SegInfo->Vertices.Num() == 0)
        {
          UE_LOG(LogSpace3DUnreal, Warning, TEXT("%s: SkeletalBodySetup[%d] does not contain any geometry"), *Owner->GetName(), j);

          delete SegInfo;
        }
        else
        {
          SkelSegments.Add(SegInfo);
        }
      }
      else
      {
        UE_LOG(LogSpace3DUnreal, Warning, TEXT("%s: SkeletalBodySetup[%d] has not created physics meshes"), *Owner->GetName(), j);
      }
    }
    if(NumVertices == 0 || Indices.Num() == 0 || SkelSegments.Num() == 0)
    {
      UE_LOG(LogSpace3DUnreal, Error, TEXT("%s: Collision mesh is empty; incorrectly created physics asset or bug"), *Owner->GetName());

      IntentionallyNotCreated = true;
      return;
    }
    check(Indices.Num() % 3 == 0);
    NumTriangles = Indices.Num() / 3;
    Space3D::BeginAtomicAccess();
    bHaveLock = true;
    uuid = Space3D::MeshAdd(NumVertices, NumTriangles, (const uint32_t*)Indices.GetData());
    UE_LOG(LogSpace3DUnreal, Log, TEXT("%s: Added skeletal mesh"), *Owner->GetName());
  }
  
  // Update skeletal mesh vertices
  check(uuid != 0);
  check(SkelComponent != nullptr);
  check(SkelMesh != nullptr);
  check(SkelPhysAsset != nullptr);
  const TArray<FTransform>& SpaceBases = SkelComponent->GetComponentSpaceTransforms();
  TArray<FVector> Vertices;
  Vertices.AddDefaulted(NumVertices);
  
  int32 v=0;
  for(int32 s=0; s < SkelSegments.Num(); ++s)
  {
    check(SkelSegments[s]->BodySetupIdx >= 0 && SkelSegments[s]->BodySetupIdx < SkelPhysAsset->SkeletalBodySetups.Num());
    UBodySetup* Setup = SkelPhysAsset->SkeletalBodySetups[SkelSegments[s]->BodySetupIdx];
    check(Setup->bCreatedPhysicsMeshes);
    
    check(SkelSegments[s]->BoneIndex >= 0 && SkelSegments[s]->BoneIndex < SpaceBases.Num());
    check(SkelSegments[s]->Vertices.Num() > 0);
    TransformInto(Vertices, v, SkelSegments[s]->Vertices, SpaceBases[SkelSegments[s]->BoneIndex]);
    v += SkelSegments[s]->Vertices.Num();
  }
  check(v == NumVertices);
  
  if(!bHaveLock)
  {
    Space3D::BeginAtomicAccess();
  }
  Space3D::MeshSetVertices(uuid, (const glm::vec3*)Vertices.GetData(), nullptr, false);
  Space3D::EndAtomicAccess();
  
  /*
  FMeshElementCollector Collector;
  int32 ViewIndex = 0;
  SkelPhysAsset->GetCollisionMesh(ViewIndex, Collector, SkelMesh, SkelComponent->GetComponentSpaceTransforms(), SkelComponent->GetComponentTransform(), FVector(1.0f, 1.0f, 1.0f));
  */
}
  
void USpace3DUnrealMesh::UpdateS3DProps()
{
  if(MaterialIndex != LastMaterialIndex)
  {
    Space3D::MeshSetMaterial(uuid, MaterialIndex);
    LastMaterialIndex = MaterialIndex;
  }
}
