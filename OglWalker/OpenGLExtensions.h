#pragma once

#define GL_ARRAY_BUFFER 34962 

namespace glext {

	typedef void(*pfnglGenBuffers)(GLsizei, GLuint*);
	typedef void(*pfnglDeleteBuffers)(GLsizei, const GLuint*);
	typedef void(*pfnglBindBuffer)(GLenum, GLuint);

	extern pfnglGenBuffers glGenBuffers;
	extern pfnglDeleteBuffers glDeleteBuffers;
	extern pfnglBindBuffer glBindBuffer;

	void InitializeExtensions();

}