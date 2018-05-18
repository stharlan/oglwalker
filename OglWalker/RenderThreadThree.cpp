
#include "stdafx.h"

const char* vs1 =
"#version 330\r\n"
"layout(location = 0) in vec3 Position;\r\n"
"layout(location = 1) in vec4 Color;\r\n"
"layout(location = 2) in vec3 Normal;\r\n"
"layout(location = 3) in vec2 TexCoord;\r\n"
"uniform mat4 gWorld;\r\n"
"out vec2 TexCoord0;\r\n"
"out vec4 Color0;\r\n"
"void main()\r\n"
"{\r\n"
"	gl_Position = gWorld * vec4(Position, 1.0);\r\n"
"	TexCoord0 = TexCoord;\r\n"
"   Color0 = Color;\r\n"
"}\r\n";

const char* fs1 =
"#version 330\r\n"
"in vec2 TexCoord0;\r\n"
"in vec4 Color0;\r\n"
"out vec4 FragColor;\r\n"
//"uniform sampler2D gSampler;\r\n"
"void main()\r\n"
"{\r\n"
//"	FragColor = texture2D(gSampler, TexCoord0.xy);\r\n"
"   FragColor = Color0;\r\n"
"}\r\n";

typedef struct {
	glm::vec3 position;
	glm::vec4 color;
	glm::vec3 normal;
	glm::vec2 texture;
} glmvec12;

void LoadGeometries(rapidjson::Document& d, GLuint *VBA, GLuint *IBA)
{
	rapidjson::Value& objectArray = d["objects"];
	for (int i = 0; i < objectArray.Size(); i++) {
		rapidjson::Value& object = objectArray[i];
		rapidjson::Value& type = object["type"];
		if (strcmp(type.GetString(), "triangle") == 0) {

			rapidjson::Value& color = object["color"];
			glm::vec4 TriangleColor(
				color[0].GetFloat(),
				color[1].GetFloat(),
				color[2].GetFloat(),
				color[3].GetFloat());

			rapidjson::Value& vertices = object["vertices"];
			rapidjson::Value& v0 = vertices[0];
			glm::vec3 gv0(v0[0].GetFloat(), v0[1].GetFloat(), v0[2].GetFloat());
			rapidjson::Value& v1 = vertices[1];
			glm::vec3 gv1(v1[0].GetFloat(), v1[1].GetFloat(), v1[2].GetFloat());
			rapidjson::Value& v2 = vertices[2];
			glm::vec3 gv2(v2[0].GetFloat(), v2[1].GetFloat(), v2[2].GetFloat());
			
			glm::vec3 gv10 = gv1 - gv0;
			glm::vec3 gv20 = gv2 - gv0;
			glm::vec3 Normal = glm::cross(gv1, gv2);
			glm::vec3 NormalUnit = glm::normalize(Normal);

			glm::vec2 TexCoord(0.0f, 0.0f);

			glmvec12 vertexArray[] = {
				{ gv0, TriangleColor, NormalUnit, TexCoord },
				{ gv1, TriangleColor, NormalUnit, TexCoord },
				{ gv2, TriangleColor, NormalUnit, TexCoord }
			};

			glGenBuffers(1, VBA);
			glBindBuffer(GL_ARRAY_BUFFER, *VBA);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

			unsigned int indexArray[] = { 0, 1, 2 };

			glGenBuffers(2, IBA);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IBA);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexArray), indexArray, GL_STATIC_DRAW);
		}
	}
}

DWORD WINAPI RenderingThreadThreeEntryPoint(void* pVoid)
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

	GLuint VBA = 0, IBA = 0;
	GLuint gWorldLocation;
	GLuint gSampler;

	fopen_s(&log, "c:\\temp\\rt.log", "w");
	fprintf(log, "starting render thread 3\n");

	GetClientRect(ctx->hWnd, &rect);

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

	char* content = CommonReadFile(ctx->InputFilename);
	rapidjson::Document d;
	d.Parse(content);

	fprintf(log, "load geometries\n");
	LoadGeometries(d, &VBA, &IBA);

	fprintf(log, "load shaders\n");
	GLuint ShaderProgram = CompileShaders(vs1, fs1, log);

	fprintf(log, "get world location\n");
	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);

	fprintf(log, "start loop\n");
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

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBA);
			glBindBuffer(GL_ARRAY_BUFFER, VBA);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)12);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)28);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)40);
			glDrawElements(GL_TRIANGLES, 1 * 3, GL_UNSIGNED_INT, 0);

			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOA[1]);
			//glBindBuffer(GL_ARRAY_BUFFER, VBOA[0]);
			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec5), 0);
			//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glmvec5), (const GLvoid*)12);
			//glBindTexture(GL_TEXTURE_2D, TEX[0]);
			//glDrawElements(GL_TRIANGLES, 18 * 3, GL_UNSIGNED_INT, 0);

			//for (GLuint i = 1; i < NUM_ARRAYS; i++) {
			//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOA[0]);
			//	glBindBuffer(GL_ARRAY_BUFFER, VBOA[i]);
			//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec5), 0);
			//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glmvec5), (const GLvoid*)12);
			//	glBindTexture(GL_TEXTURE_2D, TEX[i]);
			//	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			//}

			// disable
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);

			SwapBuffers(hdc);
		}

		fprintf(log, "FPS %.1f\n", FramesPerSecond);

	}

	glDeleteBuffers(1, &VBA);
	glDeleteBuffers(1, &IBA);
	//glDeleteTextures(NUM_ARRAYS, &TEX[0]);

	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(hrc);

	return 0;
}
