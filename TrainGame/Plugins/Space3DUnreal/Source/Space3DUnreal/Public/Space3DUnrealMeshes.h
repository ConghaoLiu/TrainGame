#pragma once

#include "Space3DUnreal.h"

#include "Space3DUnrealMeshes.generated.h"

/** A mesh which audio can reflect off and diffract around. Attach this to an actor with a UStaticMeshComponent, or a USkinnedMeshComponent (including USkeletalMeshComponent) which has a UPhysicsAsset on its USkeletalMesh. */
UCLASS(BlueprintType, ClassGroup=Audio, EditInlineNew, meta=(BlueprintSpawnableComponent))
class SPACE3DUNREAL_API USpace3DUnrealMesh : public USpace3DUnrealComponent
{
  GENERATED_BODY()
  
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "255"))
  int MaterialIndex;
  
  USpace3DUnrealMesh(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  ~USpace3DUnrealMesh();
  
  virtual void CreateS3DObject() override;
  virtual void DestroyS3DObject() override;
  virtual bool RequiresScaleScale() override;
  virtual void PreTick() override;
  virtual void UpdateS3DProps() override;
  
private:
  struct SegmentInfo {
    int32 BodySetupIdx, BoneIndex;
    TArray<FVector> Vertices;
  };

  bool bStatic;
  int LastMaterialIndex;
  int32 NumVertices, NumTriangles;
  USkinnedMeshComponent* SkelComponent;
  USkeletalMesh* SkelMesh;
  UPhysicsAsset* SkelPhysAsset;
  TArray<SegmentInfo*> SkelSegments;
};
