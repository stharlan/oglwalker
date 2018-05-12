
#include "stdafx.h"

GLuint VBO;
GLuint IBO;
GLuint gWorldLocation;

const char* vs = 
"#version 330\r\n"
"layout(location = 0) in vec3 Position;\r\n"
"uniform mat4 gWorld;\r\n"
"out vec4 Color;\r\n"
"void main()\r\n"
"{ gl_Position = gWorld * vec4(Position, 1.0);"
"Color = vec4(clamp(Position, 0.0, 1.0), 1.0); }";

const char* fs = 
"#version 330\r\n"
"in vec4 Color;\r\n"
"out vec4 FragColor;\r\n"
"void main()\r\n"
"{ FragColor = Color; }";

static void CreateVertexBuffer()
{
	glm::vec3 Vertices[4];
	Vertices[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
	Vertices[1] = glm::vec3(0.0f, -1.0f, 1.0f);
	Vertices[2] = glm::vec3(1.0f, -1.0f, 0.0f);
	Vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	unsigned int Indices[] = { 0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2 };

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

static void CompileShaders()
{
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	AddShader(ShaderProgram, vs, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	assert(gWorldLocation != 0xFFFFFFFF);

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

	fopen_s(&log, "c:\\temp\\rt.log", "w");

	QueryPerformanceFrequency(&perfFreq);

	HDC hdc = GetDC(ctx->hWnd);
	SetDCPixelFormat(hdc);
	HGLRC hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	GLenum res = glewInit();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();
	CreateIndexBuffer();

	CompileShaders();

	while (1) {

		if (WaitForSingleObject(ctx->hQuitEvent, 0) == WAIT_OBJECT_0) {
			break;
		}

		QueryPerformanceCounter(&perfCount);
		float FramesPerSecond = (float)perfFreq.QuadPart / (float)(perfCount.QuadPart - lastCount);
		lastCount = perfCount.QuadPart;

		if (TRUE == ctx->useDI) {
			if (TRUE == ReadMouseState(&mouseState))
			{

			}
			if (TRUE == ReadKeyboardState(&keystate[0]))
			{
				if (keystate[DIK_ESCAPE] == (unsigned char)128) {
					PostMessage(ctx->hWnd, CUSTOM_QUIT, 0, 0);
				}
			}
		}

		// Render Scene
		{
			glClear(GL_COLOR_BUFFER_BIT);

			//static float ngl = 0.0f;

			//ngl += 0.001f;

			//// scale is in radians
			//glm::mat4x4 World = glm::scale(glm::vec3(sin(ngl), sin(ngl), sin(ngl)));

			//glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World[0][0]);

			//glEnableVertexAttribArray(0);
			//glBindBuffer(GL_ARRAY_BUFFER, VBO);
			//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			//glDrawArrays(GL_TRIANGLES, 0, 3);

			//glDisableVertexAttribArray(0);

			static float Scale = 0.0f;

			Scale += 0.001f;

			glm::mat4x4 World;

			World[0][0] = cosf(Scale); World[0][1] = 0.0f; World[0][2] = -sinf(Scale); World[0][3] = 0.0f;
			World[1][0] = 0.0;         World[1][1] = 1.0f; World[1][2] = 0.0f; World[1][3] = 0.0f;
			World[2][0] = sinf(Scale); World[2][1] = 0.0f; World[2][2] = cosf(Scale); World[2][3] = 0.0f;
			World[3][0] = 0.0f;        World[3][1] = 0.0f; World[3][2] = 0.0f; World[3][3] = 1.0f;

			glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World[0][0]);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

			glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

			glDisableVertexAttribArray(0);

			SwapBuffers(hdc);

		}

		fprintf(log, "FPS %.1f\n", FramesPerSecond);

	}

	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(hrc);

	return 0;
}
