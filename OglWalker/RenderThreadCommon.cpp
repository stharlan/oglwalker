
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

void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
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
