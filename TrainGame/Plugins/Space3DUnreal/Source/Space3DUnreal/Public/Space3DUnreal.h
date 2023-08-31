// Some code from https://github.com/ue4plugins/TextAsset

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include <atomic>
#include <thread>

#include "Space3DUnreal.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpace3DUnreal, Log, All);

class FSpace3DUnrealModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Handle to the test dll we will load */
	void*	S3DLibraryHandle;
};

namespace Space3DUnreal {
  
  bool IsActive();
  
  float GetScaleFactor();
  void SetScaleFactor(float Scale);
  
  void SetOutputAudioAvailable();
  uint32_t GetAndConsumeOutputAudio();
  
  void SetAudioT(uint64_t t);
  uint64_t GetAudioT();
  
  inline uint64_t AudioClockToS3DTime(double AudioClock){
    return (uint64_t)(AudioClock * 1000000000.0);
  }

  extern std::atomic_flag ViewerKeepRunningFlag;
  extern std::thread* ViewerThread;

  void ViewerThreadRun();
  
}

#define U2GQ(fquat) glm::quat(fquat.W, fquat.X, fquat.Y, fquat.Z)
#define U2GV(fvec) glm::vec3(fvec.X, fvec.Y, fvec.Z)


/** Abstract base class for Space3D physics stuff. */
UCLASS(Abstract, BlueprintType)
class SPACE3DUNREAL_API USpace3DUnrealComponent : public USceneComponent
{
  GENERATED_BODY()
  
public:
  USpace3DUnrealComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  virtual ~USpace3DUnrealComponent();
  
  //Override these
  virtual void CreateS3DObject();
  virtual void DestroyS3DObject();
  virtual bool RequiresScaleScale(); //default false, override if want true
  virtual void PreTick(); //optional
  virtual void UpdateS3DProps();
  
  //Don't override these
  virtual void OnRegister() override;
  virtual void OnUnregister() override;
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
  
protected:
  uint64_t uuid; //Put uuid of object from Space3D here
  bool IntentionallyNotCreated;
  
private:
  uint64_t LastT;
};
