
#define GLM_ENABLE_EXPERIMENTAL
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)
#define GLEW_STATIC

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <Magick++.h>
#include "dddcommon.h"
#include "opengli.h"

namespace SHOGL {

	HWND g_hWnd = nullptr;
	HDC g_hdc = nullptr;
	HGLRC g_hglrc = nullptr;

	struct TriangleMesh {
		GLuint VBA;
		GLuint IBA;
		GLuint NumIndexes;
		GLenum Winding;
		glm::mat4x4 model;
	};
	TriangleMesh *meshes = nullptr;
	GLuint NumMeshes = 0;
	GLuint ShaderProgram = 0;
	GLuint WorldLocation = 0;
	GLuint gSampler = 0;
	GLuint TextureMe = 0;

	typedef struct {
		glm::vec3 position;
		glm::vec4 color;
		glm::vec3 normal;
		glm::vec2 texture;
	} glmvec12;

	DDDCOMMON::UserLocation loc = { 0.0f, 0.0f, 0.0f, 6.0f, 0.0f };

	unsigned int gScreenWidth = 0, gScreenHeight = 0;

	bool TextureLoad(const char* m_fileName, GLenum m_textureTarget, GLuint TextureObjectId)
	{
		Magick::Image m_image;
		Magick::Blob m_blob;

		try {
			m_image.read(m_fileName);
			m_image.write(&m_blob, "RGBA");
		}
		catch (Magick::Error& Error) {
			return false;
		}

		glBindTexture(m_textureTarget, TextureObjectId);
		glTexImage2D(m_textureTarget, 0, GL_RGBA, (GLsizei)m_image.columns(), (GLsizei)m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
		glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(m_textureTarget, 0);

		return true;
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

	BOOL Init(HWND hWnd, unsigned int ScreenWidth, unsigned int ScreenHeight)
	{
		gScreenWidth = ScreenWidth;
		gScreenHeight = ScreenHeight;

		g_hWnd = hWnd;

		g_hdc = GetDC(hWnd);
		SetDCPixelFormat(g_hdc);

		g_hglrc = wglCreateContext(g_hdc);
		if (FALSE == wglMakeCurrent(g_hdc, g_hglrc)) return FALSE;

		glewInit();

		return TRUE;
	}

	BOOL AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
	{

		GLuint ShaderObj = glCreateShader(ShaderType);

		if (ShaderObj == 0) return FALSE;

		const GLchar* p[1];
		p[0] = pShaderText;
		GLint Lengths[1];
		Lengths[0] = (GLint)strlen(pShaderText);
		glShaderSource(ShaderObj, 1, p, Lengths);
		glCompileShader(ShaderObj);
		GLint success;
		glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar InfoLog[1024];
			glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
			return FALSE;
		}

		glAttachShader(ShaderProgram, ShaderObj);
	}

	BOOL CompileShaders(const char* vsFilename, const char* fsFilename, GLuint* lpShaderProgram)
	{
		GLuint ShaderProgram = glCreateProgram();
		size_t sourceSize = 0;
		char* sourceCode = nullptr;

		if (ShaderProgram == 0) return FALSE;

		sourceCode = DDDCOMMON::ReadTextFile(vsFilename, &sourceSize);
		if (FALSE == AddShader(ShaderProgram, sourceCode, GL_VERTEX_SHADER)) {
			free(sourceCode);
			return FALSE;
		}

		sourceCode = DDDCOMMON::ReadTextFile(fsFilename, &sourceSize);
		if (FALSE == AddShader(ShaderProgram, sourceCode, GL_FRAGMENT_SHADER)) {
			free(sourceCode);
			return FALSE;
		}

		GLint Success = 0;
		GLchar ErrorLog[1024] = { 0 };

		glLinkProgram(ShaderProgram);
		glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
		if (Success == 0) {
			glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
			return FALSE;
		}

		glValidateProgram(ShaderProgram);
		glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
		if (!Success) {
			glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
			return FALSE;
		}

		glUseProgram(ShaderProgram);

		*lpShaderProgram = ShaderProgram;

		return TRUE;
	}

	BOOL InitPipeline(void)
	{
		glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		if (FALSE == CompileShaders("vshader.glsl", "fshader.glsl", &ShaderProgram)) return FALSE;

		WorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");

		gSampler = glGetUniformLocation(ShaderProgram, "gSampler");

		glUniform1i(gSampler, 0);

		glGenTextures(1, &TextureMe);
		TextureLoad("c:\\temp\\me.jpg", GL_TEXTURE_2D, TextureMe);

		return TRUE;
	}

	//BOOL InitGraphics(void)
	//{
	//	glmvec12 GeometryVertices[] =
	//	{
	//		{ glm::vec3(-5.0f, -5.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
	//		{ glm::vec3(-5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
	//		{ glm::vec3(5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
	//		{ glm::vec3(5.0f, -5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
	//		{ glm::vec3(-5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
	//		{ glm::vec3(5.0f,  5.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },

	//		{ glm::vec3(-10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
	//		{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
	//		{ glm::vec3(10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
	//		{ glm::vec3(10.0f, -5.0f,  10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },
	//		{ glm::vec3(-10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
	//		{ glm::vec3(10.0f, -5.0f, -10.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f) }
	//	};

	//	glGenBuffers(1, &VBA);
	//	glBindBuffer(GL_ARRAY_BUFFER, VBA);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(GeometryVertices), GeometryVertices, GL_STATIC_DRAW);

	//	unsigned int IndexArray[] = {
	//		0, 1, 2,
	//		3, 4, 5,
	//		6, 7, 8,
	//		9, 10, 11
	//	};

	//	glGenBuffers(1, &IBA);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBA);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndexArray), IndexArray, GL_STATIC_DRAW);

	//	NumIndices = 12;

	//	return TRUE;
	//}

	BOOL InitTextures(void)
	{
		return TRUE;
	}

	BOOL RenderFrame(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4x4 unTransposedWorldMatrix = glm::mat4x4(1.0f)
			* glm::perspective(glm::radians(45.0f), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 500.0f)
			* glm::lookAt(
				glm::vec3(loc.ex, loc.ey, loc.ez),
				glm::vec3(loc.ex - sinf(DEG2RAD(loc.azimuth)), loc.ey - sinf(DEG2RAD(loc.elevation)), loc.ez - cosf(DEG2RAD(loc.azimuth))),
				glm::vec3(0.0f, 1.0f, 0.0));

		for (int MeshIndex = 0; MeshIndex < NumMeshes; MeshIndex++) {

			glFrontFace(meshes[MeshIndex].Winding);

			glm::mat4x4 ThisWorld = unTransposedWorldMatrix * meshes[MeshIndex].model;
			//glUniformMatrix4fv(WorldLocation, 1, GL_FALSE, &unTransposedWorldMatrix[0][0]);
			glUniformMatrix4fv(WorldLocation, 1, GL_FALSE, &ThisWorld[0][0]);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);

			glActiveTexture(GL_TEXTURE0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[MeshIndex].IBA);
			glBindBuffer(GL_ARRAY_BUFFER, meshes[MeshIndex].VBA);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)12);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)28);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glmvec12), (const GLvoid*)40);

			glBindTexture(GL_TEXTURE_2D, TextureMe);

			glDrawElements(GL_TRIANGLES, meshes[MeshIndex].NumIndexes, GL_UNSIGNED_INT, 0);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);

		}

		SwapBuffers(g_hdc);

		return TRUE;
	}

	BOOL UpdateFrame(HWND hWnd)
	{
		ProcessInput(&loc, hWnd);
		return TRUE;
	}

	void Cleanup()
	{
		if (meshes != nullptr) free(meshes);
		if (g_hdc != nullptr) {
			wglMakeCurrent(g_hdc, nullptr);
		}
		if (g_hglrc != nullptr) {
			wglDeleteContext(g_hglrc);
		}
		if (g_hdc != nullptr) {
			ReleaseDC(g_hWnd, g_hdc);
		}
	}

	BOOL InitGraphicsA(DDDCOMMON::TriangleMeshConfig* configs, int NumConfigs)
	{

		GLuint *VBAArray = nullptr; 
		GLuint *IBAArray = nullptr;
		glmvec12 *GeometryVertices = nullptr;
		UINT* IndexArray = nullptr;

		meshes = (TriangleMesh*)malloc(NumConfigs * sizeof(TriangleMesh));
		memset(meshes, 0, NumConfigs * sizeof(TriangleMesh));
		NumMeshes = NumConfigs;

		VBAArray = (GLuint*)malloc(NumConfigs * sizeof(GLuint));
		memset(VBAArray, 0, NumConfigs * sizeof(GLuint));
		glGenBuffers(NumConfigs, VBAArray);

		IBAArray = (GLuint*)malloc(NumConfigs * sizeof(GLuint));
		memset(IBAArray, 0, NumConfigs * sizeof(GLuint));
		glGenBuffers(NumConfigs, IBAArray);

		for (int c = 0; c < NumConfigs; c++) {

			DDDCOMMON::TriangleMeshConfig *m = configs + c;

			GeometryVertices = (glmvec12*)malloc(m->NumPositions * sizeof(glmvec12));
			memset(GeometryVertices, 0, m->NumPositions * sizeof(glmvec12));
			for (int i = 0; i < m->NumPositions; i++) {
				GeometryVertices[i].position = m->positions[i];
				GeometryVertices[i].normal = glm::normalize(m->normals[i]);
				GeometryVertices[i].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				GeometryVertices[i].texture = glm::vec2(0.5f, 0.5f);
			}

			glBindBuffer(GL_ARRAY_BUFFER, VBAArray[c]);
			glBufferData(GL_ARRAY_BUFFER, m->NumPositions * sizeof(glmvec12), GeometryVertices, GL_STATIC_DRAW);
			meshes[c].VBA = VBAArray[c];

			free(GeometryVertices);

			IndexArray = (UINT*)malloc(m->NumIndexes * sizeof(UINT));
			memset(IndexArray, 0, m->NumIndexes * sizeof(UINT));
			for (int i = 0; i < m->NumIndexes; i++) IndexArray[i] = m->indexes[i];

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBAArray[c]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->NumIndexes * sizeof(UINT), IndexArray, GL_STATIC_DRAW);
			meshes[c].IBA = IBAArray[c];

			free(IndexArray);

			meshes[c].NumIndexes = m->NumIndexes;
			meshes[c].Winding = (m->winding == DDDCOMMON::MeshConfigWinding::Clockwise ? GL_CW : GL_CCW);
			meshes[c].model = m->model;
		}

		free(VBAArray);
		free(IBAArray);

		return TRUE;
	}
}
