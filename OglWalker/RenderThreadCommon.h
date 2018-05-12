#pragma once

void SetDCPixelFormat(HDC hdc);
void ChangeSize(GLsizei w, GLsizei h);
void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
bool ReadMouseState(DIMOUSESTATE *pMouseState);
bool ReadKeyboardState(unsigned char* keystate);
void SetPerspective(int w, int h);
void SetOrtho(int w, int h);