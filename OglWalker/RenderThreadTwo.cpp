
#include "stdafx.h"

#pragma comment(lib, "CORE_RL_Magick++_.lib")
#pragma comment(lib, "CORE_RL_MagickCore_.lib")

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
"Color = vec4(1.0, 0.0, 0.0, 1.0); }";
//"Color = vec4(clamp(Position, 0.0, 1.0), 1.0); }";

const char* fs = 
"#version 330\r\n"
"in vec4 Color;\r\n"
"out vec4 FragColor;\r\n"
"void main()\r\n"
"{ FragColor = Color; }";

static void CreateVertexBuffer()
{
	glm::vec3 Vertices[7];
	Vertices[0] = glm::vec3(0, 0, 0);
	Vertices[1] = glm::vec3(1, 0, 0);
	Vertices[2] = glm::vec3(0, 1, 0);
	Vertices[3] = glm::vec3(-25, 0, -25);
	Vertices[4] = glm::vec3(25, 0, -25);
	Vertices[5] = glm::vec3(25, 0, 0);
	Vertices[6] = glm::vec3(-25, 0, 0);
	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	unsigned int Indices[] = { 
		0, 1, 1, 2, 2, 0,
		3, 4, 4, 6, 6, 3,
		4, 5, 5, 6, 6, 4
	};

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

bool TextureLoad(const char* m_fileName, GLenum m_textureTarget, GLuint* lpm_textureObj)
{
	Magick::Image m_image;
	//mImage.read("c:\\temp\\black_tile.jpg");
	Magick::Blob m_blob;
	//mImage.write(&mBlob, "RGBA");

	try {
		m_image.read(m_fileName);
		m_image.write(&m_blob, "RGBA");
	}
	catch (Magick::Error& Error) {
		return false;
	}

	glGenTextures(1, lpm_textureObj);
	glBindTexture(m_textureTarget, *lpm_textureObj);
	glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.columns(), m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
	glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(m_textureTarget, 0);

	return true;
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
	float ex = 0.0f, ez = 0.0f;
	float azimuth = 180.0f;
	GLuint Texture1;

	GetClientRect(ctx->hWnd, &rect);

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

	TextureLoad("c:\\temp\\black_tile.jpg", GL_TEXTURE_2D, &Texture1);

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
				azimuth -= (mouseState.lX / 8.0f);
			}
			if (TRUE == ReadKeyboardState(&keystate[0]))
			{
				if (keystate[DIK_ESCAPE] == (unsigned char)128) {
					PostMessage(ctx->hWnd, CUSTOM_QUIT, 0, 0);
				}
				if (keystate[DIK_W] == (unsigned char)128) ez -= 0.001f;
				if (keystate[DIK_S] == (unsigned char)128) ez += 0.001f;
				if (keystate[DIK_A] == (unsigned char)128) ex -= 0.001f;
				if (keystate[DIK_D] == (unsigned char)128) ex += 0.001f;
			}
		}

		// Render Scene
		{
			glClear(GL_COLOR_BUFFER_BIT);

			glm::mat4 projmat;
			glm::mat4 modelmat;
			glm::mat4 ModelViewProjectionMatrix;

			projmat = glm::mat4(1.0);
			projmat *= glm::perspective(45.0f, (float)rect.right / (float)rect.bottom, 0.1f, 400.0f);

			modelmat = glm::mat4(1.0);
			// eye, center, up
			float azimuth_in_radians = DEG2RAD(azimuth);
			modelmat *= glm::lookAt(
				glm::vec3(ex, 6.0f, ez), // eye, my eye position
				glm::vec3(ex + sinf(azimuth_in_radians), 6.0f, ez + cosf(azimuth_in_radians)),
				glm::vec3(0.0f, 1.0f, 0.0f)); // up vector

			ModelViewProjectionMatrix = projmat * modelmat;

			glUniformMatrix4fv(gWorldLocation, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

			//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			glDrawElements(GL_LINES, 18, GL_UNSIGNED_INT, 0);

			glDisableVertexAttribArray(0);

			SwapBuffers(hdc);

		}

		fprintf(log, "FPS %.1f\n", FramesPerSecond);

	}

	wglMakeCurrent(hdc, NULL);
	wglDeleteContext(hrc);

	return 0;
}
