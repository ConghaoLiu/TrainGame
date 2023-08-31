#include "Space3DUnrealBlueprint.h"
#include "Space3D.hpp"

void UpdateParams::UpdateOrder(int order)
{
    Space3D::SpatParams* p = Space3D::GetParams();
    p->order = order;
}

void UpdateParams::UpdateRTRays(int rt_rays)
{
    Space3D::SpatParams* p = Space3D::GetParams();
    p->rt_rays = rt_rays;
}

void UpdateParams::UpdateRTBranches(int rt_branches)
{
    Space3D::SpatParams* p = Space3D::GetParams();
    p->rt_branches = rt_branches;
}

void UpdateParams::UpdateDiffNAngles(int n_angles) 
{
    Space3D::SpatParams* p = Space3D::GetParams();
    p->vdat_nangles = n_angles;
}

void UpdateParams::UpdateDefGamma(float gamma)
{
    Space3D::SpatParams* p = Space3D::GetParams();
    p->gamma_vol = gamma;
}
void UpdateParams::UpdateSSNRDEnabled(bool enabled) {
    Space3D::SpatParams* p = Space3D::GetParams();
    p->enable_ssnrd = enabled;
}

void UViewerBP::EnableViewer(bool enable)
{
  if(enable && Space3DUnreal::ViewerThread==nullptr)
  {
    Space3DUnreal::ViewerKeepRunningFlag.test_and_set();
    Space3DUnreal::ViewerThread = new std::thread(Space3DUnreal::ViewerThreadRun);
  }
  else if(!enable && Space3DUnreal::ViewerThread!=nullptr)
  {
    Space3DUnreal::ViewerKeepRunningFlag.clear();
    Space3DUnreal::ViewerThread->join();
    delete Space3DUnreal::ViewerThread;
    Space3DUnreal::ViewerThread = nullptr;
  }
}

FString UViewerBP::GetPerformance()
{
  char performance_string[MAX_PERFSTRINGSIZE] = {};
  Space3D::GetPerfString(performance_string, MAX_PERFSTRINGSIZE);
  
  return FString(MAX_PERFSTRINGSIZE, performance_string);
}
