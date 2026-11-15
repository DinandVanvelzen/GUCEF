#if defined(__APPLE__)
	#include <OpenGL/gl3.h>
#elif defined(_WIN32)
	// DV edit: glew, which is part of this code base, instead of glad
	#include <GL/glew.h>
#else
	#include <GL/gl.h>
#endif
