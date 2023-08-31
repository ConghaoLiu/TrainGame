#include "Space3DUnrealSource.h"
#include "Space3DUnreal.h"

#include "Space3D.hpp"

#define LOCTEXT_NAMESPACE "FSpace3DUnrealSource"

#if 0

UObject* USpace3DUnrealSourceSettingsFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
  return NewObject<USpace3DUnrealSourceSettings>(InParent, InClass, InName, Flags);
}

bool USpace3DUnrealSourceSettingsFactory::ShouldShowInNewMenu() const { return true; }

USpace3DUnrealSourceSettingsFactory::USpace3DUnrealSourceSettingsFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
  SupportedClass = USpace3DUnrealSourceSettings::StaticClass();
  bCreateNew = true;
  bEditAfterNew = true;
}

FSpace3DUnrealSourceSettingsActions::FSpace3DUnrealSourceSettingsActions() {}

bool FSpace3DUnrealSourceSettingsActions::CanFilter() { return true; }

uint32 FSpace3DUnrealSourceSettingsActions::GetCategories()
{
  return EAssetTypeCategories::Sounds;
}

FText FSpace3DUnrealSourceSettingsActions::GetName() const
{
  return LOCTEXT("SourceSettingsName", "Space3D Source Settings");
}

UClass* FSpace3DUnrealSourceSettingsActions::GetSupportedClass() const
{
  return USpace3DUnrealSourceSettings::StaticClass();
}

FColor FSpace3DUnrealSourceSettingsActions::GetTypeColor() const
{
  return FColor::Green;
}

bool FSpace3DUnrealSourceSettingsActions::HasActions(const TArray<UObject*>& InObjects) const
{
  return false;
}

#endif


FString FSpace3DUnrealSourceFactory::GetDisplayName()
{
  static FString DisplayName = FString(TEXT("Space3DUnreal"));
  return DisplayName;
}

bool FSpace3DUnrealSourceFactory::SupportsPlatform(const FString& PlatformName)
{
  return PlatformName == TEXT("Windows") || PlatformName == TEXT("Linux");
}

int32 FSpace3DUnrealSourceFactory::GetMaxSupportedChannels() { return 2; }

TAudioSpatializationPtr FSpace3DUnrealSourceFactory::CreateNewSpatializationPlugin(FAudioDevice* OwningDevice)
{
  UE_LOG(LogSpace3DUnreal, Display, TEXT("Creating FSpace3DUnrealSource (spatializer plugin)"));
  return MakeShareable(static_cast<IAudioSpatialization*>(new FSpace3DUnrealSource()));
}

UClass* FSpace3DUnrealSourceFactory::GetCustomSpatializationSettingsClass() const
{
  return USpace3DUnrealSourceSettings::StaticClass();
}

///

FSpace3DUnrealSource::FSpace3DUnrealSource()
{
  UE_LOG(LogSpace3DUnreal, Display, TEXT("FSpace3DUnrealSource constructor"));
  NumActiveSources = 0;
}
FSpace3DUnrealSource::~FSpace3DUnrealSource()
{
  Shutdown();
}

void FSpace3DUnrealSource::Initialize(const FAudioPluginInitializationParams InitializationParams)
{
  UE_LOG(LogSpace3DUnreal, Display, TEXT("FSpace3DUnrealSource Initialize, %d Hz, up to %d sources, buf len %d"), InitializationParams.SampleRate, InitializationParams.NumSources, InitializationParams.BufferLength);
  check(Space3DUnreal::IsActive());
  
  uuids.Empty();
  uuids.AddDefaulted(InitializationParams.NumSources);
  LastTs.Empty();
  LastTs.AddDefaulted(InitializationParams.NumSources);
  NumActiveSources = 0;
  Space3D::GetParams()->fs = (float)InitializationParams.SampleRate;
  Space3D::SetFrameLength(InitializationParams.BufferLength);
}
void FSpace3DUnrealSource::Shutdown()
{
  UE_LOG(LogSpace3DUnreal, Display, TEXT("FSpace3DUnrealSource Shutdown"));
  if(!Space3DUnreal::IsActive()) return; //Module de-initialized before spat plugin
  for(int32 i=0; i<uuids.Num(); ++i)
  {
    if(uuids[i] != 0) Space3D::SourceRemove(uuids[i]);
  }
}

void FSpace3DUnrealSource::OnInitSource(const uint32 SourceId, const FName& AudioComponentUserId, USpatializationPluginSourceSettingsBase* InSettings)
{
  UE_LOG(LogSpace3DUnreal, Display, TEXT("FSpace3DUnrealSource OnInitSource %d"), SourceId);
  check(Space3DUnreal::IsActive());
  
  if(uuids[SourceId] != 0)
  {
    Space3D::SourceRemove(uuids[SourceId]);
    uuids[SourceId] = 0;
    --NumActiveSources;
  }
  uuids[SourceId] = Space3D::SourceAdd();
  ++NumActiveSources;
  
  if(InSettings)
  {
    UE_LOG(LogSpace3DUnreal, Display, TEXT("Source added with spatialization settings."));
    USpace3DUnrealSourceSettings* Settings = CastChecked<USpace3DUnrealSourceSettings>(InSettings);
    Space3D::SourceSetVolume(uuids[SourceId], (audiofloat)Settings->Volume);
    Space3D::SourceSetDir(uuids[SourceId], static_cast<Space3D::DirType>(Settings->DirType), Settings->DirBeamWidth, Settings->DirCosMixFactor, Settings->DirUnused);
    Space3D::SourceSetThresholds(uuids[SourceId], Settings->ThresholdFull, Settings->ThresholdZero);
  }
  else
  {
    UE_LOG(LogSpace3DUnreal, Display, TEXT("Source added without spatialization settings."));
  }
}
void FSpace3DUnrealSource::OnReleaseSource(const uint32 SourceId)
{
  UE_LOG(LogSpace3DUnreal, Display, TEXT("FSpace3DUnrealSource ReleaseSource %d"), SourceId);
  check(Space3DUnreal::IsActive());
  check(uuids[SourceId] != 0);
  Space3D::SourceRemove(uuids[SourceId]);
  uuids[SourceId] = 0;
  LastTs[SourceId] = 0;
  --NumActiveSources;
}

void FSpace3DUnrealSource::ProcessAudio(const FAudioPluginSourceInputData& InputData, FAudioPluginSourceOutputData& OutputData)
{
  //Clear output audio
  FMemory::Memset(OutputData.AudioBuffer.GetData(), 0, OutputData.AudioBuffer.Num() * sizeof(float));
  check(Space3DUnreal::IsActive());
  check(InputData.NumChannels == 1 || InputData.NumChannels == 2);
  check(InputData.AudioBuffer);
  check(NumActiveSources > 0);
  check(InputData.SourceId >= 0 && InputData.SourceId < uuids.Num());
  int spls = InputData.AudioBuffer->Num() / InputData.NumChannels;
  if(spls != Space3D::FrameLength())
  {
    UE_LOG(LogSpace3DUnreal, Warning, TEXT("Wrong buffer size: input %d Space3D %d. Make sure there is a Space3DUnrealOutput submix effect running."),
      spls, Space3D::FrameLength());
    return;
  }
  //Physics update
  const FSpatializationParams* Spat = InputData.SpatializationParams;
  uint64_t uuid = uuids[InputData.SourceId];
  check(uuid != 0);
  uint64_t t = Space3DUnreal::AudioClockToS3DTime(Spat->AudioClock);
  Space3DUnreal::SetAudioT(t);
  if(t == LastTs[InputData.SourceId])
  {
    //UE_LOG(LogSpace3DUnreal, Warning, TEXT("Source processed again at same time"));
  }
  else if(t < LastTs[InputData.SourceId])
  {
    UE_LOG(LogSpace3DUnreal, Warning, TEXT("Source processed back in time"));
  }
  LastTs[InputData.SourceId] = t;
  //UE_LOG(LogSpace3DUnreal, Display, TEXT("Audio clock %f"), Spat->AudioClock);
  
  float Scale = Space3DUnreal::GetScaleFactor();
  glm::vec3 SPos = Scale * U2GV(Spat->EmitterWorldPosition);
  //glm::vec3 LPos = Scale * U2GV(Spat->ListenerPosition);
  //UE_LOG(LogSpace3DUnreal, Display, TEXT("Source: %f %f %f | Listener: %f %f %f"), SPos.x, SPos.y, SPos.z, LPos.x, LPos.y, LPos.z);
  Space3D::PhysUpdate(uuid, t, SPos, U2GQ(Spat->EmitterWorldRotation));
  
  //Input audio
  float *buf;
  TArray<float> tempBuf;
  if(InputData.NumChannels == 1)
  {
    buf = InputData.AudioBuffer->GetData();
  }
  else
  {
    //Left channel only
    tempBuf.SetNum(spls);
    buf = tempBuf.GetData();
    const float *inbuf = InputData.AudioBuffer->GetData();
    for(int i=0; i<spls; ++i) buf[i] = inbuf[InputData.NumChannels*i];
  }
  Space3D::SourceWrite(uuid, (const audiofloat*)buf);
}

void FSpace3DUnrealSource::OnAllSourcesProcessed()
{
  check(Space3DUnreal::IsActive());
  if(NumActiveSources > 0)
  {
    Space3D::Process(Space3DUnreal::GetAudioT());
    Space3DUnreal::SetOutputAudioAvailable();
  }
}

#undef LOCTEXT_NAMESPACE
