#pragma once

#include "Space3DUnreal.h"

#include "CoreMinimal.h"
#include "Sound/SoundEffectSubmix.h"

#include "Space3DUnrealOutput.generated.h"

/** The output of Space3D, containing the simulated and spatialized audio from all the sources in the scene. This plugin ignores all incoming audio in the submix and sets the submix output audio to the rendered audio from Space3D. */
USTRUCT(BlueprintType)
struct SPACE3DUNREAL_API FSpace3DUnrealOutputSettings
{
  GENERATED_USTRUCT_BODY()
  
  /** The maximum number of bounces per simulated audio path. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0", ClampMax = "6"))
  int Order;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableOrder0;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableOrder1;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableOrder2;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableOrder3;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableOrder4;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableOrder5;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableOrder6;
  
  /** Target number of rays to hit each source. For ray tracing purposes, the source is sized so that roughly this number of rays should hit the source, regardless of distance from sinks or reflection order. E.g. 25.0f. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.01", ClampMax = "1000"))
  float SRaysTgt;

  /** "Maximum" distance from the source to any sink via first-order reflections. If the first-order reflection paths from all sinks to the source are longer than this, the source will start dropping in effective size and fewer rays will hit the source than specified by sarea_tgt. E.g. 8.5f in 3m diam small room, 400.0f in stadium. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.01", ClampMax = "10000"))
  float SDistMax;

  /** Per-order multiplier for source distance / reflection path length. This just multiplies sdist_max for higher order reflections, and should be set to an estimate of the ratio between average reflection path length of the different orders. E.g. if the average second-order reflection path is 1.5x the length of the average first-order path, sdist_mult[1] = 1.5f. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "10"))
  float SDistMult1;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "10"))
  float SDistMult2;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "10"))
  float SDistMult3;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "10"))
  float SDistMult4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "10"))
  float SDistMult5;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "10"))
  float SDistMult6;

  /** The distribution of rays coming from each sink is adjusted so that the irradiance on surfaces (for the first-order reflections) is roughly uniform. However, this is capped at a minimum and maximum distance from the source. E.g. 20 m. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "1000"))
  float RDSMaxdist;

  /** E.g. 0.2 m. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.01", ClampMax = "10"))
  float RDSMindist;

  /** Number of different rays to shoot from each sink for ray tracing specular path generation. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1000", ClampMax = "200000"))
  int RTRays;
  
  /**Number of branching paths each sink ray can turn into due to reflection/transmission. Must be a power of 2, suggested around 2^(order*1.5). Example values: 16 for low complexity, 256 for high. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "2048"))
  int RTBranches;

  /** Number of refinement steps for path generation. Default 2, suggested 1-3 */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "1", ClampMax = "3"))
  int GenRefineSteps;
  
  /** Enable SSNRD. If disabled, reflection paths will have a full frequency response, even off an edge.*/
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths)
  bool EnableSSNRD;

  /** The maximum reflection order that the SSNRD network will be applied to. For paths of higher reflection order than this, a cheaper and less accurate approximation will be applied instead.*/
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0", ClampMax = "6"))
  int SSNRDMaxNetOrder;

  /** Number of rings for diffraction spatial sampling (see paper). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Diffraction, meta = (ClampMin = "0", ClampMax = "15"))
  int DiffNRings;
  
  /** Number of angles on each ring for diffraction spatial sampling (see paper). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Diffraction, meta = (ClampMin = "0", ClampMax = "254"))
  int DiffNAngles;
  
  /** The frequency of the largest / lowest sound handled by the diffraction spatial sampling. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Diffraction, meta = (ClampMin = "4.0", ClampMax = "200.0"))
  float DiffBaseFreq;
  
  /** Whether changes in path lengths due to diffraction are simulated. Should be enabled for static scenes for accurate acoustical modeling, and disabled for dynamic scenes where sources / listener are continuously moving. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Diffraction)
  bool DiffLenChange;
  
  /** Degree of multipath complexity in diffraction spatial sampling (see paper). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Diffraction, meta = (ClampMin = "0", ClampMax = "2"))
  float DiffMultipath;

  /** Which algorithm to use for path merging or path synchronization. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0", ClampMax = "2"))
  int MergeAlg;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0", ClampMax = "2"))
  int SyncAlg;

  /** If two paths from the same sink to the same source have vertices which are closer together (in total) than this distance, they will be merged into one path. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float MergeSearchDist1;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float MergeSearchDist2;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float MergeSearchDist3;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "2.0"))
  float MergeSearchDist4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "4.0"))
  float MergeSearchDist5;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "8.0"))
  float MergeSearchDist6;

  /** Maximum distance a path can move between frames and still be considered as possibly the same path. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float SyncSearchDist1;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float SyncSearchDist2;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float SyncSearchDist3;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "2.0"))
  float SyncSearchDist4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "4.0"))
  float SyncSearchDist5;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Paths, meta = (ClampMin = "0.0", ClampMax = "8.0"))
  float SyncSearchDist6;
  
  /** Amount added to path length, so values don't blow up (and to provide better curves). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spatializer, meta = (ClampMin = "0.0001", ClampMax = "0.2"))
  float LambdaVolume;
  
  /** Amount added to path length, so values don't blow up (and to provide better curves). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spatializer, meta = (ClampMin = "0.0001", ClampMax = "0.2"))
  float LambdaDelay;
  
  /** Power law for volume computation. This is an enum, see Space3D.hpp. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spatializer, meta = (ClampMin = "0", ClampMax = "3"))
  int VolMode;
  
  /** Continuously adjustable volume power for power law. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spatializer, meta = (ClampMin = "0.5", ClampMax = "3.0"))
  float VolGamma;
  
  /** Space3D Proprietary && Confidential */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProprietaryAndConfidential, meta = (ClampMin = "0.0", ClampMax = "30.0"))
  float LongestDelay;

  /** Space3D Proprietary && Confidential */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProprietaryAndConfidential, meta = (ClampMin = "0.0", ClampMax = "30.0"))
  float HeadExtraDelay;
  
  /** Space3D Proprietary && Confidential */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ProprietaryAndConfidential, meta = (ClampMin = "0.0", ClampMax = "1.0"))
  float DelayChangeFactor;
  
  /** Scale factor from unreal units to meters. Normally UU = cm so this is 0.01. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Misc, meta = (ClampMin = "0.000001"))
  float UnrealUnitScaleFactor;
  
  /** Maximum time delay which can occur along any path, in units of frames. Paths whose length causes the audio delay along them to be longer than this value will be silenced. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Misc, meta = (ClampMin = "4", ClampMax = "512"))
  int MaxPathDelayFrames;

  /** Sets the current speaker arrangement mode. See SpkrArrangeMode. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Misc, meta = (ClampMin = "0", ClampMax = "3"))
  int SpeakersArrangeMode;
  
  
  FSpace3DUnrealOutputSettings()
    : Order(0)
    , EnableOrder0(true)
    , EnableOrder1(true)
    , EnableOrder2(true)
    , EnableOrder3(true)
    , EnableOrder4(true)
    , EnableOrder5(true)
    , EnableOrder6(true)
    , SRaysTgt(49.0f)
    , SDistMax(50.0f)
    , SDistMult1(1.0f)
    , SDistMult2(1.5f)
    , SDistMult3(2.0f)
    , SDistMult4(2.5f)
    , SDistMult5(3.0f)
    , SDistMult6(3.5f)
    , RDSMaxdist(20.0f)
    , RDSMindist(0.2f)
    , RTRays(80000)
    , RTBranches(32)
    , GenRefineSteps(2)
    , EnableSSNRD(true)
    , SSNRDMaxNetOrder(1)
    , DiffNRings(9)
    , DiffNAngles(32)
    , DiffBaseFreq(40.0f)
    , DiffLenChange(false)
    , DiffMultipath(0)
    , MergeAlg(1)
    , SyncAlg(1)
    , MergeSearchDist1(0.025)
    , MergeSearchDist2(0.05)
    , MergeSearchDist3(0.1)
    , MergeSearchDist4(0.2)
    , MergeSearchDist5(0.4)
    , MergeSearchDist6(0.8)
    , SyncSearchDist1(0.5)
    , SyncSearchDist2(0.8)
    , SyncSearchDist3(1.2)
    , SyncSearchDist4(1.7)
    , SyncSearchDist5(2.3)
    , SyncSearchDist6(3.0)
    , LambdaVolume(0.01f)
    , LambdaDelay(0.01f)
    , VolMode(3)
    , VolGamma(1.0f)
    , LongestDelay(10.0f)
    , HeadExtraDelay(10.0f)
    , DelayChangeFactor(0.5f)
    , UnrealUnitScaleFactor(0.01f)
    , MaxPathDelayFrames(256)
    , SpeakersArrangeMode(1)
    {}
};

class SPACE3DUNREAL_API FSpace3DUnrealOutput : public FSoundEffectSubmix
{
public:
  FSpace3DUnrealOutput();
  virtual ~FSpace3DUnrealOutput();

	virtual void Init(const FSoundEffectSubmixInitData& InitData) override;
	
	virtual void OnPresetChanged() override;

	virtual void OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData) override;
private:
	// TODO: What was the purpose of this?
	// static FSpace3DUnrealOutput* MainOutput;
	// bool CheckGrabMainOutput();
};

UCLASS()
class SPACE3DUNREAL_API USpace3DUnrealOutputPreset : public USoundEffectSubmixPreset
{
	GENERATED_BODY()

public:
	EFFECT_PRESET_METHODS(Space3DUnrealOutput)

	UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
	void SetSettings(const FSpace3DUnrealOutputSettings& InSettings);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SubmixEffectPreset)
	FSpace3DUnrealOutputSettings Settings;
};
