#pragma once

#include "Space3DUnreal.h"

#include "Space3DUnrealSinks.generated.h"

/** A person's head with two ears, for HRTF based binaural rendering. */
UCLASS(BlueprintType, ClassGroup=Audio, EditInlineNew, meta=(BlueprintSpawnableComponent))
class SPACE3DUNREAL_API USpace3DUnrealHead : public USpace3DUnrealComponent
{
  GENERATED_BODY()
  
public:
  /** HRTF index of this head. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "255"))
  int HRTF;
  
  /** First (left ear) output channel of this head. Right ear is always the next one after this. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "30"))
  int OutputChannel;
  
  /** Enables an unpleasant test sound in the output channels this head is assigned to for routing testing. The lower pitch is the left channel and the higher pitch is the right channel. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool TestSound;
  
  USpace3DUnrealHead(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  
  virtual void CreateS3DObject() override;
  virtual void DestroyS3DObject() override;
  virtual void UpdateS3DProps() override;
};

/** A mono, omnidirectional microphone in the virtual environment. */
UCLASS(BlueprintType, ClassGroup=Audio, EditInlineNew, meta=(BlueprintSpawnableComponent))
class SPACE3DUNREAL_API USpace3DUnrealMic : public USpace3DUnrealComponent
{
  GENERATED_BODY()
  
public:
  /** Output channel of this mic. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "31"))
  int OutputChannel;
  
  /** Enables an unpleasant test sound in the output channel this mic is assigned to for routing testing. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool TestSound;
  
  USpace3DUnrealMic(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  
  virtual void CreateS3DObject() override;
  virtual void DestroyS3DObject() override;
  virtual void UpdateS3DProps() override;
};

/**
A speaker in the user's physical room.

Set this component's position to the location of the physical speaker relative to the real-world room's origin. Don't move it during gameplay--unless someone physically picks up the speaker and moves it within the real-world room, which is supported by Space3D.

See Space3DUnrealListener and Space3DUnrealRoom to represent the types of motion that typically occur in a game.
*/
UCLASS(BlueprintType, ClassGroup=Audio, EditInlineNew, meta=(BlueprintSpawnableComponent))
class SPACE3DUNREAL_API USpace3DUnrealSpeaker : public USpace3DUnrealComponent
{
  GENERATED_BODY()
  
public:
  /** Output channel of this speaker. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "31"))
  int OutputChannel;
  
  /** Enables an unpleasant test sound in the output channel this speaker is assigned to for routing testing. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool TestSound;
  
  USpace3DUnrealSpeaker(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  
  virtual void CreateS3DObject() override;
  virtual void DestroyS3DObject() override;
  virtual void UpdateS3DProps() override;
};

/**
The listener or "sweet spot" in a room with speakers in it.

Set this component's position to the location of the human listener's head relative to the real-world room's origin. If the person moves around the real-world room (e.g. while wearing an HMD), update this position. However, if the person moves around the game world but doesn't physically move in the real world, don't modify this position; use Space3DUnrealRoom for that motion.

Only create one of these; if you create more than one in different positions, their physics updates will interfere, with unpredictable results.

This only affects the outputs from speakers (Space3DUnrealSpeaker); if you are only using heads and/or mics, there is no need to create one.
*/
UCLASS(BlueprintType, ClassGroup=Audio, EditInlineNew, meta=(BlueprintSpawnableComponent))
class SPACE3DUNREAL_API USpace3DUnrealListener : public USpace3DUnrealComponent
{
  GENERATED_BODY()
  
public:
  USpace3DUnrealListener(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  
  virtual void CreateS3DObject() override;
  virtual void DestroyS3DObject() override;
  virtual void UpdateS3DProps() override;
};

/**
The mapping of the physical room (containing listener and speakers) to the virtual world.

Set this component's transform to the location, rotation, and scale of the physical room relative to the virtual world. For example, to fly over a virtual landscape, move this component continuously, and don't move the listener or speakers.

Only create one of these; if you create more than one in different positions, their physics updates will interfere, with unpredictable results.

This only affects the outputs from speakers (Space3DUnrealSpeaker); if you are only using heads and/or mics, there is no need to create one.
*/
UCLASS(BlueprintType, ClassGroup=Audio, EditInlineNew, meta=(BlueprintSpawnableComponent))
class SPACE3DUNREAL_API USpace3DUnrealRoom : public USpace3DUnrealComponent
{
  GENERATED_BODY()
  
public:
  USpace3DUnrealRoom(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
  
  virtual void CreateS3DObject() override;
  virtual void DestroyS3DObject() override;
  virtual void UpdateS3DProps() override;
};
