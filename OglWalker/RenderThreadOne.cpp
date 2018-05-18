
#include "stdafx.h"

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

void SetupRC()
{
	//pFont = FontCreate(wglGetCurrentDC(), L"Arial", 18, 0, 0);

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
}


GLuint CreateShaders(FILE* log)
{
	GLuint ShaderProgram = glCreateProgram();

	const char* vs = "#version 330\r\n"
		"layout(location = 0) in vec3 Position;\r\n"
		"uniform mat4 gWorld;\r\n"
		"out vec4 Color;\r\n"
		"void main() {\r\n"
		"gl_Position = gWorld * vec4(Position, 1.0);\r\n"
		"Color = vec4(clamp(Position, 0.0, 1.0), 1.0);\r\n"
		"}";

	const char* fs = "#version 330\r\n"
		"in vec4 Color;\r\n"
		"out vec4 FragColor;\r\n"
		"void main()\r\n"
		"{ FragColor = Color; }";

	AddShader(ShaderProgram, vs, GL_VERTEX_SHADER, log);
	AddShader(ShaderProgram, fs, GL_FRAGMENT_SHADER, log);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		exit(1);
	}

	//glUseProgram(ShaderProgram);

	GLuint gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);

	return gWorldLocation;
}

void AddCubeTris(CubeObject& c1,
	std::vector<oglw::Triangle>& AllTris,
	boost::geometry::index::rtree< GValueModel, boost::geometry::index::quadratic<16> >& GSpatialIndex)
{
	for (std::vector<oglw::Triangle>::iterator iter = c1.tris.begin(); iter != c1.tris.end(); ++iter)
	{
		AllTris.push_back(*iter);
		GBoxModel b{
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


void CreateGlBuffers(std::vector<oglw::Triangle>& AllTris, GLuint BufferIds[2])
{
	BufferIds[0] = 0;
	BufferIds[1] = 0;

	glm::vec3* verts = (glm::vec3*)malloc(AllTris.size() * 3 * sizeof(glm::vec3));
	glm::uvec3* idxs = (glm::uvec3*)malloc(AllTris.size() * sizeof(glm::uvec3));

	unsigned int ctr = 0;
	for (std::vector<oglw::Triangle>::iterator iter = AllTris.begin(); iter != AllTris.end(); ++iter) {
		verts[ctr++] = iter->p1;
		verts[ctr++] = iter->p2;
		verts[ctr++] = iter->p3;
		glm::uvec3 ti(ctr - 3, ctr - 2, ctr - 1);
		idxs[(ctr / 3) - 1] = ti;
	}

	glGenBuffers(2, &BufferIds[0]);

	glBindBuffer(GL_ARRAY_BUFFER, BufferIds[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

	free(verts);
	free(idxs);
}

DWORD WINAPI RenderingThreadOneEntryPoint(void* pVoid)
{
	RENDER_THREAD_CONTEXT* ctx = (RENDER_THREAD_CONTEXT*)pVoid;

	// create the rtree using default constructor
	boost::geometry::index::rtree< GValueModel, boost::geometry::index::quadratic<16> > GSpatialIndex;

	std::vector<oglw::Triangle> AllTris;

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
	GLuint BufferIds[2];
	GLuint WorldLocation = 0;

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

	GLenum res = glewInit();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	AddSomeStuff(AllTris, GSpatialIndex);
	CreateGlBuffers(AllTris, &BufferIds[0]);

	WorldLocation = CreateShaders(log);

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
				}
				else if (keystate[DIK_S] == (unsigned char)128) {
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
				}
				else if (keystate[DIK_A] == (unsigned char)128) {
					MovementDirectionInDegrees = EyeAzimuthInDegrees - 90.0f;
					EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
					ex = sinf(EyeAzimuthInRadians);
					ez = cosf(EyeAzimuthInRadians);
				}
				else if (keystate[DIK_D] == (unsigned char)128) {
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

		// clear stuff
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// RENDER SCENE BEGIN2

		//gluPerspective(60.0f, fAspect, 1.0, 400.0);
		//glm::mat4x4 w = glm::perspective(60.0f, (float)rect.right / (float)rect.bottom, 1.0f, 400.0f);
		//glUniformMatrix4fv(WorldLocation, 1, GL_TRUE, (const GLfloat*)&w);

		// RENDER SCENE END2

		// RENDER SCENE BEGIN
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
			////glColor3f(1.0f, 0.0f, 0.0f);
			////glBegin(GL_TRIANGLES);
			////{
			////	unsigned int ctr = 0;
			////	for (std::vector<oglw::Triangle>::iterator iter = AllTris.begin(); iter != AllTris.end(); ++iter)
			////	{
			////		if (ctr == triangleBelow || ctr == triangleLookingAt) iter->Draw();
			////		ctr++;
			////	}
			////}
			////glEnd();

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
		//glColor3f(1.0f, 1.0f, 1.0f);
		//glRasterPos2i(4, 18);
		//FontPrintf(pFont, 1, "fps %i", (int)FramesPerSecond);

		//SYSTEMTIME stime;
		//GetSystemTime(&stime);
		//glRasterPos2i(4, 36);
		//FontPrintf(pFont, 1, "%02i:%02i:%02i\n", stime.wHour, stime.wMinute, stime.wSecond);

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

	glDeleteBuffers(2, &BufferIds[0]);

	fprintf(log, "destroy rc stuff\n");
	//FontDestroy(pFont);
	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(hrc);


	fprintf(log, "quit\n");
	fclose(log);
	return 0;
}
