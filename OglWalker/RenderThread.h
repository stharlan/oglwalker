#pragma once

typedef struct {
	HANDLE hQuitEvent;
	HWND hWnd;
	BOOL useDI;
	const char* InputFilename;
} RENDER_THREAD_CONTEXT;

typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> GPointModel;
typedef boost::geometry::model::box<GPointModel> GBoxModel;
typedef boost::geometry::model::segment<GPointModel> GSegmentModel;
typedef std::pair<GBoxModel, unsigned> GValueModel;

DWORD WINAPI RenderingThreadOneEntryPoint(void* pVoid);
DWORD WINAPI RenderingThreadTwoEntryPoint(void* pVoid);
DWORD WINAPI RenderingThreadThreeEntryPoint(void* pVoid);