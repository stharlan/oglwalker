
#include "stdafx.h"

//const char* vs = 
//"#version 330\r\n"
//"layout(location = 0) in vec3 Position;\r\n"
//"uniform mat4 gWorld;\r\n"
//"out vec4 Color;\r\n"
//"void main()\r\n"
//"{ gl_Position = gWorld * vec4(Position, 1.0);"
//"Color = vec4(1.0, 0.0, 0.0, 1.0); }";

const char* vs =
"#version 330\r\n"
"layout(location = 0) in vec3 Position;\r\n"
"layout(location = 1) in vec2 TexCoord;\r\n"
"uniform mat4 gWorld;\r\n"
"out vec2 TexCoord0;\r\n"
"void main()\r\n"
"{\r\n"
"	gl_Position = gWorld * vec4(Position, 1.0);\r\n"
"	TexCoord0 = TexCoord;\r\n"
"}\r\n";

//const char* fs = 
//"#version 330\r\n"
//"in vec4 Color;\r\n"
//"out vec4 FragColor;\r\n"
//"void main()\r\n"
//"{ FragColor = Color; }";

const char* fs =
"#version 330\r\n"
"in vec2 TexCoord0;\r\n"
"out vec4 FragColor;\r\n"
"uniform sampler2D gSampler;\r\n"
"void main()\r\n"
"{\r\n"
"	FragColor = texture2D(gSampler, TexCoord0.xy);\r\n"
"}\r\n";

typedef struct {
	glm::vec3 position;
	glm::vec2 texture;
} glmvec5;

#define NUM_ARRAYS 6

#define MAKE_COORD1(x,y,z) { glm::vec3(459.0f - (float)x, (float)y, -1.0f * (float)z), glm::vec2((float)x / 459.0f, (float)z / 449.0f) }

void CreateVertexBuffer(GLuint* VBOA)
{
	glmvec5 Vertices0[] = {
		{ glm::vec3(0, 0, 0), glm::vec2(1.0f, 0.0f) },
		{ glm::vec3(459, 0, 0), glm::vec2(0.0f, 0.0f) },
		{ glm::vec3(459, 0, -449), glm::vec2(0.0f, 1.0f) },
		{ glm::vec3(0, 0, -449), glm::vec2(1.0f, 1.0f) },
		{ glm::vec3(459 - 231, -15, -240), glm::vec2(0.5032679739f,0.5345211581f) },
		{ glm::vec3(459 - 193, -13, -205), glm::vec2(0.4204793028f,0.4565701559f) },
		{ glm::vec3(459 - 200, -15, -212), glm::vec2(0.4357298475f,0.4721603563f) },
		{ glm::vec3(459 - 238, -2, -155), glm::vec2(0.5185185185f,	0.3452115813f) },
		{ glm::vec3(459 - 248, 0, -165), glm::vec2(0.5403050109f,	0.3674832962f) },
		MAKE_COORD1(286, 0, 197),
		MAKE_COORD1(303, 0, 179),
		MAKE_COORD1(344, 0, 217)
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, VBOA[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices0), Vertices0, GL_STATIC_DRAW);

	glmvec5 Vertices1[] = {
		{ glm::vec3(459 - 344, 0, -217), glm::vec2(0.0190972222f,0.7146164021f) },
		{ glm::vec3(459 - 303, 0, -179), glm::vec2(0.9625496032f,0.7291666667f) },
		{ glm::vec3(459 - 303, 15, -179), glm::vec2(0.9704861111f,0.3561507937f) },
		{ glm::vec3(459 - 344, 15, -217), glm::vec2(0.0151289683f,0.3376322751f) }
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBOA[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);

	glmvec5 Vertices2[] = {
		{ glm::vec3(459 - 303, 0, -179), glm::vec2(0.0181051587f,	0.8442460317f) },
		{ glm::vec3(459 - 286, 0, -197), glm::vec2(0.8802083333f,	0.8521825397f) },
		{ glm::vec3(459 - 286, 15, -197), glm::vec2(0.8990575397f,	0.1947751323f) },
		{ glm::vec3(459 - 303, 15, -179), glm::vec2(0.0f,	0.1498015873f) }
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBOA[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);

	glmvec5 Vertices3[] = {
		{ glm::vec3(459 - 286, 0, -197), glm::vec2(0.267609127f,0.648478836f) },
		{ glm::vec3(459 - 238, 0, -155), glm::vec2(0.910218254f,0.6597222222f) },
		{ glm::vec3(459 - 238, 15, -155), glm::vec2(0.912202381f,0.4457671958f) },
		{ glm::vec3(459 - 286, 15, -197), glm::vec2(0.2663690476f,0.4513888889f) }
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBOA[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices3), Vertices3, GL_STATIC_DRAW);

	glmvec5 Vertices4[] = {
		{ glm::vec3(459 - 238, 0, -155), glm::vec2(0.0967261905f, 0.6164021164f) },
		{ glm::vec3(459 - 193, 0, -205), glm::vec2(0.8896329365f, 0.5962301587f) },
		{ glm::vec3(459 - 193, 15, -205), glm::vec2(0.8864087302f, 0.328042328f) },
		{ glm::vec3(459 - 238, 15, -155), glm::vec2(0.0932539683f,	0.3349867725f) }
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBOA[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices4), Vertices4, GL_STATIC_DRAW);

	glmvec5 Vertices5[] = {
		{ glm::vec3(459 - 193, -15, -205), glm::vec2(0.2514880952f, 0.6802248677f) },
		{ glm::vec3(459 - 231, -15, -240), glm::vec2(0.7663690476f, 0.6851851852f) },
		{ glm::vec3(459 - 231, 15, -240), glm::vec2(0.7743055556f, 0.2748015873f) },
		{ glm::vec3(459 - 193, 15, -205), glm::vec2(0.2410714286f,	0.2774470899f) }
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBOA[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices5), Vertices5, GL_STATIC_DRAW);
}

void CreateIndexBuffer(GLuint* IBOA)
{
	unsigned int Indices0[] = { 
		0, 1, 2,
		0, 2, 3
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOA[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices0), Indices0, GL_STATIC_DRAW);

	unsigned int Indices1[] = {
		2, 3, 4,
		2, 4, 6,
		2, 6, 5,
		2, 5, 1,
		0, 11, 3,
		11, 4, 3,
		0, 9, 10,
		0, 10, 11,
		10, 4, 11,
		10, 9, 4,
		9, 6, 4,
		0, 7, 8,
		7, 6, 8,
		0, 8, 9,
		8, 6, 9,
		7, 5, 6,
		0, 1, 7,
		1, 5, 7
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOA[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices1), Indices1, GL_STATIC_DRAW);
}

DWORD WINAPI RenderingThreadTwoEntryPoint(void* pVoid)
{

	RENDER_THREAD_CONTEXT* ctx = (RENDER_THREAD_CONTEXT*)pVoid;
	unsigned char keystate[256];
	DIMOUSESTATE mouseState;
	LARGE_INTEGER perfCount;
	LARGE_INTEGER perfFreq;
	LONGLONG lastCount = 0;
	FILE* log = NULL;
	RECT rect;
	float ex = 0.0f, ey = 6.0f, ez = 0.0f;
	float azimuth = 180.0f;
	float elevation = 0.0f;

	GLuint VBOA[NUM_ARRAYS];
	GLuint TEX[NUM_ARRAYS];
	GLuint IBOA[2];
	GLuint gWorldLocation;
	GLuint gSampler;

	GetClientRect(ctx->hWnd, &rect);

	fopen_s(&log, "c:\\temp\\rt.log", "w");

	QueryPerformanceFrequency(&perfFreq);

	HDC hdc = GetDC(ctx->hWnd);
	SetDCPixelFormat(hdc);
	HGLRC hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	GLenum res = glewInit();

	glClearColor(0.5f, 0.7f, 1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glGenBuffers(NUM_ARRAYS, &VBOA[0]);
	CreateVertexBuffer(&VBOA[0]);
	glGenBuffers(2, &IBOA[0]);
	CreateIndexBuffer(&IBOA[0]);

	GLuint ShaderProgram =  CompileShaders(vs, fs, log);

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);

	gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
	assert(gSampler != 0xFFFFFFFF);

	glUniform1i(gSampler, 0);

	glGenTextures(NUM_ARRAYS, &TEX[0]);
	TextureLoad("c:\\temp\\home_capture.png", GL_TEXTURE_2D, TEX[0]);
	TextureLoad("c:\\temp\\20180512_173716.jpg", GL_TEXTURE_2D, TEX[1]);
	TextureLoad("c:\\temp\\20180512_202051.jpg", GL_TEXTURE_2D, TEX[2]);
	TextureLoad("c:\\temp\\20180513_080139.jpg", GL_TEXTURE_2D, TEX[3]);
	TextureLoad("c:\\temp\\20180513_082612.jpg", GL_TEXTURE_2D, TEX[4]);
	TextureLoad("c:\\temp\\20180513_084356.jpg", GL_TEXTURE_2D, TEX[5]);

	while (1) {

		if (WaitForSingleObject(ctx->hQuitEvent, 0) == WAIT_OBJECT_0) {
			break;
		}

		QueryPerformanceCounter(&perfCount);
		float FramesPerSecond = (float)perfFreq.QuadPart / (float)(perfCount.QuadPart - lastCount);
		float WalkingStride = 50.0f / FramesPerSecond;
		lastCount = perfCount.QuadPart;

		if (TRUE == ctx->useDI) {
			if (TRUE == ReadMouseState(&mouseState))
			{
				azimuth -= (mouseState.lX / 8.0f);
				if (azimuth < 0.0f) azimuth = azimuth + 360.0f;
				if (azimuth > 360.0f) azimuth = azimuth - 360.0f;

				elevation += (mouseState.lY / 8.0f);
				if (elevation < -90.0f) elevation = -90.0f;
				if (elevation > 90.0f) elevation = 90.0f;
			}
			if (TRUE == ReadKeyboardState(&keystate[0]))
			{
				if (keystate[DIK_ESCAPE] == (unsigned char)128) {
					PostMessage(ctx->hWnd, CUSTOM_QUIT, 0, 0);
				}

				float px = 0.0f, pz = 0.0f;
				float EyeAzimuthInRadians = 0.0f;
				float MovementDirectionInDegrees = 0.0f;

				// using standard wsad for movement
				// fwd bck strafe left and strafe right
				if (keystate[DIK_W] == (unsigned char)128) {
					if (keystate[DIK_A] == (unsigned char)128) {
						MovementDirectionInDegrees = azimuth + 45.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						px = sinf(EyeAzimuthInRadians);
						pz = cosf(EyeAzimuthInRadians);
					}
					else if (keystate[DIK_D] == (unsigned char)128) {
						MovementDirectionInDegrees = azimuth - 45.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						px = sinf(EyeAzimuthInRadians);
						pz = cosf(EyeAzimuthInRadians);
					}
					else {
						MovementDirectionInDegrees = azimuth;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						px = sinf(EyeAzimuthInRadians);
						pz = cosf(EyeAzimuthInRadians);
					}
				}
				else if (keystate[DIK_S] == (unsigned char)128) {
					if (keystate[DIK_A] == (unsigned char)128) {
						MovementDirectionInDegrees = azimuth + 135.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						px = sinf(EyeAzimuthInRadians);
						pz = cosf(EyeAzimuthInRadians);
					}
					else if (keystate[DIK_D] == (unsigned char)128) {
						MovementDirectionInDegrees = azimuth - 135.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						px = sinf(EyeAzimuthInRadians);
						pz = cosf(EyeAzimuthInRadians);
					}
					else {
						MovementDirectionInDegrees = azimuth + 180.0f;
						EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
						px = sinf(EyeAzimuthInRadians);
						pz = cosf(EyeAzimuthInRadians);
					}
				}
				else if (keystate[DIK_A] == (unsigned char)128) {
					MovementDirectionInDegrees = azimuth + 90.0f;
					EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
					px = sinf(EyeAzimuthInRadians);
					pz = cosf(EyeAzimuthInRadians);
				}
				else if (keystate[DIK_D] == (unsigned char)128) {
					MovementDirectionInDegrees = azimuth - 90.0f;
					EyeAzimuthInRadians = DEG2RAD(MovementDirectionInDegrees);
					px = sinf(EyeAzimuthInRadians);
					pz = cosf(EyeAzimuthInRadians);
				}

				if (keystate[DIK_Z] == (unsigned char)128) ey -= WalkingStride;
				if (keystate[DIK_C] == (unsigned char)128) ey += WalkingStride;

				ex += WalkingStride * px;
				ez += WalkingStride * pz;
			}
		}

		// Render Scene
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::mat4 projmat;
			glm::mat4 modelmat;
			glm::mat4 ModelViewProjectionMatrix;

			projmat = glm::mat4(1.0);
			projmat *= glm::perspective(45.0f, (float)rect.right / (float)rect.bottom, 0.1f, 1000.0f);

			modelmat = glm::mat4(1.0);
			// eye, center, up
			float azimuth_in_radians = DEG2RAD(azimuth);
			modelmat *= glm::lookAt(
				glm::vec3(ex, ey, ez), // eye, my eye position
				glm::vec3(ex + sinf(azimuth_in_radians), ey - sinf(DEG2RAD(elevation)), ez + cosf(azimuth_in_radians)),
				glm::vec3(0.0f, 1.0f, 0.0f)); // up vector

			ModelViewProjectionMatrix = projmat * modelmat;

			glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glActiveTexture(GL_TEXTURE0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOA[1]);
			glBindBuffer(GL_ARRAY_BUFFER, VBOA[0]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec5), 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glmvec5), (const GLvoid*)12);
			glBindTexture(GL_TEXTURE_2D, TEX[0]);
			glDrawElements(GL_TRIANGLES, 18 * 3, GL_UNSIGNED_INT, 0);

			for (GLuint i = 1; i < NUM_ARRAYS; i++) {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOA[0]);
				glBindBuffer(GL_ARRAY_BUFFER, VBOA[i]);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec5), 0);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glmvec5), (const GLvoid*)12);
				glBindTexture(GL_TEXTURE_2D, TEX[i]);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}

			// disable
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			SwapBuffers(hdc);
		}

		fprintf(log, "FPS %.1f\n", FramesPerSecond);

	}

	glDeleteBuffers(NUM_ARRAYS, &VBOA[0]);
	glDeleteBuffers(2, &IBOA[0]);
	glDeleteTextures(NUM_ARRAYS, &TEX[0]);

	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(hrc);

	return 0;
}
