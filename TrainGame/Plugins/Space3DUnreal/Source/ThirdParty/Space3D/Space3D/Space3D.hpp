#pragma once

#include <string>

#define _USE_MATH_DEFINES //allows for M_PI
#include <math.h>

#define GLM_FORCE_CUDA 1
#define GLM_FORCE_QUAT_DATA_WXYZ 1
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if (defined(_WIN32) || defined(_WIN64)) && defined(SPACE3D_BUILD_DYNAMIC)
	#ifdef SPACE3D_BUILD_INTERNAL
		#define SPACE3D_API __declspec(dllexport)
	#else
		#define SPACE3D_API __declspec(dllimport)
	#endif
#else
	#define SPACE3D_API
#endif

//Can change this to double for double-precision audio computation
//(HIGHLY NOT RECOMMENDED AS THIS WILL RUIN CUDA PERFORMANCE)
//#define AUDIOFLOAT_IS_DOUBLE 1
#ifndef AUDIOFLOAT_IS_DOUBLE
#define AUDIOFLOAT_IS_FLOAT 1
typedef float audiofloat;
#else
typedef double audiofloat;
#endif

//Can change this as desired
#define SPEED_OF_SOUND 345.0f

//Don't change without checking all uses
#define MAX_REFLORDER 6

namespace Space3D
{
    //==========================================================================
    // Main
    //==========================================================================
    
    ///Call on startup. If datadir is specified, the material and HRTF config
    ///files are assumed to be in there. If they are not found there or it is
    ///not specified (empty string), Space3D will go hunting for a directory
    ///containing them (possibly called "data").
    SPACE3D_API void Init(int gpu, const char *datadir = "", bool logstdoutstderr = false);
    ///Call on shutdown
    SPACE3D_API void Finalize();
    /**
     * If you need to set several properties of the scene at once without the
     * risk of processing occurring between the changes, you can call these
     * functions before beginning and after ending your access. It is not
     * necessary to call these when using the API functions normally. Please do
     * not allow much time to elapse between calling BeginAtomicAccess() and
     * EndAtomicAccess() as this will stall all other operations!
    */
    SPACE3D_API void BeginAtomicAccess();
    SPACE3D_API void EndAtomicAccess();
    ///Convenience struct to lock the API & unlock it at end of scope with RAII.
    struct APILocker {
        inline APILocker() { BeginAtomicAccess(); }
        inline ~APILocker() { EndAtomicAccess(); }
    };
    #define SPACE3D_RAII_LOCK_API Space3D::APILocker api_locker
    
    ///By default, errors in external API calls (e.g. trying to delete an object
    ///which does not exist) will throw runtime errors and terminate the
    ///calling executable. Set this to true to instead continue as best as
    ///possible in case of errors, usually by ignoring the call.
    SPACE3D_API void IgnoreAPIErrors(bool ignore);
    ///Check if an object exists. Unlike all the other uuid-taking functions,
    ///returns false rather than throwing a runtime error if the object does
    ///not exist.
    SPACE3D_API bool DoesObjectExist(uint64_t uuid);
	
	///Registers a callback which will be called with the text of any error
	///message from Space3D. Set to nullptr to disable (disabled by default).
	SPACE3D_API void RegisterErrorHandler(void (*errhandler)(const char *msg));
	///Registers a callback which will be called with output / debug text from
	///Space3D. Note that this does not include output from GPU code and some
    ///types of debug messages. Set to nullptr to disable (disabled by default).
	SPACE3D_API void RegisterMessageHandler(void (*msghandler)(const char *msg));
	
	///Get current performance string. Write this string into the given buffer.
	SPACE3D_API void GetPerfString(char *buf, size_t bufsize);
    
    //==========================================================================
    // Audio I/O
    //==========================================================================
    
    ///Returns the current number of samples per frame.
    SPACE3D_API size_t FrameLength();
    ///Sets the number of samples that are processed per frame. For best
    ///performance, this should be a power of 2 or possibly some factors of 3.
    ///This may not be called while Process() (or the Space3DAudio system)
    ///are running.
    SPACE3D_API void SetFrameLength(size_t nsamples);
    ///The maximum time delay which can occur along any path, in units
    ///of frames. (Paths whose length causes the audio delay along them to
    ///be longer than this value will be silenced.)
    SPACE3D_API size_t MaxPathDelay();
    ///Set the maximum time delay per path (units of frames).
    SPACE3D_API void SetMaxPathDelay(size_t nframes);
    ///Returns the number of output channels. Since heads and speakers may have
    ///non-contiguous output channels, this is >=, but not necessarily = to,
    ///twice the number of heads or the number of speakers. For instance in a
    ///two-player VR scenario after player 0 quits, player 1's ears remain
    ///mapped to output channels 2 and 3.
    SPACE3D_API uint32_t OutputChannelCount();
    ///Sets the number of output channels. See OutputChannelCount().
    ///This may not be called while Process() (or the Space3DAudio system)
    ///are running.
    SPACE3D_API void OutputChannelsSet(uint32_t nchannels);
    
    /**
     * To use Space3D for audio, on each loop of your audio thread:
     * - Call SourceWrite() to provide one frame of input audio for each source
     * - Call Process() to run Space3D
     * - Call OutputChannelRead() to read one frame of processed audio for each
     *   output channel
    */
    
    ///Sets the audio played by this source during the coming frame. buf must
    ///be an array of FrameLength() audiofloats. buf is copied internally at
    ///call time and need not be kept valid after the function returns.
    SPACE3D_API void SourceWrite(uint64_t uuid, const audiofloat *buf);
    ///Run Space3D's acoustic modeling and spatialization algorithms.
    ///as_of_time is the time the audio frame started; see the Physics section
    ///below.
    SPACE3D_API void Process(uint64_t as_of_time);
    ///If there were no scene changes whatsoever (including movements, physics-
    ///simulated movements, or parameter changes), this function will process
    ///another audio buffer without recomputing the paths. This is only for use
    ///with impulse response or heatmap generation, or other applications where
    ///the scene is guaranteed to be completely fixed.
    SPACE3D_API void ProcessNoSceneChange();
    ///Get output audio processed during the previous frame. The number of
    ///samples written is FrameLength(). If o >= OutputChannelCount() or there
    ///is no head/speaker mapped to this output channel, the function fills
    ///buf_out with zeros.
    SPACE3D_API void OutputChannelRead(uint32_t o, audiofloat *buf_out);
    
    ///For debugging/performance purposes, get the total number of live paths
    ///which received audio simulation during the last frame.
    SPACE3D_API size_t NumLivePaths();
    
    //==========================================================================
    // Physics
    //==========================================================================
    
    enum class CoordAxis : uint8_t
    {
        PosX = 0, NegX = 1, PosY = 2, NegY = 3, PosZ = 4, NegZ = 5
    };
    ///Specify the coordinate system used by the external program.
    ///If not specified, the default is right=PosX, forward=PosY, up=PosZ.
    SPACE3D_API void CoordinateSystem(CoordAxis right, CoordAxis forward, CoordAxis up);
    
    /**
     * All Space3D physics functions, as well as Process(), require a uint64_t
     * time value in nanoseconds. The idea is that Process() and audio I/O
     * occur at one (typically faster) rate, and physics updates occur with
     * the external program's physics update rate (e.g. a game engine running
     * at 30 or 60 fps). This means that physics updates and Process() occur
     * in an interleaved manner--this is the reason for the whole physics system.
     * 
     * Each of these two processes should call their respective functions with
     * time values that represent these two rates. For instance, if the audio I/O
     * had 5 ms long frames, the Process calls should look like Process(0),
     * Process(5,000,000), Process(10,000,000), and so on; and if the game
     * physics was running at 30 fps, physics updates should look like
     * PhysUpdate(0), PhysUpdate(33,333,333), PhysUpdate(66,666,666), etc.
     * 
     * One option to achieve this effect is for both processes to call
     * Space3D::Time() below, which gives a nanoseconds count from std::chrono::
     * high_resolution_clock. They would call Time() at the beginning of each
     * respective type of frame, and then use the returned value for e.g. all
     * physics updates to all objects during that frame. Space3D::Viewer uses
     * this approach, as it is not intended to have a strictly uniform framerate.
     * However, this approach has the downside that any jitter in processing
     * times--or worse, on Windows, the fact that many frames of audio are 
     * queued up to be processed at a time--means that these variations will
     * translate into variations in the actual physics.
     * 
     * A better approach is to track the ideal times both physics and audio
     * would be processed at. For example, if audio callbacks are 5ms, start at
     * some value and increment as_of_time by 5,000,000 every time Process is
     * called. Similarly, increment as_of_time for the physics based on the
     * simulated (ideal) time between physics updates. These two sets of times
     * should be synchronized to each other at startup, and also re-synchronized
     * if they get too far away from each other, which can happen if there is
     * lag leading to dropped frames on the audio side (or of course, if there
     * is lag on the game engine side which isn't compensated by the game 
     * engine's physics system, such as when loading a new level).
     * 
     * If you're not using Space3D::AudioIO, there is no requirement to use this
	 * Time() function to get these nanosecond values, as Space3D does not call
	 * this internally. You could use a system clock for the initial
	 * synchronization, or just initialize to zero when the program starts.
	 * If you are using Space3D::AudioIO, that system calls Process() with
	 * as_of_time values derived from this Time() function, so you should
	 * synchronize to Time().
    */
    SPACE3D_API uint64_t Time();
    
    /**
     * Simulate and get the simulated transformation of an object at some time
     * near recent PhysUpdate times. Process() effectively uses this function
     * internally to obtain the state of an object. See PhysUpdate.
     */
    SPACE3D_API glm::mat4 TOf(uint64_t uuid, uint64_t as_of_time);
    SPACE3D_API glm::vec3 POf(uint64_t uuid, uint64_t as_of_time);
    SPACE3D_API glm::quat ROf(uint64_t uuid, uint64_t as_of_time);
    SPACE3D_API glm::vec3 SOf(uint64_t uuid, uint64_t as_of_time);
    
    /**
     * This function allows the game engine to tell Space3D what the
     * transformation for a particular object is at a particular time in terms
     * of the game engine's physics simulation. Audio time (as_of_time from
     * Process()) may be slightly ahead of or behind the latest value. The
     * purpose of this system is to handle this potentially chaotic interleaving
     * of times and allow the audio updates to be smooth and reflect the ideal
     * trajectory of the object.
     * 
     * Space3D stores the last few PhysUpdate spacetime points and interpolates
     * or extrapolates based on them for the as_of_time requested by Process()
     * or TOf(), POf(), ROf(), and SOf(). If this time is too far before or
     * after the recent PhysUpdate points--farther than, for example, 50 ms--
     * the results may be erratic.
     * 
     * While Process() as_of_time may go backwards in time relative to
     * PhysUpdate() as_of_time, PhysUpdate() as_of_time itself must increase
     * monotonically. If as_of_time < any previous as_of_time, or if as_of_time
     * == the most recent as_of_time and the difference in position / rotation
     * / scale is large, a warning will be thrown and a PhysReset will be done
     * based on the new values. If as_of_time == the most recent as_of_time and
     * the difference in value is small, the new value will silently replace
     * the most recent old value.
     * 
     * Scale values may be negative, but they may never switch from positive to
     * negative or vice versa. If you need to change their sign, re-initialize
     * them with that sign with PhysReset(). Scale values also may never be 0.
     * 
     * Transformations are applied in the order scale, rotation, position. The
     * function parameters are in the opposite order because all objects have
     * position, and all objects that have scale also have rotation.
    */
    SPACE3D_API void PhysUpdate(uint64_t uuid, uint64_t as_of_time,
        const glm::vec3 &P,  
		const glm::quat &R = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
		const glm::vec3 &S = glm::vec3(1.0f, 1.0f, 1.0f));
    
    /**
     * Resets the physics state for a given object, so that there is only one
     * stored spacetime point for it (and the object is therefore stationary).
     * See PhysUpdate.
    */
    SPACE3D_API void PhysReset(uint64_t uuid, uint64_t as_of_time,
        const glm::vec3 &P,  
		const glm::quat &R = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
		const glm::vec3 &S = glm::vec3(1.0f, 1.0f, 1.0f));
    
    /**
     * Another option is to perform physics calculations on the audio thread.
     * The external program may register a callback here, and do all calculations
     * relating to physics updates during that callback. This ensures audio is
     * perfectly synchronized to the physics. However, of course, the callback
     * must not take very much time to complete its physics update, or else there
     * will be less time to compute audio.
     *
     * If this is used, call PhysReset() to set the resulting states.
     * 
     * Set this to nullptr to disable the callback (it is disabled by default).
    */
    SPACE3D_API void PhysRegisterCallback(void (*callback)());
    
    //==========================================================================
    // Meshes
    //==========================================================================

    ///Returns the number of meshes in the scene.
    SPACE3D_API size_t MeshCount();
    
    ///Returns the uuid of a mesh of a given index. Note that indices change when
    ///other meshes are added or removed.
    SPACE3D_API uint64_t MeshByIndex(size_t m);
    ///Returns the index of a mesh by uuid. Note that indices change when other
    ///meshes are added or removed.
    SPACE3D_API size_t MeshIndexOf(uint64_t uuid);
    
    ///Adds a mesh to the scene and returns its uuid. Mesh vertex positions and
    ///materials may change later, but the number of vertices and the indices
    ///may not. The vertex and material data are initialized to zero, so you
    ///should lock the API before this call and set the vertex data before
    ///unlocking, to avoid processing a mesh where all the triangles are at the
    ///origin. Meshes are made of triangles only (no quads etc.), so the length
	///of the indices array is 3 * numTriangles.
    SPACE3D_API uint64_t MeshAdd(size_t numVerts, size_t numTriangles,
        const uint32_t *indices);
    ///Removes a mesh from the scene.
    SPACE3D_API void MeshRemove(uint64_t uuid);
    
    ///Returns the number of vertices in a given mesh.
    SPACE3D_API size_t MeshVertexCountOf(uint64_t uuid);
    ///Returns the number of triangles in a given mesh.
    SPACE3D_API size_t MeshTriangleCountOf(uint64_t uuid);
    ///Returns the total number of triangles in the scene across all meshes.
    SPACE3D_API size_t MeshTotalTriangleCount();
    
    ///Sets vertex data for a mesh, at initialization or for mesh deformations.
    ///The number of vertices cannot ever be changed.
    ///The normals are not directly used by Space3D, but they are used to
    ///determine if split vertices are representing sharp edges, or whether they
    ///are split for some other reason (e.g. seam in UV map). normals may be set
    ///to nullptr, in which case it will be assumed that all split vertices are
    ///sharp edges. If normals are not length 1 (to reasonable floating-point
    ///precision), a warning will be given.
    ///identityChange must be set if the mesh topology must be recomputed. This
    ///occurs when vertices which were previously in the same location (i.e.
    ///split verts) may have moved to different locations from each other, or
    ///vice versa; or when normals changed in such a way that edges may change
    ///between smooth and sharp. For most cases, e.g. typical deformations due
    ///to skeletal animation, this should be disabled.
    SPACE3D_API void MeshSetVertices(uint64_t uuid, const glm::vec3 *verts,
        const glm::vec3 *normals, bool identityChange);
    ///Set material data for a mesh. There is one material per vertex; materials
    ///are a property of vertices, not of triangles. Each triangle interpolates
    ///between the materials at its three vertices.
    SPACE3D_API void MeshSetMaterials(uint64_t uuid, const uint8_t *matls);
    ///Sets all the vertices of a given mesh to a given material.
    SPACE3D_API void MeshSetMaterial(uint64_t uuid, uint8_t matl);
    
    //==========================================================================
    // Materials
    //==========================================================================
    
    ///Set up a new material at the given index. This will overwrite any
    ///existing material at this index.
    ///Returns false if parameters (especially the impulse response file) are
    ///invalid. See more details in db/Materials.hpp.
    SPACE3D_API bool MaterialSetUp(uint8_t matl, uint8_t r, uint8_t g, uint8_t b,
        int nxfs, const float *xfreqs, const float *xfacts, std::string irfile);
    
    //==========================================================================
    // Sources
    // Note that physics scale is ignored for sources.
    //==========================================================================
    
    ///Directionality algorithm type. Currently only used for sources, but may
    ///be used for mics/etc. in the future. Some directionality types use
    ///parameters p1 through p3.
    enum class DirType : int32_t {
        //Basic non-parametric responses.
        Omni = 0,          //equiv. CosineMix, p2 = 0.0f
        FigureEight = 1,   //equiv. CosineMix, p2 = 1.0f
        Cardioid = 2,      //equiv. CosineMix, p2 = 0.5f
        Hypercardioid = 3, //equiv. CosineMix, p2 = 0.75f
        Supercardioid = 4, //equiv. CosineMix, p2 = 0.625f
        //Basic parametric responses.
        CosineMix = 10, //Mix of Omni and FigureEight by p2
        //Scaled responses. Half-power beam width (diameter) in degrees in p1.
        ScaledFigureEight = 20,
        ScaledCardioid = 21,
        ScaledHypercardioid = 22,
        ScaledSupercardioid = 23,
        //Positive only responses. Negative response is set to 0.
        PosFigureEight = 30,
        PosHypercardioid = 31,
        PosSupercardioid = 32,
        PosCosineMix = 33,
        //Testing.
        SDYoid = 40,
    };
    
    ///Returns the number of audio sources in the scene.
    SPACE3D_API size_t SourceCount();
    
    ///Returns the uuid of a source of a given index. Note that indices change
    ///when other sources are added or removed.
    SPACE3D_API uint64_t SourceByIndex(size_t s);
    ///Returns the index of a source by uuid. Note that indices change when other
    ///sources are added or removed.
    SPACE3D_API size_t SourceIndexOf(uint64_t uuid);
    
    ///Adds a source and returns its uuid.
    SPACE3D_API uint64_t SourceAdd();
    ///Removes a source from the scene.
    SPACE3D_API void SourceRemove(uint64_t uuid);
    
    ///Returns the directionality type and optional parameters of a source.
    SPACE3D_API DirType SourceDirTypeOf(uint64_t uuid);
    SPACE3D_API float SourceDirP1Of(uint64_t uuid);
    SPACE3D_API float SourceDirP2Of(uint64_t uuid);
    SPACE3D_API float SourceDirP3Of(uint64_t uuid);
    ///Sets the directionality (type and optional parameters) of a source.
    SPACE3D_API void SourceSetDir(uint64_t uuid, DirType type, 
        float p1 = 0.0f, float p2 = 0.0f, float p3 = 0.0f);
    
    ///Returns the volume of a source.
    SPACE3D_API audiofloat SourceVolumeOf(uint64_t uuid);
    ///Sets the volume of a source.
    SPACE3D_API void SourceSetVolume(uint64_t uuid, audiofloat vol);
    
    ///Space3D <Proprietary && Confidential>
    SPACE3D_API void SourceSetThresholds(uint64_t uuid, float threshfull,
        float threshzero);
    
    //==========================================================================
    // Heads
    // Note that scale is ignored for heads. The head radius (distance between
    // each ear and the midpoint) is specified in the selected HRTF.
    //==========================================================================
    
    ///Returns the number of heads in the scene.
    SPACE3D_API size_t HeadCount();
    
    ///Returns the uuid of a head of a given index. Note that indices change
    ///when other heads are added or removed.
    SPACE3D_API uint64_t HeadByIndex(size_t h);
    ///Returns the index of a head by uuid. Note that indices change when other
    ///heads are added or removed.
    SPACE3D_API size_t HeadIndexOf(uint64_t uuid);
    
    ///Adds a head using the given HRTF index. The left and right ears of this
    ///head are output to channels out_channel and out_channel+1 respectively.
    ///It is an error to have output channels overlapping between heads.
    SPACE3D_API uint64_t HeadAdd(uint8_t hrtf_idx, uint32_t out_channel);
    ///Removes a head from the scene. This may change indices of other heads,
    ///but it does not change their output channel assignments.
    SPACE3D_API void HeadRemove(uint64_t uuid);
    
    ///Returns the HRTF index of a given head.
    SPACE3D_API uint8_t HeadHRTFOf(uint64_t uuid);
    ///Sets the HRTF index of a given head.
    SPACE3D_API void HeadSetHRTF(uint64_t uuid, uint8_t hrtf_idx);
    ///Returns the first (left ear) output channel of a given head.
    SPACE3D_API uint32_t HeadChannelOf(uint64_t uuid);
    ///Sets the first (left ear) output channel of a given head. It is an error
    ///to have output channels overlapping between heads.
    SPACE3D_API void HeadSetChannel(uint64_t uuid, uint32_t out_channel);
    ///Enables an unpleasant test sound in the output channels this head is
    ///assigned to for routing testing. The lower pitch is the left channel and
    ///the higher pitch is the right channel.
    SPACE3D_API void HeadTestSound(uint64_t uuid, bool enable);
    
    //==========================================================================
    // Mics
    // A mic is an audio sink which samples the soundfield at its location. It
    // represents an omnidirectional mic in the virtual environment. As such,
    // the mic's rotation and scale are ignored. It is equivalent to one of the
    // two ears from a head, with no HRTF applied.
    //==========================================================================
    
    ///Returns the number of mics in the scene.
    SPACE3D_API size_t MicCount();
    
    ///Returns the uuid of a mic of a given index. Note that indices change
    ///when other mics are added or removed.
    SPACE3D_API uint64_t MicByIndex(size_t m);
    ///Returns the index of a mic by uuid. Note that indices change when other
    ///mics are added or removed.
    SPACE3D_API size_t MicIndexOf(uint64_t uuid);
    
    ///Adds a mic connected to the given output channel.
    SPACE3D_API uint64_t MicAdd(uint32_t out_channel);
    ///Removes a mic from the scene. This may change indices of other mics,
    ///but it does not change their output channel assignments.
    SPACE3D_API void MicRemove(uint64_t uuid);
    
    ///Returns the output channel of a given mic.
    SPACE3D_API uint32_t MicChannelOf(uint64_t uuid);
    ///Sets the output channel of a given mic.
    SPACE3D_API void MicSetChannel(uint64_t uuid, uint32_t out_channel);
    ///Enables an unpleasant test sound in the output channel this mic is
    ///assigned to for routing testing.
    SPACE3D_API void MicTestSound(uint64_t uuid, bool enable);
    
    //==========================================================================
    // Speakers
    // Note that scale, and also currently rotation (though this may be changed
    // in the future), are ignored for speakers.
    //==========================================================================
    
    ///Returns the number of speakers in the scene.
    SPACE3D_API size_t SpeakerCount();

    ///Returns the uuid of a speaker of a given index. Note that indices change
    ///when other speakers are added or removed.
    SPACE3D_API uint64_t SpeakerByIndex(size_t s);
    ///Returns the index of a speaker by uuid. Note that indices change when
    ///other speakers are added or removed.
    SPACE3D_API size_t SpeakerIndexOf(uint64_t uuid);
    
    ///Adds a speaker to the scene, which will output to the given channel.
    ///It is an error to have two speakers set to the same output channel.
    SPACE3D_API uint64_t SpeakerAdd(uint32_t out_channel);
    ///Removes a speaker from the scene. This may change indices of other speakers,
    ///but it does not change their output channel assignments.
    SPACE3D_API void SpeakerRemove(uint64_t uuid);
    
    ///Returns the output channel of a given speaker.
    SPACE3D_API uint32_t SpeakerChannelOf(uint64_t uuid);
    ///Sets the output channel of a given speaker. It is an error to have two
    ///speakers set to the same output channel.
    SPACE3D_API void SpeakerSetChannel(uint64_t uuid, uint32_t out_channel);
    ///Enables an unpleasant test sound in the output channel this speaker is
    ///assigned to for routing testing.
    SPACE3D_API void SpeakerTestSound(uint64_t uuid, bool enable);
    
    enum class SpkrArrangeMode : int32_t {
        ///Speakers cover all directions around the listener. The listener is
        ///not allowed to move outside the convex hull of the speakers.
        ThreeD = 0,
        ///Speakers cover all directions around to the sides and above the
        ///listener, but not below. Sources which go below the listener will be
        ///gracefully faded out where there are no speakers to cover them.
        ///The listener must not approach the upper speakers of the dome
        ///(e.g. climb on a ladder), although the listener is allowed to move
        ///in Z somewhat.
        Dome = 1,
        ///Same as dome, except the listener may approach the upper speakers
        ///of the dome. However, the upper speakers must be at least as far
        ///from the center of the room as the wall / side speakers. That is,
        ///the dome must be a hemisphere or "higher" than a hemisphere.
        HighDome = 2,
        ///Speakers and listener will be in a 2D plane. In addition, speakers
        ///are allowed to not cover all directions around the listener in the
        ///plane (e.g. stereo or mono are OK).
        TwoD = 3
    };
    
    ///Gets the current speaker arrangement mode. See SpkrArrangeMode.
    SPACE3D_API SpkrArrangeMode SpeakersArrangeMode();
    ///Sets the current speaker arrangement mode. See SpkrArrangeMode.
    SPACE3D_API void SpeakersSetArrangeMode(SpkrArrangeMode mode);
    
    //==========================================================================
    // Listener (only affects speakers, independent of heads & mics)
    // Note that scale, and also currently rotation (though this may be changed
    // in the future), are ignored for the listener.
    //==========================================================================
    
    ///Returns the uuid of the listener.
    SPACE3D_API uint64_t Listener();
    
    //==========================================================================
    // Room (only affects speakers, independent of heads & mics)
    // Imagine that the listener, speakers, and all real-world objects in the
    // real-world room are back-mapped into the virtual scene. When you want to
    // move all of this relative to the virtual world, for instance to fly over
    // a virtual landscape, you can move/rotate/scale the room instead of
    // updating the listener and all speaker positions. The room is simply a
    // transformation applied to the listener and speakers before simulation.
    // This allows the listener and speakers to be permanently expressed relative
    // to the origin of the real-world room.
    //==========================================================================
    
    ///Returns the uuid of the room.
    SPACE3D_API uint64_t Room();
    
    //==========================================================================
    // Spatialization parameters
    //==========================================================================
    
    struct SpatParams {
        ///The sampling rate in Hz.
        float fs;
        
        ///The maximum number of bounces per simulated audio path.
        uint8_t order;
        ///A mask for which order paths are processed. E.g. 0b0101 means
        ///zeroth and second order paths are processed. Bits corresponding to
        ///path orders out of bounds based on order are ignored.
        uint32_t ordermask;
        
		///Target number of rays to hit each source. For ray tracing purposes,
		///the source is sized so that roughly this number of rays should hit
		///the source, regardless of distance from sinks or reflection order.
		///E.g. 25.0f.
		float srays_tgt;
		///"Maximum" distance from the source to any sink via first-order
		///reflections. If the first-order reflection paths from all sinks to
		///the source are longer than this, the source will start dropping in
		///effective size and fewer rays will hit the source than specified by
		///sarea_tgt. E.g. 8.5f in 3m diam small room, 400.0f in stadium.
		float sdist_max;
		///Per-order multiplier for source distance / reflection path length.
        ///This just multiplies sdist_max for higher order reflections, and
        ///should be set to an estimate of the ratio between average reflection
        ///path length of the different orders. E.g. if the average second-order
        ///reflection path is 1.5x the length of the average first-order path,
        ///sdist_mult[1] = 1.5f.
        float sdist_mult[MAX_REFLORDER];
		///The distribution of rays coming from each sink is adjusted so that
		///the irradiance on surfaces (for the first-order reflections) is
		///roughly uniform. However, this is capped at a minimum and maximum
		///distance from the source. E.g. 20 m.
		float rds_maxdist;
		///E.g. 0.2 m.
		float rds_mindist;
		///Number of different rays to shoot from each sink for ray tracing
		///specular path generation.
		///Example value: 20000
		uint32_t rt_rays;
		///Number of branching paths each sink ray can turn into due to
		///reflection/transmission. Must be a power of 2, suggested around
		///2^(order*1.5). Example values: 16 for low complexity, 256 for high.
		uint32_t rt_branches;
        ///Number of refinement steps for path generation. Suggest small number
		///around 2-4.
        uint32_t gen_refine_steps;
        
        ///Enable SSNRD. If disabled, reflection paths will have a full
        ///frequency response, even off an edge.
        bool enable_ssnrd;
        ///The maximum reflection order that the SSNRD network will be applied
        ///to. For paths of higher reflection order than this, a cheaper and
        ///less accurate approximation will be applied instead.
        uint8_t ssnrd_maxnetorder;
		///Maximum value for vdat_nrings.
#define VDAT_MAXRINGS 15
        ///Number of rings and angles for each ring for VDaT. Default 9/32.
        ///Set one of these to zero to disable VDaT.
		uint32_t vdat_nrings, vdat_nangles;
        ///Largest ring radius/frequency, where f = SPEED_OF_SOUND / r.
        ///Set either one with the other set to <= 0 and it will be set up
        ///automatically. Remaining rings will be each half the size.
		float vdat_base_r, vdat_base_f;
        ///Reciprocal of vdat_nangles, computed automatically.
		float vdat_nangles_rcp;
        
        ///Whether VDaT simulates changes in path length due to diffraction.
        ///Enabled produces more accurate results in static scenes, but suffers
        ///from discretization issues in dynamic scenes.
        bool vdat_difflenchange;
        ///Degree of multipath complexity in VDaT.
        ///0: only     K---+
        ///            .   |
        ///            .   |
        ///            .   |
        ///            S---+
        ///1: also     K---+ and     K    
        ///            .  /          .\   
        ///            . /           . \  
        ///            ./            .  \ 
        ///            S             S---+
        ///2: also     K    
        ///            .`\, 
        ///            .   >
        ///            .,/'
        ///            S    
        uint32_t vdat_multipath;
		
		///Which algorithm to use for path merging or path synchronization.
		enum class RadiusSearchAlg : uint8_t {
			BruteForce,
			SpatialQuery,
			RTCore
		};
		RadiusSearchAlg merge_alg;
		RadiusSearchAlg sync_alg;
        ///If two paths from the same sink to the same source have vertices
        ///which are closer together (in total) than this distance, they will be
        ///merged into one path.
        float merge_search_dist[MAX_REFLORDER];
		///Maximum distance a path can move between frames and still be
		///considered as possibly the same path.
		float sync_search_dist[MAX_REFLORDER];
		
        ///Amounts added to path length, so that values don't blow up (and to
        ///provide better curves).
        ///Suggested 1cm (0.01f).
        float lambda_volume;
        float lambda_delay;
        
        ///Power law for volume computation.
        enum class VolMode : uint32_t {
            Inverse,
            InverseSquare,
            DLogD, ///1/(d*log2(d+1))
            Gamma //adjustable
        };
        VolMode vol_mode;
        float gamma_vol;
        
        ///Space3D spatializer core parameters. <Proprietary && Confidential>
        float space3d_longestdelay;
        float space3d_headextradelay;
        float space3d_delaychangefactor;
    };
    
    /**
     * Must use BeginAtomicAccess() and EndAtomicAccess() (or SPACE3D_RAII_LOCK_API) 
     * when modifying this data structure! It's provided as direct access here
     * simply because otherwise it would be a large pile of getter and setter
     * functions.
     * 
     * Note that it is perfectly allowed to change these parameters, as well
     * as move/add/remove sources/meshes/etc., while Process() is occurring.
     * Changes will take effect on the next frame.
     * 
     * It is also safe to *read* any of these parameters without any locking.
     * Space3D itself never modifies any of them (except at init), Space3D::AudioIO
     * only modifies them when playback begins, and Space3D::Viewer only modifies
     * them when you tell it to via keyboard commands.
     * 
     * Example usage:
     * Space3D::BeginAtomicAccess();
     * Space3D::SpatParams *params = Space3D::GetParams();
     * params->order = 5;
     * params->ordermask = 0b100000; //fifth order only
     * Space3D::EndAtomicAccess();
     */
    SPACE3D_API SpatParams *GetParams();
    
}
