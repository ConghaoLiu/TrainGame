#pragma once

#include "CoreMinimal.h"
#include "IAudioExtensionPlugin.h"
#if 0
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#endif

#include "Space3DUnrealSource.generated.h"

UCLASS(editinlinenew, BlueprintType)
class SPACE3DUNREAL_API USpace3DUnrealSourceSettings : public USpatializationPluginSourceSettingsBase
{
  GENERATED_BODY()
 
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "10.0", UIMin = "0.0", UIMax = "1.5"))
  float Volume = 1.0f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0", ClampMax = "40", UIMin = "0", UIMax = "40"))
  int32 DirType = 0;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "1.0", ClampMax = "179.0", UIMin = "1.0", UIMax = "179.0"))
  float DirBeamWidth = 45.0f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float DirCosMixFactor = 0.7f;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float DirUnused = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float ThresholdFull = 0.5f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
  float ThresholdZero = 0.3f;
};

#if 0

UCLASS(hidecategories=Object)
class SPACE3DUNREAL_API USpace3DUnrealSourceSettingsFactory : public UFactory
{
 GENERATED_BODY()
public:
 USpace3DUnrealSourceSettingsFactory(const FObjectInitializer& ObjectInitializer);

 virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};

class FSpace3DUnrealSourceSettingsActions : public FAssetTypeActions_Base
{
public:
 FSpace3DUnrealSourceSettingsActions();
 virtual bool CanFilter() override;
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
};

#endif

class SPACE3DUNREAL_API FSpace3DUnrealSourceFactory : public IAudioSpatializationFactory
{
public:
  virtual FString GetDisplayName() override;
  virtual bool SupportsPlatform(const FString& PlatformName) override;
  virtual int32 GetMaxSupportedChannels() override;
  virtual UClass* GetCustomSpatializationSettingsClass() const;
  
  virtual TAudioSpatializationPtr CreateNewSpatializationPlugin(FAudioDevice* OwningDevice) override;
};

class SPACE3DUNREAL_API FSpace3DUnrealSource : public IAudioSpatialization
{
public:
  FSpace3DUnrealSource();
  virtual ~FSpace3DUnrealSource();
  
  virtual void Initialize(const FAudioPluginInitializationParams InitializationParams) override;
  virtual void Shutdown() override;
  
  virtual void OnInitSource(const uint32 SourceId, const FName& AudioComponentUserId, USpatializationPluginSourceSettingsBase* InSettings) override;
	virtual void OnReleaseSource(const uint32 SourceId) override;

	virtual void ProcessAudio(const FAudioPluginSourceInputData& InputData, FAudioPluginSourceOutputData& OutputData) override;
	virtual void OnAllSourcesProcessed() override;
private:
  TArray<uint64_t> uuids;
  TArray<uint64_t> LastTs;
  int32 NumActiveSources;
};
