#pragma once
#include "Space3D.hpp"

/**
 * External API for Space3D Viewer.
 *
 * All functions in this API must be called from the same thread (because
 * OpenGL is not thread safe). Typical usage would be:
 *
 * [on some thread, can be the main thread]
 * Space3D::Viewer::Init();
 * while(!Space3D::Viewer::WantExit()){
 *     Space3D::Viewer::DrawScene();
 *     sleep(0.03); //or similar
 * }
 * Space3D::Viewer::Finalize();
 * [stop the thread]
*/

namespace Space3D { namespace Viewer
{
    ///Create viewer window.
    SPACE3D_API void Init(bool allowedits);
    ///Destroy window and release all resources.
    SPACE3D_API void Finalize();
    ///Returns true when user input requests the viewer window or
    ///application to close. Please call this in your thread's while loop.
    SPACE3D_API bool WantExit();

    ///Sets a callback function to be called when there is a key press.
    ///Many keys and combinations are already consumed by the viewer,
    ///so these will not be passed on to this function.
    ///May set nullptr to turn off callbacks.
    SPACE3D_API void SetKeyCallback(void (*keyCallback)(char key, bool ctrl, bool shift, bool alt));

    ///Draw the current scene state in the viewer window. Also handles input.
    SPACE3D_API void DrawScene();

    ///Only Poll
    SPACE3D_API void PollScene();
}}
