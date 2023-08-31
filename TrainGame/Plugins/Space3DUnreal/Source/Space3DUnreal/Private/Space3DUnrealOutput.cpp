#include "Space3DUnrealOutput.h"

#include "Space3D.hpp"

// FSpace3DUnrealOutput* FSpace3DUnrealOutput::MainOutput = nullptr;

FSpace3DUnrealOutput::FSpace3DUnrealOutput()
{
}


FSpace3DUnrealOutput::~FSpace3DUnrealOutput()
{
  if(!Space3DUnreal::IsActive()) return;
  SPACE3D_RAII_LOCK_API;
  // if(this == MainOutput) MainOutput = nullptr;
}

/*

bool FSpace3DUnrealOutput::CheckGrabMainOutput()
{
  SPACE3D_RAII_LOCK_API;
  if(MainOutput == nullptr) MainOutput = this;
  return MainOutput == this;
}

*/

void FSpace3DUnrealOutput::Init(const FSoundEffectSubmixInitData& InitData)
{
  check(Space3DUnreal::IsActive());
  // if(!CheckGrabMainOutput()) return;
  UE_LOG(LogSpace3DUnreal, Display, TEXT("FSpace3DUnrealOutput Init %f Hz"), InitData.SampleRate);
  Space3D::GetParams()->fs = InitData.SampleRate;
}

void FSpace3DUnrealOutput::OnPresetChanged()
{
  check(Space3DUnreal::IsActive());
  // if(!CheckGrabMainOutput()) return;
  USpace3DUnrealOutputPreset* Preset2 = CastChecked<USpace3DUnrealOutputPreset>(Preset);
  FSpace3DUnrealOutputSettings Settings = Preset2->GetSettings();
  
  if(Settings.MaxPathDelayFrames != Space3D::MaxPathDelay())
  {
    Space3D::SetMaxPathDelay(Settings.MaxPathDelayFrames);
  }
  SPACE3D_RAII_LOCK_API;
  Space3D::SpatParams* p = Space3D::GetParams();
  p->order = Settings.Order;
  //UE_LOG(LogSpace3DUnreal, Display, TEXT("Space3D Preset Changed. Order set to: %d"), Settings.Order);
  p->ordermask = (uint32)Settings.EnableOrder0
    | ((uint32)Settings.EnableOrder1 << 1)
    | ((uint32)Settings.EnableOrder2 << 2)
    | ((uint32)Settings.EnableOrder3 << 3)
    | ((uint32)Settings.EnableOrder4 << 4)
    | ((uint32)Settings.EnableOrder5 << 5)
    | ((uint32)Settings.EnableOrder6 << 6);
  p->srays_tgt = Settings.SRaysTgt;
  p->sdist_max = Settings.SDistMax;
  p->sdist_mult[0] = Settings.SDistMult1;
  p->sdist_mult[1] = Settings.SDistMult2;
  p->sdist_mult[2] = Settings.SDistMult3;
  p->sdist_mult[3] = Settings.SDistMult4;
  p->sdist_mult[4] = Settings.SDistMult5;
  p->sdist_mult[5] = Settings.SDistMult6;
  p->rds_maxdist = Settings.RDSMaxdist;
  p->rds_mindist = Settings.RDSMindist;
  p->rt_rays = Settings.RTRays;
  p->rt_branches = Settings.RTBranches;
  p->gen_refine_steps = Settings.GenRefineSteps;
  p->enable_ssnrd = Settings.EnableSSNRD;
  p->ssnrd_maxnetorder = Settings.SSNRDMaxNetOrder;
  p->vdat_nrings = Settings.DiffNRings;
  p->vdat_nangles = Settings.DiffNAngles;
  p->vdat_base_r = -1.0f;
  p->vdat_base_f = Settings.DiffBaseFreq;
  p->vdat_difflenchange = Settings.DiffLenChange;
  p->vdat_multipath = Settings.DiffMultipath;
  p->merge_alg = static_cast<Space3D::SpatParams::RadiusSearchAlg>(Settings.MergeAlg);
  p->sync_alg = static_cast<Space3D::SpatParams::RadiusSearchAlg>(Settings.SyncAlg);
  p->merge_search_dist[0] = Settings.MergeSearchDist1;
  p->merge_search_dist[1] = Settings.MergeSearchDist2;
  p->merge_search_dist[2] = Settings.MergeSearchDist3;
  p->merge_search_dist[3] = Settings.MergeSearchDist4;
  p->merge_search_dist[4] = Settings.MergeSearchDist5;
  p->merge_search_dist[5] = Settings.MergeSearchDist6;
  p->sync_search_dist[0] = Settings.SyncSearchDist1;
  p->sync_search_dist[1] = Settings.SyncSearchDist2;
  p->sync_search_dist[2] = Settings.SyncSearchDist3;
  p->sync_search_dist[3] = Settings.SyncSearchDist4;
  p->sync_search_dist[4] = Settings.SyncSearchDist5;
  p->sync_search_dist[5] = Settings.SyncSearchDist6;
  p->lambda_volume = Settings.LambdaVolume;
  p->lambda_delay = Settings.LambdaDelay;
  p->vol_mode = static_cast<Space3D::SpatParams::VolMode>(Settings.VolMode);
  p->gamma_vol = Settings.VolGamma;
  p->space3d_longestdelay = Settings.LongestDelay;
  p->space3d_headextradelay = Settings.HeadExtraDelay;
  p->space3d_delaychangefactor = Settings.DelayChangeFactor;
  Space3D::SpeakersSetArrangeMode(static_cast<Space3D::SpkrArrangeMode>(Settings.SpeakersArrangeMode));
}

void FSpace3DUnrealOutput::OnProcessAudio(const FSoundEffectSubmixInputData& InData, FSoundEffectSubmixOutputData& OutData)
{
  check(Space3DUnreal::IsActive());
  bool NoOutput = false;

  
  /*
  if(!CheckGrabMainOutput())
  {
    NoOutput = true;
  }


  else
  */
  {
    uint32_t NBuffers = Space3DUnreal::GetAndConsumeOutputAudio();
    if(NBuffers > 1)
    {
      UE_LOG(LogSpace3DUnreal, Display, TEXT("Process called %d times before output. This is normal once when starting up, otherwise an issue."), NBuffers);
    }
    else if(NBuffers == 0)
    {
      NoOutput = true;
    }
    if(Space3D::FrameLength() != InData.NumFrames)
    {
      UE_LOG(LogSpace3DUnreal, Display, TEXT("Output wrong frame length %d"), InData.NumFrames);
      NoOutput = true;
    }
  }

  if(NoOutput)
  {
    //Clear output audio
    for(int32 i=0; i<OutData.AudioBuffer->Num(); ++i)
    {
      (*OutData.AudioBuffer)[i] = 0.0f;
    }
    return;
  }
  
  check(OutData.AudioBuffer->Num() == InData.NumFrames * OutData.NumChannels);
  check(OutData.NumChannels <= (int32)Space3D::OutputChannelCount());
  float* Temp = new float[InData.NumFrames];
  for(int32 c=0; c<OutData.NumChannels; ++c)
  {
    Space3D::OutputChannelRead(c, (audiofloat*)Temp);
    for(int32 s=0; s<InData.NumFrames; ++s)
    {
      (*OutData.AudioBuffer)[s*OutData.NumChannels+c] = Temp[s];
    }
  }
  delete[] Temp;
}

void USpace3DUnrealOutputPreset::SetSettings(const FSpace3DUnrealOutputSettings& InSettings)
{
    UpdateSettings(InSettings);
}
