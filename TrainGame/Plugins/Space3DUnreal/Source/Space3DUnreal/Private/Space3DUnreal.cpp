#include "Space3DUnreal.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "AudioMixer.h"

#include <chrono>
#include <locale>
#include <codecvt>

#include "Space3D.hpp"
#include "Viewer.hpp"
#include "Space3DUnrealSource.h"

DEFINE_LOG_CATEGORY(LogSpace3DUnreal)

#define LOCTEXT_NAMESPACE "FSpace3DUnrealModule"

namespace Space3DUnreal {
  
  static bool Active = false;
  bool IsActive() { return Active; }
  
  static float ScaleFactor = 0.01f;
  float GetScaleFactor() { return ScaleFactor; }
  void SetScaleFactor(float Scale) { ScaleFactor = Scale; }
  
  static std::atomic<uint32_t> NumBuffersAvailable;
  
  void SetOutputAudioAvailable() { ++NumBuffersAvailable; }
  uint32_t GetAndConsumeOutputAudio() { return NumBuffersAvailable.exchange(0); }
  
  static std::atomic<uint64_t> AudioT;
  
  void SetAudioT(uint64_t t) { AudioT.store(t, std::memory_order_relaxed); }
  uint64_t GetAudioT() { return AudioT.load(std::memory_order_relaxed); }
  
  static FSpace3DUnrealSourceFactory* SourceFactory = nullptr;
  
  std::atomic_flag ViewerKeepRunningFlag;
  std::thread* ViewerThread = nullptr;

  #if 0

  TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
  
	/**
	 * Registers a single asset type action.
	 *
	 * @param AssetTools The asset tools object to register with.
	 * @param Action The asset type action to register.
	 */
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}
  
  /** Registers asset tool actions. */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		RegisterAssetTypeAction(AssetTools, MakeShareable(new FSpace3DUnrealSourceSettingsActions()));
	}

  /** Unregisters asset tool actions. */
	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule != nullptr)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}

  #endif
  
  void ViewerThreadRun()
  {
    Space3D::Viewer::Init(false);
    while(true)
    {
      if(Space3D::Viewer::WantExit()) break;
      if(!ViewerKeepRunningFlag.test_and_set()) break;
      Space3D::Viewer::DrawScene();
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    Space3D::Viewer::Finalize();
  }
  
  // Partly from https://stackoverflow.com/questions/2573834/c-convert-string-or-char-to-wstring-or-wchar-t/8969776#8969776
  FString s2ue4(const char *s)
  {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring ws = converter.from_bytes(s);
    return FString(ws.c_str());
  }
  
  void ErrHandler(const char *msg)
  {
    UE_LOG(LogSpace3DUnreal, Error, TEXT("Space3D: %s"), *s2ue4(msg));
  }
  
  void MsgHandler(const char *msg)
  {
    UE_LOG(LogSpace3DUnreal, Log, TEXT("Space3D: %s"), *s2ue4(msg));
  }
  
}

void FSpace3DUnrealModule::StartupModule()
{
  // Get data directory from /Config/Space3DDataDir.txt
  FString DataDir;
  FFileHelper::LoadFileToString(DataDir, *(FPaths::ProjectDir() + "/Config/Space3DDataDir.txt"));
  FString GPUToUseString;
  FFileHelper::LoadFileToString(GPUToUseString, *(FPaths::ProjectDir() + "/Config/Space3DGPUChoice.txt"));
  int GPUToUse = FCString::Atoi(*GPUToUseString);

  if(DataDir.IsEmpty())
  {
	  UE_LOG(LogSpace3DUnreal, Error, TEXT("You must include the path to the Space3D data folder in /Config/Space3DDataDir.txt. Path is an absolute path or relative path from the UE project file to the Space3D data folder."));
  }
  else
  {
    DataDir = FPaths::ConvertRelativePathToFull(DataDir);
    if(!FPaths::FileExists(FPaths::Combine(DataDir, TEXT("materials.cfg"))) || !FPaths::FileExists(FPaths::Combine(DataDir, TEXT("hrtfs.cfg"))))
    {
	    UE_LOG(LogSpace3DUnreal, Error, TEXT("The path in /Config/Space3DDataDir.txt is incorrect. This must point to the Space3D data folder (containing materials.cfg, hrtfs.cfg, and other required data files)."), *DataDir);
      DataDir = "";
    }
  }

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
  //FString BaseDir = IPluginManager::Get().FindPlugin("Space3DUnreal")->GetBaseDir();
	LibraryPath = TEXT("Space3DDyn.dll"); //FPaths::Combine(*BaseDir, TEXT("Space3DDyn.dll"));
#endif

	S3DLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if(!S3DLibraryHandle)
	{
    UE_LOG(LogSpace3DUnreal, Error, TEXT("Space3D failed to initialize"));
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InitFailure", "Failed to load Space3D dynamic library. Crashing will commence in 3... 2... 1..."), nullptr);
    return;
  }
  
  UE_LOG(LogSpace3DUnreal, Log, TEXT("Space3D starting up."));
  
  Space3D::RegisterErrorHandler(Space3DUnreal::ErrHandler);
  Space3D::RegisterMessageHandler(Space3DUnreal::MsgHandler);
  Space3D::IgnoreAPIErrors(true);
  Space3D::Init(GPUToUse, TCHAR_TO_UTF8(*DataDir), true);
  
  Space3D::CoordinateSystem(Space3D::CoordAxis::PosY, Space3D::CoordAxis::PosX, Space3D::CoordAxis::PosZ);
  Space3D::OutputChannelsSet(AUDIO_MIXER_MAX_OUTPUT_CHANNELS);
  
  Space3DUnreal::NumBuffersAvailable.store(0);
  
  Space3DUnreal::SourceFactory = new FSpace3DUnrealSourceFactory();
  IModularFeatures::Get().RegisterModularFeature(Space3DUnreal::SourceFactory->GetModularFeatureName(), Space3DUnreal::SourceFactory);
  #if 0
  Space3DUnreal::RegisterAssetTools();
  #endif
 
  // Start Viewer
  Space3DUnreal::ViewerKeepRunningFlag.test_and_set();
  Space3DUnreal::ViewerThread = new std::thread(Space3DUnreal::ViewerThreadRun);
  
  Space3DUnreal::Active = true;
  //FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InitSuccess", "Successfully initialized Space3D"), nullptr);
  UE_LOG(LogSpace3DUnreal, Display, TEXT("Space3D initialized"));
	
}

void FSpace3DUnrealModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

  if(!S3DLibraryHandle) return;
  Space3DUnreal::Active = false;
  
  // Close Viewer
  if (Space3DUnreal::ViewerThread != nullptr)
  {
	  Space3DUnreal::ViewerKeepRunningFlag.clear();
	  Space3DUnreal::ViewerThread->join();
	  delete Space3DUnreal::ViewerThread;
	  Space3DUnreal::ViewerThread = nullptr;
  }
  
  #if 0
  Space3DUnreal::UnregisterAssetTools();
  #endif
  IModularFeatures::Get().UnregisterModularFeature(Space3DUnreal::SourceFactory->GetModularFeatureName(), Space3DUnreal::SourceFactory);
  delete Space3DUnreal::SourceFactory; Space3DUnreal::SourceFactory = nullptr;
  
  Space3D::Finalize();
  UE_LOG(LogSpace3DUnreal, Display, TEXT("Space3D finalized"));
  //FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FinalizeSuccess", "Successfully finalized Space3D"), nullptr);
  // Free the dll handle
  FPlatformProcess::FreeDllHandle(S3DLibraryHandle);
  S3DLibraryHandle = nullptr;
}

#define S3DUNREALCOMPONENTBUG UE_LOG(LogSpace3DUnreal, Error, TEXT("Improper use of USpace3DUnrealComponent / bug"))

USpace3DUnrealComponent::USpace3DUnrealComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), uuid(0), IntentionallyNotCreated(false), LastT(0)
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.SetTickFunctionEnable(true);
}

USpace3DUnrealComponent::~USpace3DUnrealComponent() {}
  
void USpace3DUnrealComponent::CreateS3DObject()
{
  S3DUNREALCOMPONENTBUG;
}

void USpace3DUnrealComponent::DestroyS3DObject()
{
  S3DUNREALCOMPONENTBUG;
}

bool USpace3DUnrealComponent::RequiresScaleScale()
{
    return false;
}

void USpace3DUnrealComponent::PreTick() {} //optional to override

void USpace3DUnrealComponent::UpdateS3DProps()
{
  S3DUNREALCOMPONENTBUG;
}

void USpace3DUnrealComponent::OnRegister()
{
  USceneComponent::OnRegister();
  if(GetWorld()->IsGameWorld())
  {
    CreateS3DObject();
  }
  else
  {
    UE_LOG(LogSpace3DUnreal, Display, TEXT("Not creating %s because editor world"), *(GetClass()->GetName()));
    IntentionallyNotCreated = true;
  }
}

void USpace3DUnrealComponent::OnUnregister()
{
  USceneComponent::OnUnregister();
  if(!IntentionallyNotCreated)
  {
    DestroyS3DObject();
  }
  uuid = 0;
  LastT = 0;
  IntentionallyNotCreated = false;
}

void USpace3DUnrealComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
  if(IntentionallyNotCreated) return;
  PreTick();
  if(IntentionallyNotCreated) return;
  
  if(uuid == 0 || !Space3D::DoesObjectExist(uuid))
  {
    S3DUNREALCOMPONENTBUG;
    return;
  }
  
  if(DeltaTime == 0.0f)
  {
    UE_LOG(LogSpace3DUnreal, Warning, TEXT("TickComponent DeltaTime = 0"));
  }
  else if(DeltaTime < 0.0f)
  {
    UE_LOG(LogSpace3DUnreal, Warning, TEXT("TickComponent DeltaTime < 0, = %f"), DeltaTime);
  }
  
  uint64_t EstT = LastT + (uint64_t)((double)DeltaTime * 1000000000.0);
  uint64_t NsPerFrame = (Space3D::FrameLength() * 1000000000ull) / (uint64_t)Space3D::GetParams()->fs;
  uint64_t AudioT = Space3DUnreal::GetAudioT() + NsPerFrame;
  //UE_LOG(LogSpace3DUnreal, Display, TEXT("Delta %f Est %d Audio %d ms"), DeltaTime * 1000.0f, (EstT / 1000000ull), (AudioT / 1000000ull));
  
  bool bResync = true;
  uint64_t NsOffResync = NsPerFrame * 2 + NsPerFrame / 2; //2.5 frames off
  if(EstT > AudioT && EstT - AudioT > NsOffResync)
  {
    UE_LOG(LogSpace3DUnreal, Display, TEXT("Component %d ms ahead of audio, re-synchronizing"), (EstT - AudioT) / 1000000ull);
    LastT = AudioT;
  }
  else if(AudioT > EstT && AudioT - EstT > NsOffResync)
  {
    UE_LOG(LogSpace3DUnreal, Display, TEXT("Component %d ms behind audio, re-synchronizing"), (AudioT - EstT) / 1000000ull);
    LastT = AudioT;
  }
  else
  {
    LastT = EstT;
    bResync = false;
  }
  
  glm::vec3 P = Space3DUnreal::GetScaleFactor() * U2GV(GetComponentLocation());
  glm::quat R = U2GQ(GetComponentQuat());
  glm::vec3 S = (RequiresScaleScale() ? Space3DUnreal::GetScaleFactor() : 1.0f) * U2GV(GetComponentScale());
  if(bResync)
  {
    Space3D::PhysReset(uuid, LastT, P, R, S);
  }
  else
  {
    Space3D::PhysUpdate(uuid, LastT, P, R, S);
  }
  
  UpdateS3DProps();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpace3DUnrealModule, Space3DUnreal)
