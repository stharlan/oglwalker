// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define GLEW_STATIC
#define MAX_LOADSTRING 100
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)
#define NO_TRIANGLE_FOUND UINT_MAX
#define GLM_ENABLE_EXPERIMENTAL

// Windows Header Files:
#include <windows.h>

#define CUSTOM_QUIT (WM_USER + 1)

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include <stdio.h>
#include <math.h>
#include <dinput.h>
#include <float.h>
#include <vector>
#include <iostream>

// opengl
#include <GL/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>

// for rtree index
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>

// vectors and matrices
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Magick++.h>

#include <rapidjson/document.h>

#include "RenderThreadCommon.h"
#include "RenderThread.h"

#include "I3DObject.h"
#include "Triangle.h"
#include "CubeObject.h"
#include "GeometryOperations.h"
