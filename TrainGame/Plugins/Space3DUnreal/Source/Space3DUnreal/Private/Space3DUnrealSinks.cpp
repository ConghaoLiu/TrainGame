#include "Space3DUnrealSinks.h"
#include "Space3D.hpp"

USpace3DUnrealHead::USpace3DUnrealHead(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  , HRTF(0)
  , OutputChannel(0)
  , TestSound(false)
  {}
  
void USpace3DUnrealHead::CreateS3DObject()
{
  uuid = Space3D::HeadAdd(HRTF, OutputChannel);
}

void USpace3DUnrealHead::DestroyS3DObject()
{
  Space3D::HeadRemove(uuid);
}

void USpace3DUnrealHead::UpdateS3DProps()
{
  Space3D::HeadSetHRTF(uuid, HRTF);
  Space3D::HeadSetChannel(uuid, OutputChannel);
  Space3D::HeadTestSound(uuid, TestSound);
}

USpace3DUnrealMic::USpace3DUnrealMic(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  , OutputChannel(0)
  , TestSound(false)
  {}
  
void USpace3DUnrealMic::CreateS3DObject()
{
  uuid = Space3D::MicAdd(OutputChannel);
}

void USpace3DUnrealMic::DestroyS3DObject()
{
  Space3D::MicRemove(uuid);
}

void USpace3DUnrealMic::UpdateS3DProps()
{
  Space3D::MicSetChannel(uuid, OutputChannel);
  Space3D::MicTestSound(uuid, TestSound);
}

USpace3DUnrealSpeaker::USpace3DUnrealSpeaker(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  , OutputChannel(0)
  , TestSound(false)
  {}
  
void USpace3DUnrealSpeaker::CreateS3DObject()
{
  uuid = Space3D::SpeakerAdd(OutputChannel);
}

void USpace3DUnrealSpeaker::DestroyS3DObject()
{
  Space3D::SpeakerRemove(uuid);
}

void USpace3DUnrealSpeaker::UpdateS3DProps()
{
  Space3D::SpeakerSetChannel(uuid, OutputChannel);
  Space3D::SpeakerTestSound(uuid, TestSound);
}

USpace3DUnrealListener::USpace3DUnrealListener(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  {}
  
void USpace3DUnrealListener::CreateS3DObject()
{
  uuid = Space3D::Listener();
}

void USpace3DUnrealListener::DestroyS3DObject() {}

void USpace3DUnrealListener::UpdateS3DProps() {}

USpace3DUnrealRoom::USpace3DUnrealRoom(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
  {}
  
void USpace3DUnrealRoom::CreateS3DObject()
{
  uuid = Space3D::Room();
}

void USpace3DUnrealRoom::DestroyS3DObject() {}

void USpace3DUnrealRoom::UpdateS3DProps() {}
