// OglWalker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "OglWalker.h"

#define MAX_LOADSTRING 100
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)
#define CUSTOM_QUIT (WM_USER + 1)
#define NO_TRIANGLE_FOUND UINT_MAX

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

typedef struct {
	GLuint base;
	int widths[256];
	int height;
} GLFONT;

typedef struct {
	HANDLE hQuitEvent;
	HWND hWnd;
	BOOL useDI;
} RENDER_THREAD_CONTEXT;

RENDER_THREAD_CONTEXT g_ctx;

typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> GPointModel;
typedef boost::geometry::model::box<GPointModel> GBoxModel;
typedef boost::geometry::model::segment<GPointModel> GSegmentModel;
typedef std::pair<GBoxModel, unsigned> GValueModel;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void SetDCPixelFormat(HDC hdc);
void ChangeSize(GLsizei w, GLsizei h);
void FontPuts(GLFONT* font, const char* s);
void FontDestroy(GLFONT* font);
GLFONT* FontCreate(HDC hdc, const wchar_t* typeface, int height, int weight, DWORD italic);
void FontPrintf(GLFONT     *font,   /* I - Font to use */
	int        align,   /* I - Alignment to use */
	const char *format, /* I - printf() style format string */
	...);                /* I - Other arguments as necessary */
BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd);
void CleanupDirectInput();
bool ReadMouseState(DIMOUSESTATE *pMouseState);
bool ReadKeyboardState(unsigned char* keystate);

HANDLE hRenderingThread = NULL;
DWORD RenderThreadId = 0;
GLFONT* pFont;
FILE* g_log;

void SetupRC()
{
	pFont = FontCreate(wglGetCurrentDC(), L"Arial", 18, 0, 0);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POINT_SMOOTH);
	//glEnable(GL_POLYGON_SMOOTH);
	
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	//glCullFace(GL_CW);
	//glPointSize(20.0f);

	glEnable(GL_LIGHTING);
	GLfloat ambientLight[] = { 0.3f,0.3f,0.3f,1.0f };
	GLfloat diffuseLight[] = { 0.7f,0.7f,0.7f,1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glEnable(GL_LIGHT0);
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void SetPerspective(int w, int h)
{
	GLfloat fAspect;
	if (h == 0) h = 1;
	fAspect = (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, fAspect, 1.0, 400.0);
	glMatrixMode(GL_MODELVIEW);
}

void SetOrtho(int w, int h)
{
	if (h == 0) h = 1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (GLfloat)w, 0.0, (GLfloat)h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

/*
void RenderScene(int w, int h, float fps, float azimuth, float elevation,
	float px, float py, float pz, float ex, float ez, std::vector<Triangle> &AllTris,
	int u1, unsigned int u3,
	Point& lpt, unsigned int u5)
//void RenderScene(int w, int h, float fps, float azimuth, float elevation, float px, float pz)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetPerspective(w, h);
	
	// create a grid of triangles as a "floor"
	glPushMatrix();
	{
		glLoadIdentity();

		// direction look l/r
		glRotatef(azimuth, 0.0f, 1.0f, 0.0);

		float erad = DEG2RAD(azimuth);
		GLfloat eex = cosf(erad);
		GLfloat eez = sinf(erad);

		// elevation look u/d
		glRotatef(elevation, eex, 0.0f, eez);

		// position x/y
		glTranslatef(px, -1.0f * py, pz);

		// draw the grid (white)
		glColor3f(1.0f, 1.0f, 1.0f);
		glPolygonMode(GL_FRONT, GL_LINE);
		glBegin(GL_TRIANGLES);
		{
			glLineWidth(1.0f);
			for (std::vector<Triangle>::iterator iter = AllTris.begin(); iter != AllTris.end(); ++iter)
				iter->Draw();
		}
		glEnd();

		// draw some red triangles
		glColor3f(1.0f, 0.0f, 0.0f);
		glPolygonMode(GL_FRONT, GL_FILL);
		glBegin(GL_TRIANGLES);
		{
			unsigned int ctr = 0;
			for (std::vector<Triangle>::iterator iter = AllTris.begin(); iter != AllTris.end(); ++iter)
			{
				if (ctr == u3 || ctr == u5) iter->Draw();
				ctr++;
			}
		}
		glEnd();
		glBegin(GL_POINTS);
		glVertex3f(lpt.x, lpt.y, lpt.z);
		glEnd();

		// set back to white lines
		glColor3f(1.0f, 1.0f, 1.0f);
		glPolygonMode(GL_FRONT, GL_LINE);

		// draw a sphere
		glPushMatrix();
		{
			glTranslatef(-10.0f, 10.0f, -10.0f);
			GLUquadric* pquad = gluNewQuadric();
			gluSphere(pquad, 5, 10, 10);
			gluDeleteQuadric(pquad);
		}
		glPopMatrix();

	}
	glPopMatrix();

	// create a "crosshair" in the middle of the screen
	SetOrtho(w, h); 
	glColor3f(0.0f, 1.0f, 0.0f);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	{
		glVertex2i((w / 2) - 10, h / 2);
		glVertex2i((w / 2) + 10, h / 2);
		glVertex2i(w / 2, (h / 2) - 10);
		glVertex2i(w / 2, (h / 2) + 10);
	}
	glEnd();

	// draw some text on the screen
	// debugging messages - for now
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2i(4, 18);
	FontPrintf(pFont, 1, "hello world %i", (int)fps);

	SYSTEMTIME stime;
	GetSystemTime(&stime);
	glRasterPos2i(4, 36);
	FontPrintf(pFont, 1, "%02i:%02i:%02i\n", stime.wHour, stime.wMinute, stime.wSecond);

	glRasterPos2i((w / 2) + 10, (h / 2) + 10);
	FontPrintf(pFont, 1, "%.0f degrees", azimuth);

	glRasterPos2i((w / 2) + 10, (h / 2) + 24);
	FontPrintf(pFont, 1, "%.4f, %.4f, pct %i", ex, ez, u1);

	glFinish();

}
*/

void AddCubeTris(CubeObject& c1,
	std::vector<oglw::Triangle>& AllTris,
	boost::geometry::index::rtree< GValueModel, boost::geometry::index::quadratic<16> >& GSpatialIndex)
{
	for (std::vector<oglw::Triangle>::iterator iter = c1.tris.begin(); iter != c1.tris.end(); ++iter)
	{
		AllTris.push_back(*iter);
		GBoxModel b {
			{ iter->MinX(), iter->MinY(), iter->MinZ() },
			{ iter->MaxX(), iter->MaxY(), iter->MaxZ() }
		};
		GSpatialIndex.insert(std::make_pair(b, (unsigned)(AllTris.size() - 1)));
	}
}

#define ADDCUBE(a,b,c,d,e,f) AddCubeTris(CubeObject(a,b,c,d,e,f), AllTris, GSpatialIndex);

void AddSomeStuff(std::vector<oglw::Triangle>& AllTris,
	boost::geometry::index::rtree< GValueModel, boost::geometry::index::quadratic<16> >& GSpatialIndex)
{
	// floor
	ADDCUBE(-50, -1, -50, 100, 1, 100);

	// walls
	ADDCUBE(-51, -1, -51, 101, 11, 1);
	ADDCUBE(-51, -1, 50, 101, 11, 1);
	ADDCUBE(50, -1, -51, 1, 11, 102);
	ADDCUBE(-51, -1, -50, 1, 11, 100);

	ADDCUBE(-50, 0, -50, 20, 1, 20);
	ADDCUBE(-50, 1, -50, 10, 1, 10);
	ADDCUBE(-50, 2, -50, 5, 1, 5);

	ADDCUBE(-20, 0, 20, 5, 2, 5);
	ADDCUBE(-25, 0, 20, 5, 3, 5);
	ADDCUBE(-30, 0, 20, 5, 4, 5);

	ADDCUBE(10, 0, 10, 10, 10, 1);
	ADDCUBE(10, 0, 20, 10, 10, 1);
	ADDCUBE(10, 10, 10, 10, 1, 11);
}

unsigned int FindClosestTriThatIntersectsLine(
	boost::geometry::index::rtree< GValueModel, boost::geometry::index::quadratic<16> >& GSpatialIndex,
	GSegmentModel& line, 
	std::vector<oglw::Triangle>& tris,
	glm::vec3& origin,
	glm::vec3& ray,
	glm::vec3& intersectionPoint,
	FILE* log)
{
	if (log != NULL) fprintf(log, "<= FindClosestTriThatIntersectsLine =>\n");

	// do the query
	std::vector<GValueModel> result_n;
	GSpatialIndex.query(boost::geometry::index::intersects(line), std::back_inserter(result_n));

	unsigned int indexOfClosestTri = NO_TRIANGLE_FOUND;
	float mindist = FLT_MAX;
	glm::vec3 pout;

	if (log != NULL) {
		fprintf(log, "origin %.1f, %.1f, %.1f\n", origin.x, origin.y, origin.z);
		fprintf(log, "ray %.1f, %.1f, %.1f\n", ray.x, ray.y, ray.z);
	}

	for (std::vector<GValueModel>::iterator iter = result_n.begin(); iter != result_n.end(); ++iter)
	{
		oglw::Triangle tri = tris.at(iter->second);
		if (log != NULL) {
			fprintf(log, "tri index = %i\n", iter->second);
			fprintf(log, "trip1 %.1f, %.1f, %.1f\n", tri.p1.x, tri.p1.y, tri.p1.z);
			fprintf(log, "trip2 %.1f, %.1f, %.1f\n", tri.p2.x, tri.p2.y, tri.p2.z);
			fprintf(log, "trip3 %.1f, %.1f, %.1f\n", tri.p3.x, tri.p3.y, tri.p3.z);
			fprintf(log, "boxmin %.1f, %.1f, %.1f\n",
				iter->first.min_corner().get<0>(),
				iter->first.min_corner().get<1>(),
				iter->first.min_corner().get<2>());
			fprintf(log, "boxmax %.1f, %.1f, %.1f\n",
				iter->first.max_corner().get<0>(),
				iter->first.max_corner().get<1>(),
				iter->first.max_corner().get<2>());
		}
		bool b = RayIntersectsTriangle(origin, ray, tri, pout);
		if (log != NULL) {
			fprintf(log, "result %i - pout %.1f, %.1f, %.1f\n", b, pout.x, pout.y, pout.z);
		}
		if (b) {
			//float dist = origin.Distance(pout);
			float dist = glm::distance(origin, pout);
			if (dist < mindist) {
				mindist = dist;
				indexOfClosestTri = iter->second;
				if (log != NULL) {
					fprintf(log, "dist = %.1f; mindist = %.1f; index = %i\n", dist, mindist, indexOfClosestTri);
				}
				intersectionPoint = pout;
			}
		}
	}

	return indexOfClosestTri;
}

void ProcessFloor(glm::vec3& p) {
	if (p.x >= 30.0f && p.z <= -30.0f) {
		p.y = 5.0f;
	}
	else if (p.x <= -30.0f && p.z >= 30.0f) {
		p.y = -5.0f;
	}
}

DWORD WINAPI RenderingThreadEntryPoint(void* pVoid) 
{
	RENDER_THREAD_CONTEXT* ctx = (RENDER_THREAD_CONTEXT*)pVoid;

	// create the rtree using default constructor
	boost::geometry::index::rtree< GValueModel, boost::geometry::index::quadratic<16> > GSpatialIndex;

	std::vector<oglw::Triangle> AllTris;

	// add a floor
	/*
	for (float x = -50.0f; x < 50.0f; x += 5.0f) {
		for (float z = -50.0f; z < 50.0f; z += 5.0f) {

			float height = 0;

			Point p1(x, height, z);
			Point p2(x + 5.0f, height, z + 5.0f);
			Point p3(x + 5.0f, height, z);
			ProcessFloor(p1);
			ProcessFloor(p2);
			ProcessFloor(p3);
			Triangle t1(p1, p2, p3);
			Point p1min = t1.MinBox();
			Point p1max = t1.MaxBox();
			AllTris.push_back(t1);

			box b1{ {p1min.x, p1min.y, p1min.z},{p1max.x, p1max.y, p1max.z} };
			rtree.insert(std::make_pair(b1, (unsigned)(AllTris.size() - 1)));

			Point p4(x, height, z);
			Point p5(x, height, z + 5.0f);
			Point p6(x + 5.0f, height, z + 5.0f);
			ProcessFloor(p4);
			ProcessFloor(p5);
			ProcessFloor(p6);
			Triangle t2(p4, p5, p6);
			Point p2min = t2.MinBox();
			Point p2max = t2.MaxBox();
			AllTris.push_back(t2);

			box b2{ { p2min.x, p2min.y, p2min.z },{ p2max.x, p2max.y, p2max.z } };
			rtree.insert(std::make_pair(b2, (unsigned)(AllTris.size() - 1)));

		}
	}
	*/

	AddSomeStuff(AllTris, GSpatialIndex);

	float EyeAzimuthInDegrees = 0.0f;
	float EyeElevationInDegrees = 0.0f;
	float px = 0.0f;
	float _py = 0.0f;
	float pz = 0.0f;
	float eyeHeight = 6.0f;
	float midHeight = 3.0f;
	float movementInTheX = 0.0f;
	float movementInTheY = 0.0f;
	float movementInTheZ = 0.0f;
	float proposedMovementInTheY = 0.0f;

	FILE* log = NULL;
	fopen_s(&log, "c:\\temp\\rt.log", "w");


	fprintf(log, "create rc\n");
	HDC hdc = GetDC(ctx->hWnd);
	SetDCPixelFormat(hdc);
	HGLRC hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	RECT rect;
	GetClientRect(ctx->hWnd, &rect);
	ChangeSize(rect.right, rect.bottom);

	SetupRC();

	LARGE_INTEGER perfCount;
	LARGE_INTEGER perfFreq;

	QueryPerformanceFrequency(&perfFreq);
	fprintf(log, "perf freq %lli\n", perfFreq.QuadPart);

	LONGLONG lastCount = 0;
	unsigned char keystate[256];
	DIMOUSESTATE mouseState;

	GLfloat ex = 0.0f;
	GLfloat ez = 0.0f;
	float MovementDirectionInDegrees = 0.0f;

	// main loop
	fprintf(log, "start loop\n");
	while (1) {

		// check state
		if (WaitForSingleObject(ctx->hQuitEvent, 0) == WAIT_OBJECT_0) {
			fprintf(log, "quit event is triggered\n");
			break;
		}

		// get a count and calculate fps
		QueryPerformanceCounter(&perfCount);
		float FramesPerSecond = (float)perfFreq.QuadPart / (float)(perfCount.QuadPart - lastCount);
		float WalkingStride = 25.0f / FramesPerSecond;
		lastCount = perfCount.QuadPart;

		movementInTheX = movementInTheZ = 0.0f;

		// find movement in the x and z directions
		// process direct input
		if (TRUE == ctx->useDI) {

			// mouse movement - look at and turning
			if (TRUE == ReadMouseState(&mouseState))
			{
				//fprintf(log, "mouse %i, %i, %i\n", mouseState.lX, mouseState.lY, mouseState.lZ);
				EyeAzimuthInDegrees += (mouseState.lX / 8.0f);
				if (EyeAzimuthInDegrees < 0.0f) EyeAzimuthInDegrees = EyeAzimuthInDegrees + 360.0f;
				if (EyeAzimuthInDegrees > 360.0f) EyeAzimuthInDegrees = EyeAzimuthInDegrees - 360.0f;

				EyeElevationInDegrees += (mouseState.lY / 8.0f);
				if (EyeElevationInDegrees < -90.0f) EyeElevationInDegrees = -90.0f;
				if (EyeElevationInDegrees > 90.0f) EyeElevationInDegrees = 90.0f;
			}

			// keyboard movement - move camera
			if (TRUE == ReadKeyboardState(&keystate[0]))
			{
				// quit
				fprintf(log, "esc = %i\n", keystate[DIK_ESCAPE]);
				if (keystate[DIK_ESCAPE] == (unsigned char)128) {
					PostMessage(ctx->hWnd, CUSTOM_QUIT, 0, 0);
				}

				ex = ez = 0.0f;
				float EyeAzimuthInRadians = 0.0f;
				
				// using standard wsad for movement
				// fwd bck strafe left and strafe right
				if (keystate[DIK_W] == (unsigned char)128) {
					if (keystate[DIK_A] == (unsigned char)128) {
						MovementDirectionInDegrees = EyeAzimuthInDegrees - 45.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						ex = sinf(EyeAzimuthInRadians);
						ez = cosf(EyeAzimuthInRadians);
					}
					else if (keystate[DIK_D] == (unsigned char)128) {
						MovementDirectionInDegrees = EyeAzimuthInDegrees + 45.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						ex = sinf(EyeAzimuthInRadians);
						ez = cosf(EyeAzimuthInRadians);
					}
					else {
						MovementDirectionInDegrees = EyeAzimuthInDegrees;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						ex = sinf(EyeAzimuthInRadians);
						ez = cosf(EyeAzimuthInRadians);
					}
				} else if (keystate[DIK_S] == (unsigned char)128) {
					if (keystate[DIK_A] == (unsigned char)128) {
						MovementDirectionInDegrees = EyeAzimuthInDegrees - 135.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						ex = sinf(EyeAzimuthInRadians);
						ez = cosf(EyeAzimuthInRadians);
					}
					else if (keystate[DIK_D] == (unsigned char)128) {
						MovementDirectionInDegrees = EyeAzimuthInDegrees + 135.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						ex = sinf(EyeAzimuthInRadians);
						ez = cosf(EyeAzimuthInRadians);
					}
					else {
						MovementDirectionInDegrees = EyeAzimuthInDegrees + 180.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						ex = sinf(EyeAzimuthInRadians);
						ez = cosf(EyeAzimuthInRadians);
					}
				} else if (keystate[DIK_A] == (unsigned char)128) {
					MovementDirectionInDegrees = EyeAzimuthInDegrees - 90.0f;
					EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
					ex = sinf(EyeAzimuthInRadians);
					ez = cosf(EyeAzimuthInRadians);
				} else if (keystate[DIK_D] == (unsigned char)128) {
					MovementDirectionInDegrees = EyeAzimuthInDegrees + 90.0f;
					EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
					ex = sinf(EyeAzimuthInRadians);
					ez = cosf(EyeAzimuthInRadians);
				}

				movementInTheX = -1.0 * WalkingStride * ex;
				movementInTheZ = WalkingStride * ez;

				px += movementInTheX;
				pz += movementInTheZ;
			}
		}

		// find movement in the y direction (change in elevation)
		// py is not changed in this process
		// only the movement is changed
		// each use subsequent use of py must add the elevation change
		// py is an absolute elevation - not a change in elevation
		// find the triangle directly underneath the camera
		glm::vec3 originMid(-px, _py + midHeight + movementInTheY, -pz);
		glm::vec3 ray(0.0f, -1.0f, 0.0f);
		glm::vec3 unitraylen = ray * 100.0f;
		glm::vec3 farpt = originMid + unitraylen;
		GSegmentModel los{ { originMid.x, originMid.y, originMid.z },{ farpt.x, farpt.y, farpt.z } };
		glm::vec3 elevationPoint;
		unsigned int triangleBelow = FindClosestTriThatIntersectsLine(
			GSpatialIndex, los, AllTris, originMid, ray, elevationPoint, NULL);
		if (triangleBelow != NO_TRIANGLE_FOUND) {
			//movementInTheY = elevationPoint.y;
			proposedMovementInTheY = elevationPoint.y;
		}

		// we've got our new position
		// now, construct a box around the upper portion of
		// the body - mid height to eye height, 3x x 3z
		/*
		GBoxModel UpperBody = { 
			{ -px - 1.5f, _py + midHeight + proposedMovementInTheY, -pz - 1.5f },
			{ -px + 1.5f, _py + eyeHeight + proposedMovementInTheY, -pz + 1.5f }
		};
		std::vector<GValueModel> IntersectResult_UpperBody;
		GSpatialIndex.query(boost::geometry::index::intersects(UpperBody), std::back_inserter(IntersectResult_UpperBody));
		fprintf(log, "intersect result upper body %i\n", IntersectResult_UpperBody.size());
		if (IntersectResult_UpperBody.size() > 0) {
			// if  hit something,  undo the x/z move
			px -= movementInTheX;
			pz -= movementInTheZ;
		}
		else {
			// if not hit something, apply the new y
			movementInTheY = proposedMovementInTheY;
		}
		*/
		movementInTheY = proposedMovementInTheY;

		// two problems:
		// 1. when the walker hits a wall, it stops moving
		//    when it should walk along the wall
		//    need to find the component of the movement
		//    vector parallel to the triangle and allow
		//    that
		//    need to compare movement vector to triangle
		//    normal vector
		// 2. when the walker tries to walk off something
		//    high, it hits the lower ground, but, is now
		//    in violation of intersecting the thing it
		//    just fell off of, so, it's not allowed to
		//    fall off an object >= 3

		// find the triangle that is being "looked at"
		// origin doesn't change
		glm::vec3 originEye(-px, _py + eyeHeight + movementInTheY, -pz);
		ray.x = cosf(DEG2RAD(EyeElevationInDegrees)) * -sinf(DEG2RAD(EyeAzimuthInDegrees));
		ray.y = sinf(DEG2RAD(EyeElevationInDegrees));
		ray.z = cosf(DEG2RAD(EyeElevationInDegrees)) * cosf(DEG2RAD(EyeAzimuthInDegrees));
		//glm::vec3 unitray = ray.MakeUnit();
		glm::vec3 unitray = glm::normalize(ray);
		unitraylen = ray * 100.0f;
		glm::vec3 oppRay = ray * -1.0f;
		farpt = originEye - unitraylen;
		GSegmentModel los2{ { originEye.x, originEye.y, originEye.z },{ farpt.x, farpt.y, farpt.z } };
		glm::vec3 pout;
		//fprintf(log, "origin %.1f, %.1f, %.1f\n", origin.x, origin.y, origin.z);
		//fprintf(log, "oppRay %.1f, %.1f, %.1f\n", oppRay.x, oppRay.y, oppRay.z);
		//fprintf(log, "farpt %.1f, %.1f, %.1f\n", farpt.x, farpt.y, farpt.z);
		unsigned int triangleLookingAt = FindClosestTriThatIntersectsLine(
			GSpatialIndex, los2, AllTris, originEye, oppRay, pout, NULL);

		// RENDER SCENE BEGIN

		// clear stuff
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set a perspective matrix
		SetPerspective(rect.right, rect.bottom);

		// create a grid of triangles as a "floor"
		glPushMatrix();
		{
			glLoadIdentity();

			// direction look l/r
			glRotatef(EyeAzimuthInDegrees, 0.0f, 1.0f, 0.0);

			float EyeAzimuthInRadians = DEG2RAD(EyeAzimuthInDegrees);
			GLfloat eex = cosf(EyeAzimuthInRadians);
			GLfloat eez = sinf(EyeAzimuthInRadians);

			// elevation look u/d
			glRotatef(EyeElevationInDegrees, eex, 0.0f, eez);

			// position x/y
			glTranslatef(px, -1.0f * (_py + eyeHeight + movementInTheY), pz);

			// draw the grid (white)
			glColor3f(0.9f, 0.9f, 1.0f);
			glPolygonMode(GL_FRONT, GL_FILL);
			glBegin(GL_TRIANGLES);
			{
				//glLineWidth(1.0f);
				for (std::vector<oglw::Triangle>::iterator iter = AllTris.begin(); iter != AllTris.end(); ++iter)
					iter->Draw();
			}
			glEnd();

			// draw some red triangles
			/*
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_TRIANGLES);
			{
				unsigned int ctr = 0;
				for (std::vector<oglw::Triangle>::iterator iter = AllTris.begin(); iter != AllTris.end(); ++iter)
				{
					if (ctr == triangleBelow || ctr == triangleLookingAt) iter->Draw();
					ctr++;
				}
			}
			glEnd();
			*/

			//glBegin(GL_POINTS);
			//glVertex3f(farpt.x, farpt.y, farpt.z);
			//glEnd();

			// draw surface normals
			//glBegin(GL_LINES);
			//for (std::vector<Triangle>::iterator iter = AllTris.begin(); iter != AllTris.end(); ++iter)
			//{
				//glVertex3f(iter->cntrd.x, iter->cntrd.y, iter->cntrd.z);
				//glVertex3f(
					//iter->cntrd.x + iter->sfcnrml.x,
					//iter->cntrd.y + iter->sfcnrml.y,
					//iter->cntrd.z + iter->sfcnrml.z);
			//}
			//glEnd();

			// set back to white lines
			glColor3f(1.0f, 1.0f, 1.0f);
			glPolygonMode(GL_FRONT, GL_FILL);

			// draw a sphere
			glPushMatrix();
			{
				glTranslatef(-100.0f, 100.0f, -100.0f);
				GLUquadric* pquad = gluNewQuadric();
				gluSphere(pquad, 5, 10, 10);
				gluDeleteQuadric(pquad);
			}
			glPopMatrix();

		}
		glPopMatrix();

		// create a "crosshair" in the middle of the screen
		SetOrtho(rect.right, rect.bottom);
		glColor3f(0.0f, 1.0f, 0.0f);
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		{
			glVertex2i((rect.right / 2) - 10, rect.bottom / 2);
			glVertex2i((rect.right / 2) + 10, rect.bottom / 2);
			glVertex2i(rect.right / 2, (rect.bottom / 2) - 10);
			glVertex2i(rect.right / 2, (rect.bottom / 2) + 10);
		}
		glEnd();

		// draw some text on the screen
		// debugging messages - for now
		glColor3f(1.0f, 1.0f, 1.0f);
		glRasterPos2i(4, 18);
		FontPrintf(pFont, 1, "fps %i", (int)FramesPerSecond);

		SYSTEMTIME stime;
		GetSystemTime(&stime);
		glRasterPos2i(4, 36);
		FontPrintf(pFont, 1, "%02i:%02i:%02i\n", stime.wHour, stime.wMinute, stime.wSecond);

		glRasterPos2i((rect.right / 2) + 10, (rect.bottom / 2) + 10);
		//FontPrintf(pFont, 1, "%.0f degrees", EyeAzimuthInDegrees);
		//FontPrintf(pFont, 1, "isect body %i", IntersectResult_UpperBody.size());

		//glRasterPos2i((rect.right / 2) + 10, (rect.bottom / 2) + 24);
		//FontPrintf(pFont, 1, "%.4f, %.4f", ex, ez);

		// finish it all
		glFinish();

		// RENDER SCENE END

		// present to screen
		SwapBuffers(hdc);
	}

	fprintf(log, "destroy rc stuff\n");
	FontDestroy(pFont);
	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(hrc);


	fprintf(log, "quit\n");
	fclose(log);
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	fopen_s(&g_log, "c:\\temp\\ow.log", "w");

	//objl::Loader loader;
	//std::string objfile = "";
	//bool bRet = loader.LoadFile(objfile);

	//Point origin(0, 0, 0);
	//Point ray(0, -1, 0);
	//Triangle t(Point(0, -2, 1), Point(1, -2, -1), Point(-1, -2, -1));
	//Point result;
	//bool b = RayIntersectsTriangle(origin, ray, t, result);
	//fprintf(g_log, "%i %.1f %.1f %.1f\n", b, result.x, result.y, result.z);

	//Point a(5, 1, 0);
	//Point b(-1, 0, 0);
	//float adotb = a.dotProduct(b);
	//float bdotb = b.dotProduct(b);
	//Point rp = b * (adotb / bdotb);
	//fprintf(g_log, "proj %.1f, %.1f, %.1f\n", rp.x, rp.y, rp.z);

	//Point p1(0, 0, 0);
	//Point p2(1, 0, 0);
	//Point p3(1, 1, 0);
	//Triangle t(p1, p2, p3);
	//Point nv = t.SurfaceNormal();
	//fprintf(g_log, "nv %.1f, %.1f, %.1f\n", nv.x, nv.y, nv.z);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OGLWALKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
	HWND hWnd = InitInstance(hInstance, nCmdShow);
	if(hWnd == NULL)
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OGLWALKER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	fclose(g_log);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OGLWALKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDC_OGLWALKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hDesktopWnd = GetDesktopWindow();
   HDC hDesktopDc = GetDC(hDesktopWnd);

   int sx = GetDeviceCaps(hDesktopDc, HORZRES);
   int sy = GetDeviceCaps(hDesktopDc, VERTRES);

   ReleaseDC(hDesktopWnd, hDesktopDc);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, 
	   //WS_OVERLAPPEDWINDOW,
	   WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
	   0,0,sx,sy,
		//CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
	   nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return NULL;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL useDI = FALSE;
    switch (message)
    {
	case WM_CREATE:
		{
			// create the rendering thread
			g_ctx.hQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			g_ctx.hWnd = hWnd;
			if (FALSE == SetupDirectInput(hInst, hWnd)) {
				useDI = FALSE;
			}
			else {
				useDI = TRUE;
				ShowCursor(FALSE);
			}
			g_ctx.useDI = useDI;
			hRenderingThread = (HANDLE)CreateThread(NULL, 0, RenderingThreadEntryPoint, (LPVOID)&g_ctx, 0, &RenderThreadId);
		}
		break;
	case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
			ValidateRect(hWnd, NULL);
        }
        break;
	case CUSTOM_QUIT:
    case WM_DESTROY:
		{
			SetEvent(g_ctx.hQuitEvent);
			WaitForSingleObject(hRenderingThread, INFINITE);
			if (TRUE == useDI) {
				ShowCursor(TRUE);
				CleanupDirectInput();
			}
			PostQuitMessage(0);
		}
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void SetDCPixelFormat(HDC hdc) 
{
	int nPixelFormat;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1, // version
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, // bit color
		0,0,0,0,0,0,
		0,0,
		0,0,0,0,0,
		16, // depth buffer
		0,
		0,
		0,
		0,
		0,0,0 };
	nPixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, nPixelFormat, &pfd);
}

void ChangeSize(GLsizei w, GLsizei h) {
	fprintf(g_log, "ChangeSize start %i, %i\n", w, h);
	GLfloat fAspect;
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
	fAspect = (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60.0f, fAspect, 1.0, 400.0);
	glOrtho(0.0, (GLfloat)w, 0.0, (GLfloat)h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	fprintf(g_log, "ChangeSize end\n");
}

/*
* Limits...
*/

#define MAX_STRING	1024


/*
* 'FontCreate()' - Load Windows font bitmaps into OpenGL display lists.
*/

GLFONT *                         /* O - Font data */
FontCreate(HDC        hdc,       /* I - Device Context */
	const wchar_t *typeface, /* I - Font specification */
	int        height,    /* I - Font height/size in pixels */
	int        weight,    /* I - Weight of font (bold, etc) */
	DWORD      italic)    /* I - Text is italic */
{
	GLFONT *font;                /* Font data pointer */
	HFONT  fontid;               /* Windows font ID */
	int    charset;              /* Character set code */

								 /* Allocate memory */
	if ((font = (GLFONT*)calloc(1, sizeof(GLFONT))) == (GLFONT *)0)
		return ((GLFONT *)0);

	/* Allocate display lists */
	if ((font->base = glGenLists(256)) == 0)
	{
		free(font);
		return (0);
	}

	/* Select a character set */
	if (wcscmp(typeface, L"symbol") == 0)
		charset = SYMBOL_CHARSET;
	else
		charset = ANSI_CHARSET;

	/* Load the font */
	fontid = CreateFont(height, 0, 0, 0, weight, italic, FALSE, FALSE,
		charset, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
		DRAFT_QUALITY, DEFAULT_PITCH, typeface);

	SelectObject(hdc, fontid);

	/* Create bitmaps for each character */
	wglUseFontBitmaps(hdc, 0, 256, font->base);

	/* Get the width and height information for each character */
	GetCharWidth(hdc, 0, 255, font->widths);
	font->height = height;

	return (font);
}

/*
* 'FontDestroy()' - Delete the specified font.
*/

void
FontDestroy(GLFONT *font) /* I - Font to delete */
{
	if (font == (GLFONT *)0)
		return;

	glDeleteLists(font->base, 256);
	free(font);
}


/*
* 'FontPuts()' - Display a string using the specified font.
*/

void
FontPuts(GLFONT     *font, /* I - Font to use */
	const char *s)    /* I - String to display */
{
	if (font == (GLFONT *)0 || s == NULL)
		return;

	glPushAttrib(GL_LIST_BIT);
	glListBase(font->base);
	glCallLists((GLsizei)strlen(s), GL_UNSIGNED_BYTE, s);
	glPopAttrib();
}


/*
* 'FontPrintf()' - Display a formatted string using the specified font.
*/

void
FontPrintf(GLFONT     *font,   /* I - Font to use */
	int        align,   /* I - Alignment to use */
	const char *format, /* I - printf() style format string */
	...)                /* I - Other arguments as necessary */
{
	va_list       ap;          /* Argument pointer */
	unsigned char s[1024],     /* Output string */
		*ptr;        /* Pointer into string */
	int           width;       /* Width of string in pixels */

	if (font == (GLFONT *)0 || format == (char *)0)
		return;

	/* Format the string */
	va_start(ap, format);
	vsprintf_s((char *)s, 1024, format, ap);
	va_end(ap);

	/* Figure out the width of the string in pixels... */
	for (ptr = s, width = 0; *ptr; ptr++)
		width += font->widths[*ptr];

	/* Adjust the bitmap position as needed to justify the text */
	if (align < 0)
		glBitmap(0, 0, 0, 0, (float)-width, 0, NULL);
	else if (align == 0)
		glBitmap(0, 0, 0, 0, (float)-width / 2, 0, NULL);

	/* Draw the string */
	FontPuts(font, (const char*)s);
}

IDirectInput8 *pDirectInput8 = NULL;
IDirectInputDevice8 *pKeyboard = NULL;
IDirectInputDevice8 *pMouse = NULL;

bool ReadKeyboardState(unsigned char* keystate)
{
	HRESULT result;
	result = pKeyboard->GetDeviceState(sizeof(unsigned char[256]), (LPVOID)keystate);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			pKeyboard->Acquire();
		}
		else {
			return false;
		}
	}
	return true;
}

bool ReadMouseState(DIMOUSESTATE *pMouseState)
{
	HRESULT result;

	// Read the mouse device.
	result = pMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)pMouseState);
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			pMouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}

void CleanupDirectInput()
{
	if (pMouse != NULL) {
		pMouse->Unacquire();
		pMouse->Release();
	}
	if (pKeyboard != NULL) {
		pKeyboard->Unacquire();
		pKeyboard->Release();
	}
	if (pDirectInput8 != NULL) pDirectInput8->Release();
}

BOOL SetupDirectInput(HINSTANCE hInst, HWND hWnd)
{
	if (FAILED(DirectInput8Create(hInst, 0x0800, IID_IDirectInput8, (void**)&pDirectInput8, NULL)))
		goto ERR;

	if (FAILED(pDirectInput8->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL)))
		goto ERR;

	if (FAILED(pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
		goto ERR;

	if (FAILED(pKeyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		goto ERR;

	if (FAILED(pKeyboard->Acquire()))
		goto ERR;

	if (FAILED(pDirectInput8->CreateDevice(GUID_SysMouse, &pMouse, NULL)))
		goto ERR;

	if (FAILED(pMouse->SetDataFormat(&c_dfDIMouse)))
		goto ERR;

	if (FAILED(pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		goto ERR;

	if (FAILED(pMouse->Acquire()))
		goto ERR;

	return TRUE;

ERR:
	CleanupDirectInput();
	return FALSE;
}
