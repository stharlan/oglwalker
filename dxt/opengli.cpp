
#define GLM_ENABLE_EXPERIMENTAL
#define PI 3.14159f
#define DEG2RAD(x) (x * PI / 180.0f)
#define GLEW_STATIC

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
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
		UINT VBAIndex;
		UINT IBAIndex;
		GLuint NumIndexes;
		GLenum Winding;
		glm::mat4x4 model;
		GLuint MeshTextureId;
	};
	TriangleMesh *meshes = nullptr;
	GLuint NumMeshes = 0;
	GLuint gShaderProgram = 0;
	//GLuint gPerspectiveViewMatrix = 0;
	//GLuint gModelMatrix = 0;
	GLuint u_PMatrix;
	GLuint u_VMatrix;
	GLuint u_MMatrix;

	GLuint gLightPosId = 0;
	GLuint gSampler = 0;

	typedef struct {
		glm::vec3 position;
		glm::vec4 color;
		glm::vec3 normal;
		glm::vec2 texture;
		float lightmag;
	} glmvec13;

	DDDCOMMON::UserLocation loc = { 0.0f, 0.0f, 0.0f, 6.0f, 0.0f };

	unsigned int gScreenWidth = 0, gScreenHeight = 0;

	GLuint* lpTexA = nullptr;
	UINT NumTextures = 0;

	GLuint *VIBufferArray = nullptr;
	UINT NumVIBuffers = 0;

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
		glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

		return TRUE;
	}

	BOOL CompileShaders(const char* vsFilename, const char* fsFilename, GLuint* lpShaderProgram)
	{
		GLuint ShaderProgram = glCreateProgram();
		size_t sourceSize = 0;
		char* sourceCode = nullptr;

		if (ShaderProgram == 0) return FALSE;

		sourceCode = DDDCOMMON::ReadTextFile(vsFilename, &sourceSize);
		if (FALSE == AddShader(ShaderProgram, sourceCode, GL_VERTEX_SHADER)) {
			MessageBox(nullptr, "Vertex shader failed to compile.", "ERROR", MB_OK);
			free(sourceCode);
			return FALSE;
		}

		sourceCode = DDDCOMMON::ReadTextFile(fsFilename, &sourceSize);
		if (FALSE == AddShader(ShaderProgram, sourceCode, GL_FRAGMENT_SHADER)) {
			MessageBox(nullptr, "Pixel shader failed to compile.", "ERROR", MB_OK);
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
		std::ofstream glinfo("glinfo.txt");
		const GLubyte* answer = glGetString(GL_VENDOR); glinfo << "Vendor: " << answer << std::endl;
		answer = glGetString(GL_RENDERER); glinfo << "Renderer: " << answer << std::endl;
		answer = glGetString(GL_VERSION); glinfo << "Version: " << answer << std::endl;
		answer = glGetString(GL_SHADING_LANGUAGE_VERSION); glinfo << "GLSL Version: " << answer << std::endl;
		answer = glGetString(GL_EXTENSIONS); glinfo << "Extensions: " << answer << std::endl;

		glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		if (FALSE == CompileShaders("vshader2.glsl", "fshader2.glsl", &gShaderProgram)) return FALSE;

		//gPerspectiveViewMatrix = glGetUniformLocation(gShaderProgram, "gPerspectiveViewMatrix");

		//gModelMatrix = glGetUniformLocation(gShaderProgram, "gModelMatrix");

		u_PMatrix = glGetUniformLocation(gShaderProgram, "u_PMatrix");
		u_VMatrix = glGetUniformLocation(gShaderProgram, "u_VMatrix");
		u_MMatrix = glGetUniformLocation(gShaderProgram, "u_MMatrix");

		gLightPosId = glGetUniformLocation(gShaderProgram, "gLightPos");

		gSampler = glGetUniformLocation(gShaderProgram, "gSampler");

		glUniform1i(gSampler, 0);

		return TRUE;
	}

	BOOL RenderFrame(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 EyeLoc = glm::vec3(loc.ex, loc.ey, loc.ez);
		glm::vec3 EyeNormal = glm::vec3(loc.ex - sinf(DEG2RAD(loc.azimuth)), loc.ey - sinf(DEG2RAD(loc.elevation)), loc.ez - cosf(DEG2RAD(loc.azimuth)));

		//glm::mat4x4 unTransposedWorldMatrix = glm::mat4x4(1.0f)
			//* glm::perspective(glm::radians(45.0f), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 500.0f)
			//* glm::lookAt(
				//EyeLoc,
				//EyeNormal,
				//glm::vec3(0.0f, 1.0f, 0.0));

		// set the perspective view matrix
		//glUniformMatrix4fv(gPerspectiveViewMatrix, 1, GL_FALSE, &unTransposedWorldMatrix[0][0]);

		glm::mat4x4 pMatrix = glm::mat4x4(1.0f) *
			glm::perspective(glm::radians(45.0f), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 500.0f);
		glUniformMatrix4fv(u_PMatrix, 1, GL_FALSE, &pMatrix[0][0]);

		glm::mat4x4 vMatrix = glm::mat4x4(1.0f) *
			glm::lookAt(EyeLoc, EyeNormal, glm::vec3(0.0f, 1.0f, 0.0));
		glUniformMatrix4fv(u_VMatrix, 1, GL_FALSE, &vMatrix[0][0]);

		//glm::vec3 LightPos(0.0f, 4.0f, 0.0f);
		glUniform3fv(gLightPosId, 1, &EyeLoc[0]);

		for (UINT MeshIndex = 0; MeshIndex < NumMeshes; MeshIndex++) {

			glFrontFace(meshes[MeshIndex].Winding);

			// set the model matrix
			//glUniformMatrix4fv(gModelMatrix, 1, GL_FALSE, &meshes[MeshIndex].model[0][0]);
			glUniformMatrix4fv(u_MMatrix, 1, GL_FALSE, &meshes[MeshIndex].model[0][0]);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);

			glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, lpTexA[meshes[MeshIndex].MeshTextureId]);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIBufferArray[meshes[MeshIndex].IBAIndex]);
			glBindBuffer(GL_ARRAY_BUFFER, VIBufferArray[meshes[MeshIndex].VBAIndex]);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec13), (const GLvoid*)0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glmvec13), (const GLvoid*)12);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glmvec13), (const GLvoid*)28);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glmvec13), (const GLvoid*)40);
			glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(glmvec13), (const GLvoid*)48);

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
		glDeleteBuffers(NumVIBuffers, VIBufferArray);
		glDeleteTextures(NumTextures, lpTexA);
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

		glmvec13 *GeometryVertices13 = nullptr;
		UINT* IndexArray = nullptr;

		meshes = (TriangleMesh*)malloc(NumConfigs * sizeof(TriangleMesh));
		memset(meshes, 0, NumConfigs * sizeof(TriangleMesh));
		NumMeshes = NumConfigs;

		VIBufferArray = (GLuint*)malloc(NumConfigs * 2 * sizeof(GLuint));
		memset(VIBufferArray, 0, NumConfigs * 2 * sizeof(GLuint));
		glGenBuffers(NumConfigs * 2, VIBufferArray);

		for (int c = 0; c < NumConfigs; c++) {

			DDDCOMMON::TriangleMeshConfig *m = configs + c;

			GeometryVertices13 = (glmvec13*)malloc(m->NumPositions * sizeof(glmvec13));
			memset(GeometryVertices13, 0, m->NumPositions * sizeof(glmvec13));
			for (UINT i = 0; i < m->NumPositions; i++) {
				GeometryVertices13[i].position = m->positions[i];
				GeometryVertices13[i].normal = glm::normalize(m->normals[i]);
				GeometryVertices13[i].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				if (m->NumTexCoords > 0) {
					GeometryVertices13[i].texture = m->texcoords[i];
				}
				else {
					GeometryVertices13[i].texture = glm::vec2(0.5f, 0.5f);
				}

				// to find the light magnitude
				// 1. transpose the position using the model matrix (model matrix * pos4 (1.0))
				// 2. calc light_pos - pos and normalize
				// 3. transpose the normal vector usnig the model matrix (model matrix * nrm4 (0.0))
				// 4. calc dot of 2 and 3
				// 5. clamp 0 to 1

				GeometryVertices13[i].lightmag = 1.0f;
			}

			glBindBuffer(GL_ARRAY_BUFFER, VIBufferArray[c]);
			glBufferData(GL_ARRAY_BUFFER, m->NumPositions * sizeof(glmvec13), GeometryVertices13, GL_STATIC_DRAW);
			meshes[c].VBAIndex = c;

			free(GeometryVertices13);

			IndexArray = (UINT*)malloc(m->NumIndexes * sizeof(UINT));
			memset(IndexArray, 0, m->NumIndexes * sizeof(UINT));
			for (UINT i = 0; i < m->NumIndexes; i++) IndexArray[i] = m->indexes[i];

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIBufferArray[NumConfigs + c]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->NumIndexes * sizeof(UINT), IndexArray, GL_STATIC_DRAW);
			meshes[c].IBAIndex = NumConfigs + c;

			free(IndexArray);

			meshes[c].NumIndexes = m->NumIndexes;
			meshes[c].Winding = (m->winding == DDDCOMMON::MeshConfigWinding::Clockwise ? GL_CW : GL_CCW);
			meshes[c].model = m->model;
			meshes[c].MeshTextureId = m->TextureId;

		}

		NumVIBuffers = NumConfigs;

		return TRUE;
	}

	BOOL InitTextures(std::vector<std::string> TextureFilenameList)
	{
		if (TextureFilenameList.size() < 1) return TRUE;
		lpTexA = (GLuint*)malloc(TextureFilenameList.size() * sizeof(GLuint));
		if (lpTexA == nullptr) return FALSE;
		memset(lpTexA, 0, TextureFilenameList.size() * sizeof(GLuint));
		glGenTextures(TextureFilenameList.size(), lpTexA);
		for (int i = 0; i < TextureFilenameList.size(); i++) {
			TextureLoad(TextureFilenameList[i].c_str(), GL_TEXTURE_2D, lpTexA[i]);
		}
		NumTextures = TextureFilenameList.size();
		return TRUE;
	}
}
