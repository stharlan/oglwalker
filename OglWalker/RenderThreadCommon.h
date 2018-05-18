#pragma once

void SetDCPixelFormat(HDC hdc);
void ChangeSize(GLsizei w, GLsizei h);
void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType, FILE* log);
bool ReadMouseState(DIMOUSESTATE *pMouseState);
bool ReadKeyboardState(unsigned char* keystate);
void SetPerspective(int w, int h);
void SetOrtho(int w, int h);
char* CommonReadFile(const char* filename);
bool TextureLoad(const char* m_fileName, GLenum m_textureTarget, GLuint TextureObjectId);
GLuint CompileShaders(const char* vs1, const char* fs1, FILE* log);