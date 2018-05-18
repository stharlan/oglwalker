
#include "stdafx.h"

IDirectInputDevice8 *pMouse = NULL;
IDirectInputDevice8 *pKeyboard = NULL;

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
	GLfloat fAspect;
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
	fAspect = (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60.0f, fAspect, 1.0, 400.0);
	glOrtho(0.0, (GLfloat)w, 0.0, (GLfloat)h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType, FILE* log)
{

	fprintf(log, "creating shader %s\n", pShaderText);
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		exit(1);
	}

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
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
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

char* CommonReadFile(const char* filename)
{
	FILE *fin = NULL;
	fopen_s(&fin, "c:\\temp\\objects.json", "r");
	fseek(fin, 0, SEEK_END);
	unsigned int flen = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	char* content = (char*)malloc(flen + 1);
	ZeroMemory(content, flen + 1);
	fread_s(content, flen + 1, flen, 1, fin);
	fclose(fin);
	return content;
}

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

	//glGenTextures(1, lpm_textureObj);
	glBindTexture(m_textureTarget, TextureObjectId);
	glTexImage2D(m_textureTarget, 0, GL_RGBA, (GLsizei)m_image.columns(), (GLsizei)m_image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
	glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(m_textureTarget, 0);

	return true;
}

GLuint CompileShaders(const char* vs1, const char* fs1, FILE* log)
{
	fprintf(log, "compiling shaders\n");

	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(log, "Error creating shader program\n");
		exit(1);
	}

	fprintf(log, "adding shaders\n");
	AddShader(ShaderProgram, vs1, GL_VERTEX_SHADER, log);
	AddShader(ShaderProgram, fs1, GL_FRAGMENT_SHADER, log);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	fprintf(log, "linking shaders\n");
	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(log, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(log, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	return ShaderProgram;
}
