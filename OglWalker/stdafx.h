// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define GLEW_STATIC

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include <GL/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <math.h>
#include <dinput.h>
#include <float.h>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include "I3DObject.h"
#include "Triangle.h"
#include "CubeObject.h"
#include "GeometryOperations.h"
