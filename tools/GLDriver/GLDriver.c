/*
 * Copyright (C) Dinand Vanvelzen. 2002 - 2004.  All rights reserved.
 *
 * All source code herein is the property of Dinand Vanvelzen. You may not sell
 * or otherwise commercially exploit the source or things you created based on
 * the source.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL DINAND VANVELZEN BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>         /* debug assertion utilities */
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>         /* memory management */
#endif
#include "GLDriver.h"       /* function ptr extern refrences and includes */

#pragma warning( disable: 4100 ) /* unrefrenced formal parameter */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define UInt32  unsigned long
#define Float32 float

/*-------------------------------------------------------------------------*/

/*
 *   OpenGL uses the standard calling convention.
 *   The following macro will adapt the syntax to your compiler   
 */
#undef GLDRIVER_CALLSPEC_PREFIX
#undef GLDRIVER_CALLSPEC_SUFFIX  
#if ( defined ( __BORLANDC__ ) || defined ( _MSC_VER ) )
  #define GLDRIVER_CALLSPEC_PREFIX __stdcall
  #define GLDRIVER_CALLSPEC_SUFFIX
#else
  #define GLDRIVER_CALLSPEC_PREFIX
  #define GLDRIVER_CALLSPEC_SUFFIX  __attribute__((stdcall))
#endif

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_MSWIN_BUILD
  #define glGetProcAddress    wglGetProcAddress
#endif

/*-------------------------------------------------------------------------*/

/*
 *      Undef all the macros used in debug mode to link to the debug OpenGL
 *      1.1 functions so that they dont redirect here causing a loop.
 */
#ifdef GLDRIVER_DEBUG_MODE
#undef glAccum
#undef glAlphaFunc
#undef glAreTexturesResident
#undef glArrayElement
#undef glBegin
#undef glBindTexture
#undef glBitmap
#undef glBlendFunc
#undef glCallList
#undef glCallLists
#undef glClear
#undef glClearAccum
#undef glClearColor
#undef glClearDepth
#undef glClearIndex
#undef glClearStencil
#undef glClipPlane
#undef glColor3b
#undef glColor3bv
#undef glColor3d
#undef glColor3dv
#undef glColor3f
#undef glColor3fv
#undef glColor3i
#undef glColor3iv
#undef glColor3s
#undef glColor3sv
#undef glColor3ub
#undef glColor3ubv
#undef glColor3ui
#undef glColor3uiv
#undef glColor3us
#undef glColor3usv
#undef glColor4b
#undef glColor4bv
#undef glColor4d
#undef glColor4dv
#undef glColor4f
#undef glColor4fv
#undef glColor4i
#undef glColor4iv
#undef glColor4s
#undef glColor4sv
#undef glColor4ub
#undef glColor4ubv
#undef glColor4ui
#undef glColor4uiv
#undef glColor4us
#undef glColor4usv
#undef glColorMask
#undef glColorMaterial
#undef glColorPointer
#undef glCopyPixels
#undef glCopyTexImage1D
#undef glCopyTexImage2D
#undef glCopyTexSubImage1D
#undef glCopyTexSubImage2D
#undef glCullFace
#undef glDeleteLists
#undef glDeleteTextures
#undef glDepthFunc
#undef glDepthMask
#undef glDepthRange
#undef glDisable
#undef glDisableClientState
#undef glDrawArrays
#undef glDrawBuffer
#undef glDrawElements
#undef glDrawPixels
#undef glEdgeFlag
#undef glEdgeFlagPointer
#undef glEdgeFlagv
#undef glEnable
#undef glEnableClientState
#undef glEnd
#undef glEndList
#undef glEvalCoord1d
#undef glEvalCoord1dv
#undef glEvalCoord1f
#undef glEvalCoord1fv
#undef glEvalCoord2d
#undef glEvalCoord2dv
#undef glEvalCoord2f
#undef glEvalCoord2fv
#undef glEvalMesh1
#undef glEvalMesh2
#undef glEvalPoint1
#undef glEvalPoint2
#undef glFeedbackBuffer
#undef glFinish
#undef glFlush
#undef glFogf
#undef glFogfv
#undef glFogi
#undef glFogiv
#undef glFrontFace
#undef glFrustum
#undef glGenLists
#undef glGenTextures
#undef glGetBooleanv
#undef glGetClipPlane
#undef glGetDoublev
#undef glGetError
#undef glGetFloatv
#undef glGetIntegerv
#undef glGetLightfv
#undef glGetLightiv
#undef glGetMapdv
#undef glGetMapfv
#undef glGetMapiv
#undef glGetMaterialfv
#undef glGetMaterialiv
#undef glGetPixelMapfv
#undef glGetPixelMapuiv
#undef glGetPixelMapusv
#undef glGetPointerv
#undef glGetPolygonStipple
#undef glGetString
#undef glGetTexEnvfv
#undef glGetTexEnviv
#undef glGetTexGendv
#undef glGetTexGenfv
#undef glGetTexGeniv
#undef glGetTexImage
#undef glGetTexLevelParameterfv
#undef glGetTexLevelParameteriv
#undef glGetTexParameterfv
#undef glGetTexParameteriv
#undef glHint
#undef glIndexMask
#undef glIndexPointer
#undef glIndexd
#undef glIndexdv
#undef glIndexf
#undef glIndexfv
#undef glIndexi
#undef glIndexiv
#undef glIndexs
#undef glIndexsv
#undef glIndexub
#undef glIndexubv
#undef glInitNames
#undef glInterleavedArrays
#undef glIsEnabled
#undef glIsList
#undef glIsTexture
#undef glLightModelf
#undef glLightModelfv
#undef glLightModeli
#undef glLightModeliv
#undef glLightf
#undef glLightfv
#undef glLighti
#undef glLightiv
#undef glLineStipple
#undef glLineWidth
#undef glListBase
#undef glLoadIdentity
#undef glLoadMatrixd
#undef glLoadMatrixf
#undef glLoadName
#undef glLogicOp
#undef glMap1d
#undef glMap1f
#undef glMap2d
#undef glMap2f
#undef glMapGrid1d
#undef glMapGrid1f
#undef glMapGrid2d
#undef glMapGrid2f
#undef glMaterialf
#undef glMaterialfv
#undef glMateriali
#undef glMaterialiv
#undef glMatrixMode
#undef glMultMatrixd
#undef glMultMatrixf
#undef glNewList
#undef glNormal3b
#undef glNormal3bv
#undef glNormal3d
#undef glNormal3dv
#undef glNormal3f
#undef glNormal3fv
#undef glNormal3i
#undef glNormal3iv
#undef glNormal3s
#undef glNormal3sv
#undef glNormalPointer
#undef glOrtho
#undef glPassThrough
#undef glPixelMapfv
#undef glPixelMapuiv
#undef glPixelMapusv
#undef glPixelStoref
#undef glPixelStorei
#undef glPixelTransferf
#undef glPixelTransferi
#undef glPixelZoom
#undef glPointSize
#undef glPolygonMode
#undef glPolygonOffset
#undef glPolygonStipple
#undef glPopAttrib
#undef glPopClientAttrib
#undef glPopMatrix
#undef glPopName
#undef glPrioritizeTextures
#undef glPushAttrib
#undef glPushClientAttrib
#undef glPushMatrix
#undef glPushName
#undef glRasterPos2d
#undef glRasterPos2dv
#undef glRasterPos2f
#undef glRasterPos2fv
#undef glRasterPos2i
#undef glRasterPos2iv
#undef glRasterPos2s
#undef glRasterPos2sv
#undef glRasterPos3d
#undef glRasterPos3dv
#undef glRasterPos3f
#undef glRasterPos3fv
#undef glRasterPos3i
#undef glRasterPos3iv
#undef glRasterPos3s
#undef glRasterPos3sv
#undef glRasterPos4d
#undef glRasterPos4dv
#undef glRasterPos4f
#undef glRasterPos4fv
#undef glRasterPos4i
#undef glRasterPos4iv
#undef glRasterPos4s
#undef glRasterPos4sv
#undef glReadBuffer
#undef glReadPixels
#undef glRectd
#undef glRectdv
#undef glRectf
#undef glRectfv
#undef glRecti
#undef glRectiv
#undef glRects
#undef glRectsv
#undef glRenderMode
#undef glRotated
#undef glRotatef
#undef glScaled
#undef glScalef
#undef glScissor
#undef glSelectBuffer
#undef glShadeModel
#undef glStencilFunc
#undef glStencilMask
#undef glStencilOp
#undef glTexCoord1d
#undef glTexCoord1dv
#undef glTexCoord1f
#undef glTexCoord1fv
#undef glTexCoord1i
#undef glTexCoord1iv
#undef glTexCoord1s
#undef glTexCoord1sv
#undef glTexCoord2d
#undef glTexCoord2dv
#undef glTexCoord2f
#undef glTexCoord2fv
#undef glTexCoord2i
#undef glTexCoord2iv
#undef glTexCoord2s
#undef glTexCoord2sv
#undef glTexCoord3d
#undef glTexCoord3dv
#undef glTexCoord3f
#undef glTexCoord3fv
#undef glTexCoord3i
#undef glTexCoord3iv
#undef glTexCoord3s
#undef glTexCoord3sv
#undef glTexCoord4d
#undef glTexCoord4dv
#undef glTexCoord4f
#undef glTexCoord4fv
#undef glTexCoord4i
#undef glTexCoord4iv
#undef glTexCoord4s
#undef glTexCoord4sv
#undef glTexCoordPointer
#undef glTexEnvf
#undef glTexEnvfv
#undef glTexEnvi
#undef glTexEnviv
#undef glTexGend
#undef glTexGendv
#undef glTexGenf
#undef glTexGenfv
#undef glTexGeni
#undef glTexGeniv
#undef glTexImage1D
#undef glTexImage2D
#undef glTexParameterf
#undef glTexParameterfv
#undef glTexParameteri
#undef glTexParameteriv
#undef glTexSubImage1D
#undef glTexSubImage2D
#undef glTranslated
#undef glTranslatef
#undef glVertex2d
#undef glVertex2dv
#undef glVertex2f
#undef glVertex2fv
#undef glVertex2i
#undef glVertex2iv
#undef glVertex2s
#undef glVertex2sv
#undef glVertex3d
#undef glVertex3dv
#undef glVertex3f
#undef glVertex3fv
#undef glVertex3i
#undef glVertex3iv
#undef glVertex3s
#undef glVertex3sv
#undef glVertex4d
#undef glVertex4dv
#undef glVertex4f
#undef glVertex4fv
#undef glVertex4i
#undef glVertex4iv
#undef glVertex4s
#undef glVertex4sv
#undef glVertexPointer
#undef glViewport

#endif /* GLDRIVER_DEBUG_MODE ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Global vars used to store driver info.
 */
static int status = GLDRIVER_FALSE;
static char *drvlib = NULL;
static float glversion = 0;
static GLenum glerror = GL_NO_ERROR;

#ifdef GLDRIVER_DEBUG_MODE
static GLboolean bes = 0;   /* are we in a glBegin() - glEnd() segment ? */
static TOGLErrorCallback userhandler = NULL;
#endif /* GLDRIVER_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

/*-------------------------------------------------------------------------*/

/*
 *      Function pointers for all the OpenGL 1.1 functions.
 */
void GLDRIVER_CALLSPEC_PREFIX (*glAccum) (GLenum op, GLfloat value) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glAlphaFunc) (GLenum func, GLclampf ref) GLDRIVER_CALLSPEC_SUFFIX = NULL;
GLboolean GLDRIVER_CALLSPEC_PREFIX (*glAreTexturesResident) (GLsizei n, const GLuint *textures, GLboolean *residences) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glArrayElement) (GLint i) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBegin) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBindTexture) (GLenum target, GLuint texture) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBitmap) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBlendFunc) (GLenum sfactor, GLenum dfactor) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCallList) (GLuint list) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCallLists) (GLsizei n, GLenum type, const GLvoid *lists) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClear) (GLbitfield mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClearAccum) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClearDepth) (GLclampd depth) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClearIndex) (GLfloat c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClearStencil) (GLint s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClipPlane) (GLenum plane, const GLdouble *equation) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3b) (GLbyte red, GLbyte green, GLbyte blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3bv) (const GLbyte *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3d) (GLdouble red, GLdouble green, GLdouble blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3f) (GLfloat red, GLfloat green, GLfloat blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3i) (GLint red, GLint green, GLint blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3s) (GLshort red, GLshort green, GLshort blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3ub) (GLubyte red, GLubyte green, GLubyte blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3ubv) (const GLubyte *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3ui) (GLuint red, GLuint green, GLuint blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3uiv) (const GLuint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3us) (GLushort red, GLushort green, GLushort blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor3usv) (const GLushort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4b) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4bv) (const GLbyte *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4d) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4i) (GLint red, GLint green, GLint blue, GLint alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4s) (GLshort red, GLshort green, GLshort blue, GLshort alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4ubv) (const GLubyte *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4ui) (GLuint red, GLuint green, GLuint blue, GLuint alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4uiv) (const GLuint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4us) (GLushort red, GLushort green, GLushort blue, GLushort alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColor4usv) (const GLushort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColorMaterial) (GLenum face, GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCopyTexImage1D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCullFace) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDeleteLists) (GLuint list, GLsizei range) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDeleteTextures) (GLsizei n, const GLuint *textures) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDepthFunc) (GLenum func) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDepthMask) (GLboolean flag) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDepthRange) (GLclampd zNear, GLclampd zFar) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDisable) (GLenum cap) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDisableClientState) (GLenum array) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDrawArrays) (GLenum mode, GLint first, GLsizei count) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDrawBuffer) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDrawPixels) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEdgeFlag) (GLboolean flag) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEdgeFlagPointer) (GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEdgeFlagv) (const GLboolean *flag) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEnable) (GLenum cap) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEnableClientState) (GLenum array) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEnd) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEndList) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord1d) (GLdouble u) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord1dv) (const GLdouble *u) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord1f) (GLfloat u) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord1fv) (const GLfloat *u) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord2d) (GLdouble u, GLdouble v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord2dv) (const GLdouble *u) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord2f) (GLfloat u, GLfloat v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalCoord2fv) (const GLfloat *u) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalMesh1) (GLenum mode, GLint i1, GLint i2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalMesh2) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalPoint1) (GLint i) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEvalPoint2) (GLint i, GLint j) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFeedbackBuffer) (GLsizei size, GLenum type, GLfloat *buffer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFinish) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFlush) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogf) (GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogfv) (GLenum pname, const GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogi) (GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogiv) (GLenum pname, const GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFrontFace) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) GLDRIVER_CALLSPEC_SUFFIX = NULL;
GLuint GLDRIVER_CALLSPEC_PREFIX (*glGenLists) (GLsizei range) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGenTextures) (GLsizei n, GLuint *textures) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetBooleanv) (GLenum pname, GLboolean *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetClipPlane) (GLenum plane, GLdouble *equation) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetDoublev) (GLenum pname, GLdouble *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
GLenum GLDRIVER_CALLSPEC_PREFIX (*glGetError) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetFloatv) (GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetIntegerv) (GLenum pname, GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetLightfv) (GLenum light, GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetLightiv) (GLenum light, GLenum pname, GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetMapdv) (GLenum target, GLenum query, GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetMapfv) (GLenum target, GLenum query, GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetMapiv) (GLenum target, GLenum query, GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetMaterialfv) (GLenum face, GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetMaterialiv) (GLenum face, GLenum pname, GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetPixelMapfv) (GLenum map, GLfloat *values) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetPixelMapuiv) (GLenum map, GLuint *values) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetPixelMapusv) (GLenum map, GLushort *values) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetPointerv) (GLenum pname, GLvoid* *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetPolygonStipple) (GLubyte *mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
const GLubyte * GLDRIVER_CALLSPEC_PREFIX (*glGetString) (GLenum name) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexEnvfv) (GLenum target, GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexEnviv) (GLenum target, GLenum pname, GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexGendv) (GLenum coord, GLenum pname, GLdouble *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexGenfv) (GLenum coord, GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexGeniv) (GLenum coord, GLenum pname, GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetTexParameteriv) (GLenum target, GLenum pname, GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glHint) (GLenum target, GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexMask) (GLuint mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexPointer) (GLenum type, GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexd) (GLdouble c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexdv) (const GLdouble *c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexf) (GLfloat c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexfv) (const GLfloat *c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexi) (GLint c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexiv) (const GLint *c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexs) (GLshort c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexsv) (const GLshort *c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexub) (GLubyte c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIndexubv) (const GLubyte *c) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glInitNames) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glInterleavedArrays) (GLenum format, GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
GLboolean GLDRIVER_CALLSPEC_PREFIX (*glIsEnabled) (GLenum cap) GLDRIVER_CALLSPEC_SUFFIX = NULL;
GLboolean GLDRIVER_CALLSPEC_PREFIX (*glIsList) (GLuint list) GLDRIVER_CALLSPEC_SUFFIX = NULL;
GLboolean GLDRIVER_CALLSPEC_PREFIX (*glIsTexture) (GLuint texture) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLightModelf) (GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLightModelfv) (GLenum pname, const GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLightModeli) (GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLightModeliv) (GLenum pname, const GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLightf) (GLenum light, GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLightfv) (GLenum light, GLenum pname, const GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLighti) (GLenum light, GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLightiv) (GLenum light, GLenum pname, const GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLineStipple) (GLint factor, GLushort pattern) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLineWidth) (GLfloat width) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glListBase) (GLuint base) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLoadIdentity) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLoadMatrixd) (const GLdouble *m) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLoadMatrixf) (const GLfloat *m) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLoadName) (GLuint name) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLogicOp) (GLenum opcode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMap1d) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMap1f) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMap2d) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMap2f) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMapGrid1d) (GLint un, GLdouble u1, GLdouble u2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMapGrid1f) (GLint un, GLfloat u1, GLfloat u2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMapGrid2d) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMapGrid2f) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMaterialf) (GLenum face, GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMaterialfv) (GLenum face, GLenum pname, const GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMateriali) (GLenum face, GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMaterialiv) (GLenum face, GLenum pname, const GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMatrixMode) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultMatrixd) (const GLdouble *m) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultMatrixf) (const GLfloat *m) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNewList) (GLuint list, GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3b) (GLbyte nx, GLbyte ny, GLbyte nz) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3bv) (const GLbyte *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3d) (GLdouble nx, GLdouble ny, GLdouble nz) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3i) (GLint nx, GLint ny, GLint nz) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3s) (GLshort nx, GLshort ny, GLshort nz) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormal3sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glNormalPointer) (GLenum type, GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPassThrough) (GLfloat token) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelMapfv) (GLenum map, GLsizei mapsize, const GLfloat *values) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelMapuiv) (GLenum map, GLsizei mapsize, const GLuint *values) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelMapusv) (GLenum map, GLsizei mapsize, const GLushort *values) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelStoref) (GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelStorei) (GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelTransferf) (GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelTransferi) (GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPixelZoom) (GLfloat xfactor, GLfloat yfactor) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPointSize) (GLfloat size) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPolygonMode) (GLenum face, GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPolygonOffset) (GLfloat factor, GLfloat units) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPolygonStipple) (const GLubyte *mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPopAttrib) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPopClientAttrib) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPopMatrix) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPopName) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPrioritizeTextures) (GLsizei n, const GLuint *textures, const GLclampf *priorities) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPushAttrib) (GLbitfield mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPushClientAttrib) (GLbitfield mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPushMatrix) (void) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPushName) (GLuint name) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2d) (GLdouble x, GLdouble y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2f) (GLfloat x, GLfloat y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2i) (GLint x, GLint y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2s) (GLshort x, GLshort y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos2sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3d) (GLdouble x, GLdouble y, GLdouble z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3f) (GLfloat x, GLfloat y, GLfloat z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3i) (GLint x, GLint y, GLint z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3s) (GLshort x, GLshort y, GLshort z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos3sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4i) (GLint x, GLint y, GLint z, GLint w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRasterPos4sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glReadBuffer) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRectdv) (const GLdouble *v1, const GLdouble *v2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRectfv) (const GLfloat *v1, const GLfloat *v2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRecti) (GLint x1, GLint y1, GLint x2, GLint y2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRectiv) (const GLint *v1, const GLint *v2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRectsv) (const GLshort *v1, const GLshort *v2) GLDRIVER_CALLSPEC_SUFFIX = NULL;
GLint GLDRIVER_CALLSPEC_PREFIX (*glRenderMode) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glScaled) (GLdouble x, GLdouble y, GLdouble z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glScalef) (GLfloat x, GLfloat y, GLfloat z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glScissor) (GLint x, GLint y, GLsizei width, GLsizei height) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSelectBuffer) (GLsizei size, GLuint *buffer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glShadeModel) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glStencilFunc) (GLenum func, GLint ref, GLuint mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glStencilMask) (GLuint mask) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1d) (GLdouble s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1f) (GLfloat s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1i) (GLint s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1s) (GLshort s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord1sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2d) (GLdouble s, GLdouble t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2f) (GLfloat s, GLfloat t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2i) (GLint s, GLint t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2s) (GLshort s, GLshort t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord2sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3d) (GLdouble s, GLdouble t, GLdouble r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3f) (GLfloat s, GLfloat t, GLfloat r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3i) (GLint s, GLint t, GLint r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3s) (GLshort s, GLshort t, GLshort r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord3sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4d) (GLdouble s, GLdouble t, GLdouble r, GLdouble q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4f) (GLfloat s, GLfloat t, GLfloat r, GLfloat q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4i) (GLint s, GLint t, GLint r, GLint q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4s) (GLshort s, GLshort t, GLshort r, GLshort q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoord4sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexEnvf) (GLenum target, GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexEnvfv) (GLenum target, GLenum pname, const GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexEnvi) (GLenum target, GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexEnviv) (GLenum target, GLenum pname, const GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexGend) (GLenum coord, GLenum pname, GLdouble param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexGendv) (GLenum coord, GLenum pname, const GLdouble *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexGenf) (GLenum coord, GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexGenfv) (GLenum coord, GLenum pname, const GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexGeni) (GLenum coord, GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexGeniv) (GLenum coord, GLenum pname, const GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexParameterf) (GLenum target, GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexParameteri) (GLenum target, GLenum pname, GLint param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexParameteriv) (GLenum target, GLenum pname, const GLint *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTranslated) (GLdouble x, GLdouble y, GLdouble z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTranslatef) (GLfloat x, GLfloat y, GLfloat z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2d) (GLdouble x, GLdouble y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2f) (GLfloat x, GLfloat y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2i) (GLint x, GLint y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2s) (GLshort x, GLshort y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex2sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3d) (GLdouble x, GLdouble y, GLdouble z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3f) (GLfloat x, GLfloat y, GLfloat z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3i) (GLint x, GLint y, GLint z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3s) (GLshort x, GLshort y, GLshort z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex3sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4i) (GLint x, GLint y, GLint z, GLint w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4s) (GLshort x, GLshort y, GLshort z, GLshort w) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertex4sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glViewport) (GLint x, GLint y, GLsizei width, GLsizei height) GLDRIVER_CALLSPEC_SUFFIX = NULL;

/*-------------------------------------------------------------------------*/

/*
 *      Function pointers for all the OpenGL 1.2 functions.
 */
void GLDRIVER_CALLSPEC_PREFIX (*glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexImage3D) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels) GLDRIVER_CALLSPEC_SUFFIX = NULL;

/*-------------------------------------------------------------------------*/

/*
 *      Function pointers for all the OpenGL 1.3 functions.
 */
void GLDRIVER_CALLSPEC_PREFIX (*glActiveTexture) (GLenum texture) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glClientActiveTexture) (GLenum texture) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCompressedTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCompressedTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetCompressedTexImage) (GLenum target, GLint lod, GLvoid *img) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLoadTransposeMatrixd) (const GLdouble m[16]) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glLoadTransposeMatrixf) (const GLfloat m[16]) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultTransposeMatrixd) (const GLdouble m[16]) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultTransposeMatrixf) (const GLfloat m[16]) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1d) (GLenum target, GLdouble s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1dv) (GLenum target, const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1f) (GLenum target, GLfloat s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1fv) (GLenum target, const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1i) (GLenum target, GLint s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1iv) (GLenum target, const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1s) (GLenum target, GLshort s) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord1sv) (GLenum target, const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2d) (GLenum target, GLdouble s, GLdouble t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2dv) (GLenum target, const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2f) (GLenum target, GLfloat s, GLfloat t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2fv) (GLenum target, const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2i) (GLenum target, GLint s, GLint t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2iv) (GLenum target, const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2s) (GLenum target, GLshort s, GLshort t) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord2sv) (GLenum target, const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3d) (GLenum target, GLdouble s, GLdouble t, GLdouble r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3dv) (GLenum target, const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3f) (GLenum target, GLfloat s, GLfloat t, GLfloat r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3fv) (GLenum target, const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3i) (GLenum target, GLint s, GLint t, GLint r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3iv) (GLenum target, const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3s) (GLenum target, GLshort s, GLshort t, GLshort r) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord3sv) (GLenum target, const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4d) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4dv) (GLenum target, const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4f) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4fv) (GLenum target, const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4i) (GLenum target, GLint s, GLint t, GLint r, GLint q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4iv) (GLenum target, const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4s) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiTexCoord4sv) (GLenum target, const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSampleCoverage) (GLclampf value, GLboolean invert) GLDRIVER_CALLSPEC_SUFFIX = NULL;

/*-------------------------------------------------------------------------*/

/*
 *      Function pointers for all the OpenGL 1.4 functions.
 */
void GLDRIVER_CALLSPEC_PREFIX (*glBlendColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBlendEquation) (GLenum mode) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBlendFuncSeparate) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogCoordPointer) (GLenum type, GLsizei stride, const GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogCoordd) (GLdouble coord) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogCoorddv) (const GLdouble *coord) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogCoordf) (GLfloat coord) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glFogCoordfv) (const GLfloat *coord) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiDrawArrays) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMultiDrawElements) (GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPointParameterf) (GLenum pname, GLfloat param) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glPointParameterfv) (GLenum pname, GLfloat *params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3b) (GLbyte red, GLbyte green, GLbyte blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3bv) (const GLbyte *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3d) (GLdouble red, GLdouble green, GLdouble blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3dv) (const GLdouble *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3f) (GLfloat red, GLfloat green, GLfloat blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3fv) (const GLfloat *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3i) (GLint red, GLint green, GLint blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3iv) (const GLint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3s) (GLshort red, GLshort green, GLshort blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3sv) (const GLshort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3ub) (GLubyte red, GLubyte green, GLubyte blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3ubv) (const GLubyte *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3ui) (GLuint red, GLuint green, GLuint blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3uiv) (const GLuint *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3us) (GLushort red, GLushort green, GLushort blue) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColor3usv) (const GLushort *v) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glSecondaryColorPointer) (GLint size, GLenum type, GLsizei stride, GLvoid *pointer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2d) (GLdouble x, GLdouble y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2dv) (const GLdouble *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2f) (GLfloat x, GLfloat y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2fv) (const GLfloat *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2i) (GLint x, GLint y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2iv) (const GLint *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2s) (GLshort x, GLshort y) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos2sv) (const GLshort *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3d) (GLdouble x, GLdouble y, GLdouble z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3dv) (const GLdouble *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3f) (GLfloat x, GLfloat y, GLfloat z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3fv) (const GLfloat *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3i) (GLint x, GLint y, GLint z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3iv) (const GLint *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3s) (GLshort x, GLshort y, GLshort z) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glWindowPos3sv) (const GLshort *p) GLDRIVER_CALLSPEC_SUFFIX = NULL;

/*-------------------------------------------------------------------------*/

/*
 *      Function pointers for all the OpenGL 1.5 functions.
 */
void GLDRIVER_CALLSPEC_PREFIX (*glBeginQuery) (GLenum target, GLuint id) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBindBuffer) (GLenum target, GLuint buffer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBufferData) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDeleteBuffers) (GLsizei n, const GLuint* buffers) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glDeleteQueries) (GLsizei n, const GLuint* ids) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glEndQuery) (GLenum target) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGenBuffers) (GLsizei n, GLuint* buffers) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGenQueries) (GLsizei n, GLuint* ids) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetBufferParameteriv) (GLenum target, GLenum pname, GLint* params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetBufferPointerv) (GLenum target, GLenum pname, GLvoid** params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetQueryObjectiv) (GLuint id, GLenum pname, GLint* params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint* params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glGetQueryiv) (GLenum target, GLenum pname, GLint params) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIsBuffer) (GLuint buffer) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glIsQuery) (GLuint id) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glMapBuffer) (GLenum target, GLenum access) GLDRIVER_CALLSPEC_SUFFIX = NULL;
void GLDRIVER_CALLSPEC_PREFIX (*glUnmapBuffer) (GLenum target) GLDRIVER_CALLSPEC_SUFFIX = NULL;

/*-------------------------------------------------------------------------*/

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DEBUG_MODE

/*-------------------------------------------------------------------------*/

void 
glNoOpErrorHandler( GLenum error       ,
                    const char* glfunc ,
                    const char* file   ,
                    UInt32 line        ) GLDRIVER_CALLSPEC_SUFFIX
{
        /* Don't do anything */
}

/*-------------------------------------------------------------------------*/

GLenum GLDRIVER_CALLSPEC_PREFIX
glGetErrorDEBUG( const char* file ,
                 UInt32 line      ) GLDRIVER_CALLSPEC_SUFFIX
{
    /* glGetError() should not be called inbetween a call to glBegin and the corresponding call to glEnd. */
    assert( bes == 0 );
    
    glerror = glGetError();
    
    if ( glerror != GL_NO_ERROR )
    {
        userhandler( glerror, "glGetError()", file, line ); 
    }

    return glerror;
}

/*-------------------------------------------------------------------------*/

const char*
glGetErrorStringDEBUG( GLenum error )
{
        if ( error == GL_NO_ERROR ) return NULL;
        switch ( error )
        {
                case GL_INVALID_ENUM :
                {
                        return "GL_INVALID_ENUM";
                }
                case GL_INVALID_OPERATION :
                {
                        return "GL_INVALID_OPERATION";
                }
                case GL_INVALID_VALUE :
                {
                        return "GL_INVALID_VALUE";
                }
                case GL_STACK_OVERFLOW :
                {
                        return "GL_STACK_OVERFLOW";
                }
                case GL_STACK_UNDERFLOW :
                {
                        return "GL_STACK_UNDERFLOW";
                }
                case GL_OUT_OF_MEMORY :
                {
                        return "GL_OUT_OF_MEMORY";
                }
                default :
                {
                        return NULL;
                }
        }
}

/*-------------------------------------------------------------------------*/

/**
 *  Forward declaration of the glGetErrorDEBUG() function
 */
GLenum GLDRIVER_CALLSPEC_PREFIX
glGetErrorDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/*
 *      Debug versions of the OpenGL 1.1 functions
 *      They add error checking after each call and trace information when used
 *      in combination with the macros.
 */
void GLDRIVER_CALLSPEC_PREFIX glAccumDEBUG( GLenum op, GLfloat value, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glAccum( op, value ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glAccum()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glAlphaFuncDEBUG(GLenum func, GLclampf ref, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glAlphaFunc( func, ref ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glAlphaFunc()", file, line ); } }
GLboolean GLDRIVER_CALLSPEC_PREFIX glAreTexturesResidentDEBUG( GLsizei n, const GLuint *textures, GLboolean *residences, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { GLboolean retval = glAreTexturesResident( n, textures, residences ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glAreTexturesResident()", file, line ); } return retval; }
void GLDRIVER_CALLSPEC_PREFIX glArrayElementDEBUG( GLint i, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glArrayElement( i ); if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glArrayElement()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glBeginDEBUG( GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glBegin( mode ); bes = 1; }
void GLDRIVER_CALLSPEC_PREFIX glBindTextureDEBUG(GLenum target, GLuint texture, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glBindTexture( target, texture ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glBindTexture()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glBitmapDEBUG(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glBitmap( width, height, xorig, yorig, xmove, ymove, bitmap ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glBitmap()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glBlendFuncDEBUG(GLenum sfactor, GLenum dfactor, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glBlendFunc( sfactor, dfactor ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glBlendFunc()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCallListDEBUG(GLuint list, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCallList( list ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCallList()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCallListsDEBUG(GLsizei n, GLenum type, const GLvoid *lists, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCallLists( n, type, lists ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCallLists()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glClearDEBUG(GLbitfield mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glClear( mask ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glClear()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glClearAccumDEBUG(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glClearAccum( red, green, blue, alpha ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glClearAccum()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glClearColorDEBUG(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glClearColor( red, green, blue, alpha ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glClearColor()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glClearDepthDEBUG(GLclampd depth, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glClearDepth( depth ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glClearDepth()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glClearIndexDEBUG(GLfloat c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glClearIndex( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glClearIndex()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glClearStencilDEBUG(GLint s, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glClearStencil( s ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glClearStencil()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glClipPlaneDEBUG(GLenum plane, const GLdouble *equation, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glClipPlane( plane, equation ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glClipPlane()",file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glColor3bDEBUG(GLbyte red, GLbyte green, GLbyte blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3b( red, green, blue ); if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3b()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3bvDEBUG(const GLbyte *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3bv( v ); if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3bv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3dDEBUG(GLdouble red, GLdouble green, GLdouble blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3d( red, green, blue ); if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3d()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3dv( v ); if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3dv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3fDEBUG(GLfloat red, GLfloat green, GLfloat blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3f( red, green, blue ); if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3f()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3fv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3fv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3iDEBUG(GLint red, GLint green, GLint blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3i( red, green, blue );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3i()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3iv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3iv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3sDEBUG(GLshort red, GLshort green, GLshort blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3s( red, green, blue );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3s()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3sv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3sv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3ubDEBUG(GLubyte red, GLubyte green, GLubyte blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3ub( red, green, blue );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3ub()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3ubvDEBUG(const GLubyte *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3ubv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3ubv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3uiDEBUG(GLuint red, GLuint green, GLuint blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3ui( red, green, blue );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3ui()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3uivDEBUG(const GLuint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3uiv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3uiv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3usDEBUG(GLushort red, GLushort green, GLushort blue, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3us( red, green, blue );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3us()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor3usvDEBUG(const GLushort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor3usv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor3usv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4bDEBUG(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4b( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4b()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4bvDEBUG(const GLbyte *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4bv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4bv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4dDEBUG(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4d( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4d()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4dv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4dv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4fDEBUG(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4f( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4f()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4fv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4fv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4iDEBUG(GLint red, GLint green, GLint blue, GLint alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4i( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4i()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4iv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4iv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4sDEBUG(GLshort red, GLshort green, GLshort blue, GLshort alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4s( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4s()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4sv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4sv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4ubDEBUG(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4ub( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4ub()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4ubvDEBUG(const GLubyte *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4ubv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4ubv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4uiDEBUG(GLuint red, GLuint green, GLuint blue, GLuint alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4ui( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4ui()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4uivDEBUG(const GLuint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4uiv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4uiv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4usDEBUG(GLushort red, GLushort green, GLushort blue, GLushort alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4us( red, green, blue, alpha );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4us()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColor4usvDEBUG(const GLushort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColor4usv( v );  if ( !bes ) { glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColor4usv()", file, line ); } } }
void GLDRIVER_CALLSPEC_PREFIX glColorMaskDEBUG(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColorMask( red, green, blue, alpha ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColorMask()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glColorMaterialDEBUG(GLenum face, GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColorMaterial( face, mode ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColorMaterial()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glColorPointerDEBUG(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glColorPointer( size, type, stride, pointer ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glColorPointer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCopyPixelsDEBUG(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCopyPixels( x, y, width, height, type ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCopyPixels()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCopyTexImage1DDEBUG(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCopyTexImage1D( target, level, internalFormat, x, y, width, border ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCopyTexImage1D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCopyTexImage2DDEBUG(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCopyTexImage2D( target, level, internalFormat, x, y, width, height, border ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCopyTexImage2D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCopyTexSubImage1DDEBUG(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCopyTexSubImage1D( target, level, xoffset, x, y, width ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCopyTexSubImage1D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCopyTexSubImage2DDEBUG(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCopyTexSubImage2D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glCullFaceDEBUG(GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glCullFace( mode ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCullFace()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDeleteListsDEBUG(GLuint list, GLsizei range, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDeleteLists( list, range ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDeleteLists()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDeleteTexturesDEBUG(GLsizei n, const GLuint *textures, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDeleteTextures( n, textures ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDeleteTextures()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDepthFuncDEBUG(GLenum func, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDepthFunc( func ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDepthFunc()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDepthMaskDEBUG(GLboolean flag, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDepthMask( flag ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDepthMask()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDepthRangeDEBUG(GLclampd zNear, GLclampd zFar, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDepthRange( zNear, zFar ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDepthRange()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDisableDEBUG(GLenum capconst, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDisable( capconst ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDisable()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDisableClientStateDEBUG(GLenum array, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDisableClientState( array ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDisableClientState()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDrawArraysDEBUG(GLenum mode, GLint first, GLsizei countconst, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDrawArrays( mode, first, countconst ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCullFace()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDrawBufferDEBUG(GLenum modeconst, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDrawBuffer( modeconst ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDrawBuffer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDrawElementsDEBUG(GLenum mode, GLsizei count, GLenum type, const GLvoid *indicesconst, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDrawElements( mode, count, type, indicesconst ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDrawElements()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glDrawPixelsDEBUG(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixelsconst, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glDrawPixels( width, height, format, type, pixelsconst ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glDrawPixels()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEdgeFlagDEBUG(GLboolean flagconst, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEdgeFlag( flagconst ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEdgeFlag()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEdgeFlagPointerDEBUG(GLsizei stride, const GLvoid *pointer, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEdgeFlagPointer( stride, pointer ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEdgeFlagPointer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEdgeFlagvDEBUG(const GLboolean *flag, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEdgeFlagv( flag ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEdgeFlagv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEnableDEBUG(GLenum cap, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEnable( cap ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glCullFace()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEnableClientStateDEBUG(GLenum array, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEnableClientState( array ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEnableClientState()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEndDEBUG( const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEnd(); bes = 0; glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEnd()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEndListDEBUG( const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEndList(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEndList()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord1dDEBUG(GLdouble u, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord1d( u ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord1d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord1dvDEBUG(const GLdouble *u, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord1dv( u ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord1dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord1fDEBUG(GLfloat u, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord1f( u ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord1f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord1fvDEBUG(const GLfloat *u, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord1fv( u ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord1fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord2dDEBUG(GLdouble u, GLdouble v, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord2d( u, v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord2d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord2dvDEBUG(const GLdouble *u, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord2dv( u ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord2dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord2fDEBUG(GLfloat u, GLfloat v, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord2f( u, v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord2f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalCoord2fvDEBUG(const GLfloat *u, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalCoord2fv( u ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalCoord2fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalMesh1DEBUG(GLenum mode, GLint i1, GLint i2, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalMesh1( mode, i1, i2 ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalMesh1()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalMesh2DEBUG(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalMesh2( mode, i1, i2, j1, j2 ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalMesh2()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalPoint1DEBUG(GLint i, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalPoint1( i ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalPoint1()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glEvalPoint2DEBUG(GLint i, GLint j, const char* file,  UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glEvalPoint2( i, j ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glEvalPoint2()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFeedbackBufferDEBUG(GLsizei size, GLenum type, GLfloat *buffer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFeedbackBuffer(size, type, buffer); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFeedbackBuffer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFinishDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFinish(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFinish()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFlushDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFlush(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFlush()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFogfDEBUG(GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFogf( pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFogf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFogfvDEBUG(GLenum pname, const GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFogfv(pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFogfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFogiDEBUG(GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFogi( pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFogi()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFogivDEBUG(GLenum pname, const GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFogiv(pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFogiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFrontFaceDEBUG(GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFrontFace(mode); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFrontFace()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glFrustumDEBUG(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glFrustum(left, right, bottom, top, zNear, zFar); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glFrustum()", file, line ); } }
GLuint GLDRIVER_CALLSPEC_PREFIX glGenListsDEBUG(GLsizei range, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { GLuint retVal = glGenLists( range); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGenLists()", file, line ); } return retVal; }
void GLDRIVER_CALLSPEC_PREFIX glGenTexturesDEBUG(GLsizei n, GLuint *textures, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGenTextures( n, textures); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGenTextures()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetBooleanvDEBUG(GLenum pname, GLboolean *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetBooleanv( pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetBooleanv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetClipPlaneDEBUG(GLenum plane, GLdouble *equation, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetClipPlane( plane, equation); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetClipPlane()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetDoublevDEBUG(GLenum pname, GLdouble *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetDoublev( pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetDoublev()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetFloatvDEBUG(GLenum pname, GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetFloatv( pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetFloatv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetIntegervDEBUG(GLenum pname, GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetIntegerv( pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetIntegerv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetLightfvDEBUG(GLenum light, GLenum pname, GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetLightfv(light, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetLightfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetLightivDEBUG(GLenum light, GLenum pname, GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetLightiv(light, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetLightiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetMapdvDEBUG(GLenum target, GLenum query, GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetMapdv(target, query, v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetMapdv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetMapfvDEBUG(GLenum target, GLenum query, GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetMapfv(target, query, v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetMapfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetMapivDEBUG(GLenum target, GLenum query, GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetMapiv(target, query, v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetMapiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetMaterialfvDEBUG(GLenum face, GLenum pname, GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetMaterialfv(face, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetMaterialfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetMaterialivDEBUG(GLenum face, GLenum pname, GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetMaterialiv(face, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetMaterialiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetPixelMapfvDEBUG(GLenum map, GLfloat *values, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetPixelMapfv( map, values); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetPixelMapfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetPixelMapuivDEBUG(GLenum map, GLuint *values, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetPixelMapuiv( map, values); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetPixelMapuiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetPixelMapusvDEBUG(GLenum map, GLushort *values, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetPixelMapusv( map, values); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetPixelMapusv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetPointervDEBUG(GLenum pname, GLvoid* *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetPointerv( pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetPointerv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetPolygonStippleDEBUG(GLubyte *mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetPolygonStipple(mask); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetPolygonStipple()", file, line ); } }
const GLubyte* GLDRIVER_CALLSPEC_PREFIX glGetStringDEBUG(GLenum name, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { const GLubyte* retValue = glGetString( name); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetString()", file, line ); } return retValue; }
void GLDRIVER_CALLSPEC_PREFIX glGetTexEnvfvDEBUG(GLenum target, GLenum pname, GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexEnvfv( target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexEnvfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexEnvivDEBUG(GLenum target, GLenum pname, GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexEnviv( target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexEnviv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexGendvDEBUG(GLenum coord, GLenum pname, GLdouble *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexGendv(coord, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexGendv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexGenfvDEBUG(GLenum coord, GLenum pname, GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexGenfv(coord, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexGenfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexGenivDEBUG(GLenum coord, GLenum pname, GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexGeniv( coord, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexGeniv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexImageDEBUG(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexImage(target, level, format, type, pixels); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexImage()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexLevelParameterfvDEBUG(GLenum target, GLint level, GLenum pname, GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexLevelParameterfv(target, level, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexLevelParameterfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexLevelParameterivDEBUG(GLenum target, GLint level, GLenum pname, GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexLevelParameteriv(target, level, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexLevelParameteriv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexParameterfvDEBUG(GLenum target, GLenum pname, GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexParameterfv( target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexParameterfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glGetTexParameterivDEBUG(GLenum target, GLenum pname, GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glGetTexParameteriv( target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glGetTexParameteriv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glHintDEBUG(GLenum target, GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glHint( target, mode); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glHint()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexMaskDEBUG(GLuint mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexMask(mask); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexMask()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexPointerDEBUG(GLenum type, GLsizei stride, const GLvoid *pointer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexPointer( type, stride, pointer); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexPointer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexdDEBUG(GLdouble c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexd(c); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexd()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexdvDEBUG(const GLdouble *c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexdv(c); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexdv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexfDEBUG(GLfloat c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexf( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexfvDEBUG(const GLfloat *c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexfv( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexiDEBUG(GLint c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexi(c); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexi()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexivDEBUG(const GLint *c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexiv( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexsDEBUG(GLshort c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexs( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexs()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexsvDEBUG(const GLshort *c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexsv( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexsv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexubDEBUG(GLubyte c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexub( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexub()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glIndexubvDEBUG(const GLubyte *c, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glIndexubv( c ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIndexubv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glInitNamesDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glInitNames(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glInitNames()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glInterleavedArraysDEBUG(GLenum format, GLsizei stride, const GLvoid *pointer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glInterleavedArrays(format, stride, pointer); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glInterleavedArrays()", file, line ); } }
GLboolean GLDRIVER_CALLSPEC_PREFIX glIsEnabledDEBUG(GLenum cap, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { GLboolean retValue = glIsEnabled( cap ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIsEnabled()", file, line ); } return retValue; }
GLboolean GLDRIVER_CALLSPEC_PREFIX glIsListDEBUG(GLuint list, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { GLboolean retValue = glIsList( list ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIsList()", file, line ); } return retValue; }
GLboolean GLDRIVER_CALLSPEC_PREFIX glIsTextureDEBUG(GLuint texture, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { GLboolean retValue = glIsTexture( texture ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glIsTexture()", file, line ); } return retValue; }
void GLDRIVER_CALLSPEC_PREFIX glLightModelfDEBUG(GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLightModelf(pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLightModelf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLightModelfvDEBUG(GLenum pname, const GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLightModelfv(pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLightModelfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLightModeliDEBUG(GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLightModeli(pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLightModeli()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLightModelivDEBUG(GLenum pname, const GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLightModeliv( pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLightModeliv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLightfDEBUG(GLenum light, GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLightf(light, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLightf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLightfvDEBUG(GLenum light, GLenum pname, const GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLightfv( light, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLightfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLightiDEBUG(GLenum light, GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLighti(light, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLighti()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLightivDEBUG(GLenum light, GLenum pname, const GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLightiv( light, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLightiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLineStippleDEBUG(GLint factor, GLushort pattern, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLineStipple( factor, pattern); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLineStipple()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLineWidthDEBUG(GLfloat width, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLineWidth(width); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLineWidth()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glListBaseDEBUG(GLuint base, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glListBase(base); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glListBase()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLoadIdentityDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLoadIdentity(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLoadIdentity()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLoadMatrixdDEBUG(const GLdouble *m, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLoadMatrixd(m); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLoadMatrixd()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLoadMatrixfDEBUG(const GLfloat *m, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLoadMatrixf( m ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLoadMatrixf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLoadNameDEBUG(GLuint name, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLoadName( name ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLoadName()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glLogicOpDEBUG(GLenum opcode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glLogicOp( opcode ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glLogicOp()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMap1dDEBUG(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMap1d(target, u1, u2, stride, order, points); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMap1d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMap1fDEBUG(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMap1f(target, u1, u2, stride, order, points); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMap1f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMap2dDEBUG(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMap2d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMap2fDEBUG(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMap2f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMapGrid1dDEBUG(GLint un, GLdouble u1, GLdouble u2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMapGrid1d(un, u1, u2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMapGrid1d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMapGrid1fDEBUG(GLint un, GLfloat u1, GLfloat u2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMapGrid1f(un, u1, u2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMapGrid1f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMapGrid2dDEBUG(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMapGrid2d(un, u1, u2, vn, v1, v2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMapGrid2d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMapGrid2fDEBUG(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMapGrid2f( un, u1, u2, vn, v1, v2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMapGrid2f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMaterialfDEBUG(GLenum face, GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMaterialf(face, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMaterialf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMaterialfvDEBUG(GLenum face, GLenum pname, const GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMaterialfv( face, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMaterialfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMaterialiDEBUG(GLenum face, GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMateriali( face, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMateriali()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMaterialivDEBUG(GLenum face, GLenum pname, const GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMaterialiv( face, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMaterialiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMatrixModeDEBUG(GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMatrixMode(mode); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMatrixMode()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMultMatrixdDEBUG(const GLdouble *m, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMultMatrixd( m ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMultMatrixd()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glMultMatrixfDEBUG(const GLfloat *m, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glMultMatrixf( m ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glMultMatrixf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNewListDEBUG(GLuint list, GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNewList( list, mode ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNewList()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3bDEBUG(GLbyte nx, GLbyte ny, GLbyte nz, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3b(nx, ny, nz); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3b()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3bvDEBUG(const GLbyte *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3bv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3bv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3dDEBUG(GLdouble nx, GLdouble ny, GLdouble nz, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3d( nx, ny, nz); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3dv( v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3fDEBUG(GLfloat nx, GLfloat ny, GLfloat nz, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3f( nx, ny, nz); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3iDEBUG(GLint nx, GLint ny, GLint nz, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3i( nx, ny, nz); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3iv( v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3sDEBUG(GLshort nx, GLshort ny, GLshort nz, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3s(nx, ny, nz); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormal3svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormal3sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormal3sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glNormalPointerDEBUG(GLenum type, GLsizei stride, const GLvoid *pointer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glNormalPointer( type, stride, pointer); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glNormalPointer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glOrthoDEBUG(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glOrtho(left, right, bottom, top, zNear, zFar); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glOrtho()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPassThroughDEBUG(GLfloat token, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPassThrough(token); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPassThrough()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelMapfvDEBUG(GLenum map, GLsizei mapsize, const GLfloat *values, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelMapfv( map, mapsize, values); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelMapfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelMapuivDEBUG(GLenum map, GLsizei mapsize, const GLuint *values, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelMapuiv( map, mapsize, values); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelMapuiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelMapusvDEBUG(GLenum map, GLsizei mapsize, const GLushort *values, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelMapusv(map, mapsize, values); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelMapusv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelStorefDEBUG(GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelStoref(pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelStoref()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelStoreiDEBUG(GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelStorei(pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelStorei()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelTransferfDEBUG(GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelTransferf(pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelTransferf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelTransferiDEBUG(GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelTransferi( pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelTransferi()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPixelZoomDEBUG(GLfloat xfactor, GLfloat yfactor, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPixelZoom(xfactor, yfactor); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPixelZoom()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPointSizeDEBUG(GLfloat size, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPointSize(size); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPointSize()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPolygonModeDEBUG(GLenum face, GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPolygonMode( face, mode); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPolygonMode()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPolygonOffsetDEBUG(GLfloat factor, GLfloat units, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPolygonOffset(factor, units); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPolygonOffset()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPolygonStippleDEBUG(const GLubyte *mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPolygonStipple(mask); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPolygonStipple()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPopAttribDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPopAttrib(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPopAttrib()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPopClientAttribDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPopClientAttrib(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPopClientAttrib()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPopMatrixDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPopMatrix(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPopMatrix()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPopNameDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPopName(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPopName()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPrioritizeTexturesDEBUG(GLsizei n, const GLuint *textures, const GLclampf *priorities, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPrioritizeTextures(n, textures, priorities); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPrioritizeTextures()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPushAttribDEBUG(GLbitfield mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPushAttrib( mask ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPushAttrib()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPushClientAttribDEBUG(GLbitfield mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPushClientAttrib( mask ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPushClientAttrib()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPushMatrixDEBUG( const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPushMatrix(); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPushMatrix()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glPushNameDEBUG(GLuint name, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glPushName( name ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glPushName()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2dDEBUG(GLdouble x, GLdouble y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2d(x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2fDEBUG(GLfloat x, GLfloat y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2f(x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2iDEBUG(GLint x, GLint y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2i(x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2sDEBUG(GLshort x, GLshort y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2s( x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos2svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos2sv( v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos2sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3dDEBUG(GLdouble x, GLdouble y, GLdouble z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3d( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3fDEBUG(GLfloat x, GLfloat y, GLfloat z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3f( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3fv( v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3iDEBUG(GLint x, GLint y, GLint z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3i( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3sDEBUG(GLshort x, GLshort y, GLshort z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3s( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos3svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos3sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos3sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4dDEBUG(GLdouble x, GLdouble y, GLdouble z, GLdouble w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4d(x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4dv( v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4fDEBUG(GLfloat x, GLfloat y, GLfloat z, GLfloat w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4f(x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4iDEBUG(GLint x, GLint y, GLint z, GLint w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4i(x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4sDEBUG(GLshort x, GLshort y, GLshort z, GLshort w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4s( x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRasterPos4svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRasterPos4sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRasterPos4sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glReadBufferDEBUG(GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glReadBuffer(mode); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glReadBuffer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glReadPixelsDEBUG(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glReadPixels(x, y, width, height, format, type, pixels); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glReadPixels()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectdDEBUG(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRectd( x1, y1, x2, y2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRectd()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectdvDEBUG(const GLdouble *v1, const GLdouble *v2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRectdv(v1, v2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRectdv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectfDEBUG(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRectf( x1, y1, x2, y2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRectf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectfvDEBUG(const GLfloat *v1, const GLfloat *v2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRectfv(v1,v2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRectfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectiDEBUG(GLint x1, GLint y1, GLint x2, GLint y2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRecti(x1, y1, x2, y2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRecti()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectivDEBUG(const GLint *v1, const GLint *v2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRectiv( v1, v2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRectiv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectsDEBUG(GLshort x1, GLshort y1, GLshort x2, GLshort y2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRects(x1, y1, x2, y2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRects()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRectsvDEBUG(const GLshort *v1, const GLshort *v2, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRectsv( v1, v2); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRectsv()", file, line ); } }
GLint GLDRIVER_CALLSPEC_PREFIX glRenderModeDEBUG(GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { GLint retValue = glRenderMode(mode); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRenderMode()", file, line ); } return retValue; }
void GLDRIVER_CALLSPEC_PREFIX glRotatedDEBUG(GLdouble angle, GLdouble x, GLdouble y, GLdouble z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRotated( angle, x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRotated()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glRotatefDEBUG(GLfloat angle, GLfloat x, GLfloat y, GLfloat z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glRotatef(angle, x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glRotatef()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glScaledDEBUG(GLdouble x, GLdouble y, GLdouble z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glScaled( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glScaled()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glScalefDEBUG(GLfloat x, GLfloat y, GLfloat z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glScalef(x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glScalef()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glScissorDEBUG(GLint x, GLint y, GLsizei width, GLsizei height, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glScissor(x, y, width, height); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glScissor()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glSelectBufferDEBUG(GLsizei size, GLuint *buffer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glSelectBuffer(size, buffer); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glSelectBuffer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glShadeModelDEBUG(GLenum mode, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glShadeModel(mode); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glShadeModel()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glStencilFuncDEBUG(GLenum func, GLint ref, GLuint mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glStencilFunc( func, ref, mask); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glStencilFunc()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glStencilMaskDEBUG(GLuint mask, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glStencilMask(mask); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glStencilMask()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glStencilOpDEBUG(GLenum fail, GLenum zfail, GLenum zpass, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glStencilOp(fail, zfail, zpass); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glStencilOp()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1dDEBUG(GLdouble s, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1d( s); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1fDEBUG(GLfloat s, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1f(s); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1iDEBUG(GLint s, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1i( s ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1sDEBUG(GLshort s, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1s(s); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord1svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord1sv( v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord1sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2dDEBUG(GLdouble s, GLdouble t, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2d( s, t); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2dv( v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2fDEBUG(GLfloat s, GLfloat t, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2f( s, t); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2fv( v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2iDEBUG(GLint s, GLint t, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2i( s, t); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2iv( v ); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2sDEBUG(GLshort s, GLshort t, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2s( s, t); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord2svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord2sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord2sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3dDEBUG(GLdouble s, GLdouble t, GLdouble r, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3d( s, t, r); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3fDEBUG(GLfloat s, GLfloat t, GLfloat r, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3f( s, t, r); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3iDEBUG(GLint s, GLint t, GLint r, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3i( s, t, r); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3sDEBUG(GLshort s, GLshort t, GLshort r, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3s(s, t, r); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord3svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord3sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord3sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4dDEBUG(GLdouble s, GLdouble t, GLdouble r, GLdouble q, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4d( s, t, r, q); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4fDEBUG(GLfloat s, GLfloat t, GLfloat r, GLfloat q, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4f( s, t, r, q); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4iDEBUG(GLint s, GLint t, GLint r, GLint q, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4i( s, t, r, q); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4sDEBUG(GLshort s, GLshort t, GLshort r, GLshort q, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4s(s, t, r, q); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoord4svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoord4sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoord4sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexCoordPointerDEBUG(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexCoordPointer(size, type, stride, pointer); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexCoordPointer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexEnvfDEBUG(GLenum target, GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexEnvf(target, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexEnvf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexEnvfvDEBUG(GLenum target, GLenum pname, const GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexEnvfv(target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexEnvfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexEnviDEBUG(GLenum target, GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexEnvi(target, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexEnvi()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexEnvivDEBUG(GLenum target, GLenum pname, const GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexEnviv( target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexEnviv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexGendDEBUG(GLenum coord, GLenum pname, GLdouble param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexGend( coord, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexGend()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexGendvDEBUG(GLenum coord, GLenum pname, const GLdouble *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexGendv(coord, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexGendv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexGenfDEBUG(GLenum coord, GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexGenf(coord, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexGenf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexGenfvDEBUG(GLenum coord, GLenum pname, const GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexGenfv(coord, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexGenfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexGeniDEBUG(GLenum coord, GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexGeni( coord, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexGeni()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexGenivDEBUG(GLenum coord, GLenum pname, const GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexGeniv(coord, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexGeniv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexImage1DDEBUG(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexImage1D(target, level, internalformat, width, border, format, type, pixels); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexImage1D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexImage2DDEBUG(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexImage2D( target, level, internalformat, width, height, border, format, type, pixels); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexImage2D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexParameterfDEBUG(GLenum target, GLenum pname, GLfloat param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexParameterf(target, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexParameterf()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexParameterfvDEBUG(GLenum target, GLenum pname, const GLfloat *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexParameterfv( target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexParameterfv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexParameteriDEBUG(GLenum target, GLenum pname, GLint param, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexParameteri(target, pname, param); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexParameteri()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexParameterivDEBUG(GLenum target, GLenum pname, const GLint *params, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexParameteriv(target, pname, params); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexParameteriv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexSubImage1DDEBUG(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexSubImage1D( target, level, xoffset, width, format, type, pixels); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexSubImage1D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTexSubImage2DDEBUG(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTexSubImage2D()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTranslatedDEBUG(GLdouble x, GLdouble y, GLdouble z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTranslated( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTranslated()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glTranslatefDEBUG(GLfloat x, GLfloat y, GLfloat z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glTranslatef( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glTranslatef()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2dDEBUG(GLdouble x, GLdouble y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2d(x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2fDEBUG(GLfloat x, GLfloat y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2f(x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2fv( v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2iDEBUG(GLint x, GLint y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2i(x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2iv( v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2sDEBUG(GLshort x, GLshort y, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2s(x, y); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex2svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex2sv( v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex2sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3dDEBUG(GLdouble x, GLdouble y, GLdouble z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3d( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3fDEBUG(GLfloat x, GLfloat y, GLfloat z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3f( x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3iDEBUG(GLint x, GLint y, GLint z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3i(x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3sDEBUG(GLshort x, GLshort y, GLshort z, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3s(x, y, z); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex3svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex3sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex3sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4dDEBUG(GLdouble x, GLdouble y, GLdouble z, GLdouble w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4d(x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4d()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4dvDEBUG(const GLdouble *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4dv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4dv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4fDEBUG(GLfloat x, GLfloat y, GLfloat z, GLfloat w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4f(x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4f()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4fvDEBUG(const GLfloat *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4fv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4fv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4iDEBUG(GLint x, GLint y, GLint z, GLint w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4i( x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4i()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4ivDEBUG(const GLint *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4iv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4iv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4sDEBUG(GLshort x, GLshort y, GLshort z, GLshort w, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4s(x, y, z, w); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4s()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertex4svDEBUG(const GLshort *v, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertex4sv(v); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertex4sv()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glVertexPointerDEBUG(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glVertexPointer(size, type, stride, pointer); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glVertexPointer()", file, line ); } }
void GLDRIVER_CALLSPEC_PREFIX glViewportDEBUG(GLint x, GLint y, GLsizei width, GLsizei height, const char* file, UInt32 line ) GLDRIVER_CALLSPEC_SUFFIX { glViewport(x, y, width, height); glGetError(); if ( glerror != GL_NO_ERROR ) { userhandler( glerror, "glViewport()", file, line ); } }

/*-------------------------------------------------------------------------*/

/**
 *      Sets the error callback function.
 *      Only available in debug mode !!!
 *      The callback function will be called if a call to a gl function
 *      generated a gl error. Trace information will be provided.
 */
void
glErrorHandler( TOGLErrorCallback callbackfunc )
{
        userhandler = callbackfunc;
}

/*-------------------------------------------------------------------------*/

#endif /* GLDRIVER_DEBUG_MODE ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

static int
glLoadOpenGL1_1Functions( void )
{
        /*
         *      Now that the library is loaded we will attempt to link all the
         *      gl* function pointers to the GL functions in the loaded library
         *      All these if statements may seem awfully inefficient but keep in
         *      mind that branch prediction will keep the overhead to a minimum.
         *      Plus we want to be sure each and every function is accounted for.
         */
        glAccum = glGetProcAddress( "glAccum" );
        if ( !glAccum ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glAlphaFunc = glGetProcAddress( "glAlphaFunc" );
        if ( !glAlphaFunc ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glAreTexturesResident = glGetProcAddress( "glAreTexturesResident" );
        if ( !glAreTexturesResident ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glArrayElement = glGetProcAddress( "glArrayElement" );
        if ( !glArrayElement ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBegin = glGetProcAddress( "glBegin" );
        if ( !glBegin ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBindTexture = glGetProcAddress( "glBindTexture" );
        if ( !glBindTexture ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBitmap = glGetProcAddress( "glBitmap" );
        if ( !glBitmap ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBlendFunc = glGetProcAddress( "glBlendFunc" );
        if ( !glBlendFunc ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCallList = glGetProcAddress( "glCallList" );
        if ( !glCallList ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCallLists = glGetProcAddress( "glCallLists" );
        if ( !glCallLists ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClear = glGetProcAddress( "glClear" );
        if ( !glClear ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClearAccum = glGetProcAddress( "glClearAccum" );
        if ( !glClearAccum ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClearColor = glGetProcAddress( "glClearColor" );
        if ( !glClearColor ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClearDepth = glGetProcAddress( "glClearDepth" );
        if ( !glClearDepth ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClearIndex = glGetProcAddress( "glClearIndex" );
        if ( !glClearIndex ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClearStencil = glGetProcAddress( "glClearStencil" );
        if ( !glClearStencil ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClipPlane = glGetProcAddress( "glClipPlane" );
        if ( !glClipPlane ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3b = glGetProcAddress( "glColor3b" );
        if ( !glColor3b ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3bv = glGetProcAddress( "glColor3bv" );
        if ( !glColor3bv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3d = glGetProcAddress( "glColor3d" );
        if ( !glColor3d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3dv = glGetProcAddress( "glColor3dv" );
        if ( !glColor3dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3f = glGetProcAddress( "glColor3f" );
        if ( !glColor3f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3fv = glGetProcAddress( "glColor3fv" );
        if ( !glColor3fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3i = glGetProcAddress( "glColor3i" );
        if ( !glColor3i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3iv = glGetProcAddress( "glColor3iv" );
        if ( !glColor3iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3s = glGetProcAddress( "glColor3s" );
        if ( !glColor3s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3sv = glGetProcAddress( "glColor3sv" );
        if ( !glColor3sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3ub = glGetProcAddress( "glColor3ub" );
        if ( !glColor3ub ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3ubv = glGetProcAddress( "glColor3ubv" );
        if ( !glColor3ubv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3ui = glGetProcAddress( "glColor3ui" );
        if ( !glColor3ui ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3uiv = glGetProcAddress( "glColor3uiv" );
        if ( !glColor3uiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3us = glGetProcAddress( "glColor3us" );
        if ( !glColor3us ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor3usv = glGetProcAddress( "glColor3usv" );
        if ( !glColor3usv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4b = glGetProcAddress( "glColor4b" );
        if ( !glColor4b ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4bv = glGetProcAddress( "glColor4bv" );
        if ( !glColor4bv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4b = glGetProcAddress( "glColor4b" );
        if ( !glColor4b ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4bv = glGetProcAddress( "glColor4bv" );
        if ( !glColor4bv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4d = glGetProcAddress( "glColor4d" );
        if ( !glColor4d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4dv = glGetProcAddress( "glColor4dv" );
        if ( !glColor4dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4f = glGetProcAddress( "glColor4f" );
        if ( !glColor4f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4fv = glGetProcAddress( "glColor4fv" );
        if ( !glColor4fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4i = glGetProcAddress( "glColor4i" );
        if ( !glColor4i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4iv = glGetProcAddress( "glColor4iv" );
        if ( !glColor4iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4s = glGetProcAddress( "glColor4s" );
        if ( !glColor4s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4sv = glGetProcAddress( "glColor4sv" );
        if ( !glColor4sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4ub = glGetProcAddress( "glColor4ub" );
        if ( !glColor4ub ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4ubv = glGetProcAddress( "glColor4ubv" );
        if ( !glColor4ubv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4ui = glGetProcAddress( "glColor4ui" );
        if ( !glColor4ui ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4uiv = glGetProcAddress( "glColor4uiv" );
        if ( !glColor4uiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4us = glGetProcAddress( "glColor4us" );
        if ( !glColor4us ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColor4usv = glGetProcAddress( "glColor4usv" );
        if ( !glColor4usv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColorMask = glGetProcAddress( "glColorMask" );
        if ( !glColorMask ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColorMaterial = glGetProcAddress( "glColorMaterial" );
        if ( !glColorMaterial ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColorPointer = glGetProcAddress( "glColorPointer" );
        if ( !glColorPointer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCopyPixels = glGetProcAddress( "glCopyPixels" );
        if ( !glCopyPixels ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColorMask = glGetProcAddress( "glColorMask" );
        if ( !glColorMask ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColorMaterial = glGetProcAddress( "glColorMaterial" );
        if ( !glColorMaterial ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glColorPointer = glGetProcAddress( "glColorPointer" );
        if ( !glColorPointer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCopyPixels = glGetProcAddress( "glCopyPixels" );
        if ( !glCopyPixels ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCopyTexImage1D = glGetProcAddress( "glCopyTexImage1D" );
        if ( !glCopyTexImage1D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCopyTexImage2D = glGetProcAddress( "glCopyTexImage2D" );
        if ( !glCopyTexImage2D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCopyTexSubImage1D = glGetProcAddress( "glCopyTexSubImage1D" );
        if ( !glCopyTexSubImage1D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCopyTexSubImage2D = glGetProcAddress( "glCopyTexSubImage2D" );
        if ( !glCopyTexSubImage2D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCullFace = glGetProcAddress( "glCullFace" );
        if ( !glCullFace ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDeleteLists = glGetProcAddress( "glDeleteLists" );
        if ( !glDeleteLists ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDeleteTextures = glGetProcAddress( "glDeleteTextures" );
        if ( !glDeleteTextures ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDepthFunc = glGetProcAddress( "glDepthFunc" );
        if ( !glDepthFunc ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDepthMask = glGetProcAddress( "glDepthMask" );
        if ( !glDepthMask ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDepthRange = glGetProcAddress( "glDepthRange" );
        if ( !glDepthRange ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDisable = glGetProcAddress( "glDisable" );
        if ( !glDisable ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDisableClientState = glGetProcAddress( "glDisableClientState" );
        if ( !glDisableClientState ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDrawArrays = glGetProcAddress( "glDrawArrays" );
        if ( !glDrawArrays ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDrawBuffer = glGetProcAddress( "glDrawBuffer" );
        if ( !glDrawBuffer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDrawElements = glGetProcAddress( "glDrawElements" );
        if ( !glDrawElements ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDrawPixels = glGetProcAddress( "glDrawPixels" );
        if ( !glDrawPixels ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEdgeFlag = glGetProcAddress( "glEdgeFlag" );
        if ( !glEdgeFlag ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEdgeFlagPointer = glGetProcAddress( "glEdgeFlagPointer" );
        if ( !glEdgeFlagPointer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEdgeFlagv = glGetProcAddress( "glEdgeFlagv" );
        if ( !glEdgeFlagv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEnable = glGetProcAddress( "glEnable" );
        if ( !glEdgeFlagv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEnableClientState = glGetProcAddress( "glEnableClientState" );
        if ( !glEnableClientState ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEnd = glGetProcAddress( "glEnd" );
        if ( !glEnd ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEndList = glGetProcAddress( "glEndList" );
        if ( !glEndList ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord1d = glGetProcAddress( "glEvalCoord1d" );
        if ( !glEvalCoord1d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord1dv = glGetProcAddress( "glEvalCoord1dv" );
        if ( !glEvalCoord1dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord1f = glGetProcAddress( "glEvalCoord1f" );
        if ( !glEvalCoord1f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord1fv = glGetProcAddress( "glEvalCoord1fv" );
        if ( !glEvalCoord1fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord2d = glGetProcAddress( "glEvalCoord2d" );
        if ( !glEvalCoord2d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord2dv = glGetProcAddress( "glEvalCoord2dv" );
        if ( !glEvalCoord2dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord2f = glGetProcAddress( "glEvalCoord2f" );
        if ( !glEvalCoord2f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalCoord2fv = glGetProcAddress( "glEvalCoord2fv" );
        if ( !glEvalCoord2fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalMesh1 = glGetProcAddress( "glEvalMesh1" );
        if ( !glEvalMesh1 ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalMesh2 = glGetProcAddress( "glEvalMesh2" );
        if ( !glEvalMesh2 ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalPoint1 = glGetProcAddress( "glEvalPoint1" );
        if ( !glEvalPoint1 ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEvalPoint2 = glGetProcAddress( "glEvalPoint2" );
        if ( !glEvalPoint2 ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFeedbackBuffer = glGetProcAddress( "glFeedbackBuffer" );
        if ( !glFeedbackBuffer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFinish = glGetProcAddress( "glFinish" );
        if ( !glFinish ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFlush = glGetProcAddress( "glFlush" );
        if ( !glFlush ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogf = glGetProcAddress( "glFogf" );
        if ( !glFogf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogfv = glGetProcAddress( "glFogfv" );
        if ( !glFogfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogi = glGetProcAddress( "glFogi" );
        if ( !glFogi ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogiv = glGetProcAddress( "glFogiv" );
        if ( !glFogiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFrontFace = glGetProcAddress( "glFrontFace" );
        if ( !glFrontFace ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFrustum = glGetProcAddress( "glFrustum" );
        if ( !glFrustum ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGenLists = glGetProcAddress( "glGenLists" );
        if ( !glGenLists ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGenTextures = glGetProcAddress( "glGenTextures" );
        if ( !glGenTextures ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetBooleanv = glGetProcAddress( "glGetBooleanv" );
        if ( !glGetBooleanv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetClipPlane = glGetProcAddress( "glGetClipPlane" );
        if ( !glGetClipPlane ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetDoublev = glGetProcAddress( "glGetDoublev" );
        if ( !glGetDoublev ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetError = glGetProcAddress( "glGetError" );
        if ( !glGetError ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetFloatv = glGetProcAddress( "glGetFloatv" );
        if ( !glGetFloatv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetIntegerv = glGetProcAddress( "glGetIntegerv" );
        if ( !glGetIntegerv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetLightfv = glGetProcAddress( "glGetLightfv" );
        if ( !glGetLightfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetLightiv = glGetProcAddress( "glGetLightiv" );
        if ( !glGetLightiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetMapdv = glGetProcAddress( "glGetMapdv" );
        if ( !glGetMapdv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetMapfv = glGetProcAddress( "glGetMapfv" );
        if ( !glGetMapfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetMapiv = glGetProcAddress( "glGetMapiv" );
        if ( !glGetMapiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetMaterialfv = glGetProcAddress( "glGetMaterialfv" );
        if ( !glGetMaterialfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetMaterialiv = glGetProcAddress( "glGetMaterialiv" );
        if ( !glGetMaterialiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetPixelMapfv = glGetProcAddress( "glGetPixelMapfv" );
        if ( !glGetPixelMapfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetPixelMapuiv = glGetProcAddress( "glGetPixelMapuiv" );
        if ( !glGetPixelMapuiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetPixelMapusv = glGetProcAddress( "glGetPixelMapusv" );
        if ( !glGetPixelMapusv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetPointerv = glGetProcAddress( "glGetPointerv" );
        if ( !glGetPointerv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetPolygonStipple = glGetProcAddress( "glGetPolygonStipple" );
        if ( !glGetPolygonStipple ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetString = glGetProcAddress( "glGetString" );
        if ( !glGetString ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexEnvfv = glGetProcAddress( "glGetTexEnvfv" );
        if ( !glGetTexEnvfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexEnviv = glGetProcAddress( "glGetTexEnviv" );
        if ( !glGetTexEnviv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexGendv = glGetProcAddress( "glGetTexGendv" );
        if ( !glGetTexGendv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexGenfv = glGetProcAddress( "glGetTexGenfv" );
        if ( !glGetTexGenfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexGeniv = glGetProcAddress( "glGetTexGeniv" );
        if ( !glGetTexGeniv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexImage = glGetProcAddress( "glGetTexImage" );
        if ( !glGetTexImage ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexLevelParameterfv = glGetProcAddress( "glGetTexLevelParameterfv" );
        if ( !glGetTexLevelParameterfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexLevelParameteriv = glGetProcAddress( "glGetTexLevelParameteriv" );
        if ( !glGetTexLevelParameteriv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexParameterfv = glGetProcAddress( "glGetTexParameterfv" );
        if ( !glGetTexParameterfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetTexParameteriv = glGetProcAddress( "glGetTexParameteriv" );
        if ( !glGetTexParameteriv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glHint = glGetProcAddress( "glHint" );
        if ( !glHint ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexMask = glGetProcAddress( "glIndexMask" );
        if ( !glIndexMask ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexPointer = glGetProcAddress( "glIndexPointer" );
        if ( !glIndexPointer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexd = glGetProcAddress( "glIndexd" );
        if ( !glIndexd ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexdv = glGetProcAddress( "glIndexdv" );
        if ( !glIndexdv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexf = glGetProcAddress( "glIndexf" );
        if ( !glIndexf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexfv = glGetProcAddress( "glIndexfv" );
        if ( !glIndexfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexi = glGetProcAddress( "glIndexi" );
        if ( !glIndexi ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexiv = glGetProcAddress( "glIndexiv" );
        if ( !glIndexiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexs = glGetProcAddress( "glIndexs" );
        if ( !glIndexs ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexsv = glGetProcAddress( "glIndexsv" );
        if ( !glIndexsv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexub = glGetProcAddress( "glIndexub" );
        if ( !glIndexub ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIndexubv = glGetProcAddress( "glIndexubv" );
        if ( !glIndexubv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glInitNames = glGetProcAddress( "glInitNames" );
        if ( !glInitNames ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glInterleavedArrays = glGetProcAddress( "glInterleavedArrays" );
        if ( !glInterleavedArrays ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIsEnabled = glGetProcAddress( "glIsEnabled" );
        if ( !glIsEnabled ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIsList = glGetProcAddress( "glIsList" );
        if ( !glIsList ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIsTexture = glGetProcAddress( "glIsTexture" );
        if ( !glIsTexture ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLightModelf = glGetProcAddress( "glLightModelf" );
        if ( !glLightModelf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLightModelfv = glGetProcAddress( "glLightModelfv" );
        if ( !glLightModelfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLightModeli = glGetProcAddress( "glLightModeli" );
        if ( !glLightModeli ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLightModeliv = glGetProcAddress( "glLightModeliv" );
        if ( !glLightModeliv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLightf = glGetProcAddress( "glLightf" );
        if ( !glLightf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLightfv = glGetProcAddress( "glLightfv" );
        if ( !glLightfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLighti = glGetProcAddress( "glLighti" );
        if ( !glIsList ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLightiv = glGetProcAddress( "glLightiv" );
        if ( !glLightiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLineStipple = glGetProcAddress( "glLineStipple" );
        if ( !glLineStipple ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLineWidth = glGetProcAddress( "glLineWidth" );
        if ( !glLineWidth ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glListBase = glGetProcAddress( "glListBase" );
        if ( !glListBase ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLoadIdentity = glGetProcAddress( "glLoadIdentity" );
        if ( !glLoadIdentity ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLoadMatrixd = glGetProcAddress( "glLoadMatrixd" );
        if ( !glLoadMatrixd ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLoadMatrixf = glGetProcAddress( "glLoadMatrixf" );
        if ( !glLoadMatrixf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLoadName = glGetProcAddress( "glLoadName" );
        if ( !glLoadName ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLogicOp = glGetProcAddress( "glLogicOp" );
        if ( !glLogicOp ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMap1d = glGetProcAddress( "glMap1d" );
        if ( !glMap1d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMap1f = glGetProcAddress( "glMap1f" );
        if ( !glMap1f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMap2d = glGetProcAddress( "glMap2d" );
        if ( !glMap2d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMap2f = glGetProcAddress( "glMap2f" );
        if ( !glMap2f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMapGrid1d = glGetProcAddress( "glMapGrid1d" );
        if ( !glMapGrid1d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMapGrid1f = glGetProcAddress( "glMapGrid1f" );
        if ( !glMapGrid1f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMapGrid2d = glGetProcAddress( "glMapGrid2d" );
        if ( !glMapGrid2d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMapGrid2f = glGetProcAddress( "glMapGrid2f" );
        if ( !glMapGrid2f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMaterialf = glGetProcAddress( "glMaterialf" );
        if ( !glMaterialf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMaterialfv = glGetProcAddress( "glMaterialfv" );
        if ( !glMaterialfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMateriali = glGetProcAddress( "glMateriali" );
        if ( !glMateriali ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMaterialiv = glGetProcAddress( "glMaterialiv" );
        if ( !glMaterialiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMatrixMode = glGetProcAddress( "glMatrixMode" );
        if ( !glMatrixMode ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultMatrixd = glGetProcAddress( "glMultMatrixd" );
        if ( !glMultMatrixd ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultMatrixf = glGetProcAddress( "glMultMatrixf" );
        if ( !glMultMatrixf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNewList = glGetProcAddress( "glNewList" );
        if ( !glNewList ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3b = glGetProcAddress( "glNormal3b" );
        if ( !glNormal3b ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3bv = glGetProcAddress( "glNormal3bv" );
        if ( !glNormal3bv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3d = glGetProcAddress( "glNormal3d" );
        if ( !glNormal3d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3dv = glGetProcAddress( "glNormal3dv" );
        if ( !glNormal3dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3f = glGetProcAddress( "glNormal3f" );
        if ( !glNormal3f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }                                                                
        glNormal3fv = glGetProcAddress( "glNormal3fv" );
        if ( !glNormal3fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3i = glGetProcAddress( "glNormal3i" );
        if ( !glNormal3i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3iv = glGetProcAddress( "glNormal3iv" );
        if ( !glNormal3iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3s = glGetProcAddress( "glNormal3s" );
        if ( !glNormal3s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormal3sv = glGetProcAddress( "glNormal3sv" );
        if ( !glNormal3sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glNormalPointer = glGetProcAddress( "glNormalPointer" );
        if ( !glNormalPointer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glOrtho = glGetProcAddress( "glOrtho" );
        if ( !glOrtho ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPassThrough = glGetProcAddress( "glPassThrough" );
        if ( !glPassThrough ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelMapfv = glGetProcAddress( "glPixelMapfv" );
        if ( !glPixelMapfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelMapuiv = glGetProcAddress( "glPixelMapuiv" );
        if ( !glPixelMapuiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelMapusv = glGetProcAddress( "glPixelMapusv" );
        if ( !glPixelMapusv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelStoref = glGetProcAddress( "glPixelStoref" );
        if ( !glPixelStoref ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelStorei = glGetProcAddress( "glPixelStorei" );
        if ( !glPixelStorei ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelTransferf = glGetProcAddress( "glPixelTransferf" );
        if ( !glPixelTransferf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelTransferi = glGetProcAddress( "glPixelTransferi" );
        if ( !glPixelTransferi ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPixelZoom = glGetProcAddress( "glPixelZoom" );
        if ( !glPixelZoom ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPointSize = glGetProcAddress( "glPointSize" );
        if ( !glPointSize ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPolygonMode = glGetProcAddress( "glPolygonMode" );
        if ( !glPolygonMode ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPolygonOffset = glGetProcAddress( "glPolygonOffset" );
        if ( !glPolygonOffset ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPolygonStipple = glGetProcAddress( "glPolygonStipple" );
        if ( !glPolygonStipple ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPopAttrib = glGetProcAddress( "glPopAttrib" );
        if ( !glPopAttrib ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPopClientAttrib = glGetProcAddress( "glPopClientAttrib" );
        if ( !glPopClientAttrib ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPopMatrix = glGetProcAddress( "glPopMatrix" );
        if ( !glPopMatrix ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPopName = glGetProcAddress( "glPopName" );
        if ( !glPopName ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }                                                                
        glPrioritizeTextures = glGetProcAddress( "glPrioritizeTextures" );
        if ( !glPrioritizeTextures ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPushAttrib = glGetProcAddress( "glPushAttrib" );
        if ( !glPushAttrib ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPushClientAttrib = glGetProcAddress( "glPushClientAttrib" );
        if ( !glPushClientAttrib ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPushMatrix = glGetProcAddress( "glPushMatrix" );
        if ( !glPushMatrix ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPushName = glGetProcAddress( "glPushName" );
        if ( !glPushName ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2d = glGetProcAddress( "glRasterPos2d" );
        if ( !glRasterPos2d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2dv = glGetProcAddress( "glRasterPos2dv" );
        if ( !glRasterPos2dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2f = glGetProcAddress( "glRasterPos2f" );
        if ( !glRasterPos2f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2fv = glGetProcAddress( "glRasterPos2fv" );
        if ( !glRasterPos2fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2i = glGetProcAddress( "glRasterPos2i" );
        if ( !glRasterPos2i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2iv = glGetProcAddress( "glRasterPos2iv" );
        if ( !glRasterPos2iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2s = glGetProcAddress( "glRasterPos2s" );
        if ( !glRasterPos2s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos2sv = glGetProcAddress( "glRasterPos2sv" );
        if ( !glRasterPos2sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3d = glGetProcAddress( "glRasterPos3d" );
        if ( !glRasterPos3d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3dv = glGetProcAddress( "glRasterPos3dv" );
        if ( !glRasterPos3dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3f = glGetProcAddress( "glRasterPos3f" );
        if ( !glRasterPos3f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3fv = glGetProcAddress( "glRasterPos3fv" );
        if ( !glRasterPos3fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3i = glGetProcAddress( "glRasterPos3i" );
        if ( !glRasterPos3i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3iv = glGetProcAddress( "glRasterPos3iv" );
        if ( !glRasterPos3iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3s = glGetProcAddress( "glRasterPos3s" );
        if ( !glRasterPos3s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos3sv = glGetProcAddress( "glRasterPos3sv" );
        if ( !glRasterPos3sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4d = glGetProcAddress( "glRasterPos4d" );
        if ( !glRasterPos4d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4dv = glGetProcAddress( "glRasterPos4dv" );
        if ( !glRasterPos4dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4f = glGetProcAddress( "glRasterPos4f" );
        if ( !glRasterPos4f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4fv = glGetProcAddress( "glRasterPos4fv" );
        if ( !glRasterPos4fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4i = glGetProcAddress( "glRasterPos4i" );
        if ( !glRasterPos4i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4iv = glGetProcAddress( "glRasterPos4iv" );
        if ( !glRasterPos4iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4s = glGetProcAddress( "glRasterPos4s" );
        if ( !glRasterPos4s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4sv = glGetProcAddress( "glRasterPos4sv" );
        if ( !glRasterPos4sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glReadBuffer = glGetProcAddress( "glReadBuffer" );
        if ( !glReadBuffer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glReadPixels = glGetProcAddress( "glReadPixels" );
        if ( !glReadPixels ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRectd = glGetProcAddress( "glRectd" );
        if ( !glRectd ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRectdv = glGetProcAddress( "glRectdv" );
        if ( !glRectdv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRectf = glGetProcAddress( "glRectf" );
        if ( !glRectf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRectfv = glGetProcAddress( "glRectfv" );
        if ( !glRectfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRecti = glGetProcAddress( "glRecti" );
        if ( !glRecti ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRectiv = glGetProcAddress( "glRectiv" );
        if ( !glRectiv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRects = glGetProcAddress( "glRects" );
        if ( !glRects ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRectsv = glGetProcAddress( "glRectsv" );
        if ( !glRectsv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4sv = glGetProcAddress( "glRasterPos4sv" );
        if ( !glRasterPos4sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4sv = glGetProcAddress( "glRasterPos4sv" );
        if ( !glRasterPos4sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRasterPos4sv = glGetProcAddress( "glRasterPos4sv" );
        if ( !glRasterPos4sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }                                
        glRenderMode = glGetProcAddress( "glRenderMode" );
        if ( !glRenderMode ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRotated = glGetProcAddress( "glRotated" );
        if ( !glRotated ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glRotatef = glGetProcAddress( "glRotatef" );
        if ( !glRotatef ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glScaled = glGetProcAddress( "glScaled" );
        if ( !glScaled ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glScalef = glGetProcAddress( "glScalef" );
        if ( !glScalef ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glScissor = glGetProcAddress( "glScissor" );
        if ( !glScissor ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSelectBuffer = glGetProcAddress( "glSelectBuffer" );
        if ( !glSelectBuffer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glShadeModel = glGetProcAddress( "glShadeModel" );
        if ( !glShadeModel ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glStencilFunc = glGetProcAddress( "glStencilFunc" );
        if ( !glStencilFunc ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glStencilMask = glGetProcAddress( "glStencilMask" );
        if ( !glStencilMask ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glStencilOp = glGetProcAddress( "glStencilOp" );
        if ( !glStencilOp ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1d = glGetProcAddress( "glTexCoord1d" );
        if ( !glTexCoord1d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1dv = glGetProcAddress( "glTexCoord1dv" );
        if ( !glTexCoord1dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1f = glGetProcAddress( "glTexCoord1f" );
        if ( !glTexCoord1f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1fv = glGetProcAddress( "glTexCoord1fv" );
        if ( !glTexCoord1fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1i = glGetProcAddress( "glTexCoord1i" );
        if ( !glTexCoord1i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1iv = glGetProcAddress( "glTexCoord1iv" );
        if ( !glTexCoord1iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1s = glGetProcAddress( "glTexCoord1s" );
        if ( !glTexCoord1s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord1sv = glGetProcAddress( "glTexCoord1sv" );
        if ( !glTexCoord1sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2d = glGetProcAddress( "glTexCoord2d" );
        if ( !glTexCoord2d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2dv = glGetProcAddress( "glTexCoord2dv" );
        if ( !glTexCoord2dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2f = glGetProcAddress( "glTexCoord2f" );
        if ( !glTexCoord2f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2fv = glGetProcAddress( "glTexCoord2fv" );
        if ( !glTexCoord2fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2i = glGetProcAddress( "glTexCoord2i" );
        if ( !glTexCoord2i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2iv = glGetProcAddress( "glTexCoord2iv" );
        if ( !glTexCoord2iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2s = glGetProcAddress( "glTexCoord2s" );
        if ( !glTexCoord2s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord2sv = glGetProcAddress( "glTexCoord2sv" );
        if ( !glTexCoord2sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3d = glGetProcAddress( "glTexCoord3d" );
        if ( !glTexCoord3d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3dv = glGetProcAddress( "glTexCoord3dv" );
        if ( !glTexCoord3dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3f = glGetProcAddress( "glTexCoord3f" );
        if ( !glTexCoord3f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3fv = glGetProcAddress( "glTexCoord3fv" );
        if ( !glTexCoord3fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3i = glGetProcAddress( "glTexCoord3i" );
        if ( !glTexCoord3i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3iv = glGetProcAddress( "glTexCoord3iv" );
        if ( !glTexCoord3iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3s = glGetProcAddress( "glTexCoord3s" );
        if ( !glTexCoord3s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord3sv = glGetProcAddress( "glTexCoord3sv" );
        if ( !glTexCoord3sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4d = glGetProcAddress( "glTexCoord4d" );
        if ( !glTexCoord4d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4dv = glGetProcAddress( "glTexCoord4dv" );
        if ( !glTexCoord4dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4f = glGetProcAddress( "glTexCoord4f" );
        if ( !glTexCoord4f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4fv = glGetProcAddress( "glTexCoord4fv" );
        if ( !glTexCoord4fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4i = glGetProcAddress( "glTexCoord4i" );
        if ( !glTexCoord4i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4iv = glGetProcAddress( "glTexCoord4iv" );
        if ( !glTexCoord4iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4s = glGetProcAddress( "glTexCoord4s" );
        if ( !glTexCoord4s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoord4sv = glGetProcAddress( "glTexCoord4sv" );
        if ( !glTexCoord4sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexCoordPointer = glGetProcAddress( "glTexCoordPointer" );
        if ( !glTexCoordPointer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexEnvf = glGetProcAddress( "glTexEnvf" );
        if ( !glTexEnvf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexEnvfv = glGetProcAddress( "glTexEnvfv" );
        if ( !glTexEnvfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexEnvi = glGetProcAddress( "glTexEnvi" );
        if ( !glTexEnvi ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexEnviv = glGetProcAddress( "glTexEnviv" );
        if ( !glTexEnviv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexGend = glGetProcAddress( "glTexGend" );
        if ( !glTexGend ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexGendv = glGetProcAddress( "glTexGendv" );
        if ( !glTexGendv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexGenf = glGetProcAddress( "glTexGenf" );
        if ( !glTexGenf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexGenfv = glGetProcAddress( "glTexGenfv" );
        if ( !glTexGenfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexGeni = glGetProcAddress( "glTexGeni" );
        if ( !glTexGeni ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexGeniv = glGetProcAddress( "glTexGeniv" );
        if ( !glTexGeniv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexImage1D = glGetProcAddress( "glTexImage1D" );
        if ( !glTexImage1D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexImage2D = glGetProcAddress( "glTexImage2D" );
        if ( !glTexImage2D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexParameterf = glGetProcAddress( "glTexParameterf" );
        if ( !glTexParameterf ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexParameterfv = glGetProcAddress( "glTexParameterfv" );
        if ( !glTexParameterfv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexParameteri = glGetProcAddress( "glTexParameteri" );
        if ( !glTexParameteri ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexParameteriv = glGetProcAddress( "glTexParameteriv" );
        if ( !glTexParameteriv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexSubImage1D = glGetProcAddress( "glTexSubImage1D" );
        if ( !glTexSubImage1D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexSubImage2D = glGetProcAddress( "glTexSubImage2D" );
        if ( !glTexSubImage2D ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTranslated = glGetProcAddress( "glTranslated" );
        if ( !glTranslated ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTranslatef = glGetProcAddress( "glTranslatef" );
        if ( !glTranslatef ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glVertex2d = glGetProcAddress( "glVertex2d" );
        if ( !glVertex2d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glVertex2dv = glGetProcAddress( "glVertex2dv" );
        if ( !glVertex2dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glVertex2f = glGetProcAddress( "glVertex2f" );
        if ( !glVertex2f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glVertex2fv = glGetProcAddress( "glVertex2fv" );
        if ( !glVertex2fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex2i = glGetProcAddress( "glVertex2i" );
        if ( !glVertex2i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex2iv = glGetProcAddress( "glVertex2iv" );
        if ( !glVertex2iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex2s = glGetProcAddress( "glVertex2s" );
        if ( !glVertex2s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex2sv = glGetProcAddress( "glVertex2sv" );
        if ( !glVertex2sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3d = glGetProcAddress( "glVertex3d" );
        if ( !glVertex3d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3dv = glGetProcAddress( "glVertex3dv" );
        if ( !glVertex3dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3f = glGetProcAddress( "glVertex3f" );
        if ( !glVertex3f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3fv = glGetProcAddress( "glVertex3fv" );
        if ( !glVertex3fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3i = glGetProcAddress( "glVertex3i" );
        if ( !glVertex3i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3iv = glGetProcAddress( "glVertex3iv" );
        if ( !glVertex3iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3s = glGetProcAddress( "glVertex3s" );
        if ( !glVertex3s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex3sv = glGetProcAddress( "glVertex3sv" );
        if ( !glVertex3sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4d = glGetProcAddress( "glVertex4d" );
        if ( !glVertex4d ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4dv = glGetProcAddress( "glVertex4dv" );
        if ( !glVertex4dv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4f = glGetProcAddress( "glVertex4f" );
        if ( !glVertex4f ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4fv = glGetProcAddress( "glVertex4fv" );
        if ( !glVertex4fv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4i = glGetProcAddress( "glVertex4i" );
        if ( !glVertex4i ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4iv = glGetProcAddress( "glVertex4iv" );
        if ( !glVertex4iv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4s = glGetProcAddress( "glVertex4s" );
        if ( !glVertex4s ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertex4sv = glGetProcAddress( "glVertex4sv" );
        if ( !glVertex4sv ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glVertexPointer = glGetProcAddress( "glVertexPointer" );
        if ( !glVertexPointer ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
       glViewport = glGetProcAddress( "glViewport" );
        if ( !glViewport ) { glUnloadDriver(); status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }

        return status;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

static int
glLoadOpenGL1_2Functions( void )
{
        glCopyTexSubImage3D = glGetProcAddress( "glCopyTexSubImage3D" );
        if ( !glCopyTexSubImage3D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDrawRangeElements = glGetProcAddress( "glDrawRangeElements" );
        if ( !glDrawRangeElements ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexImage3D = glGetProcAddress( "glTexImage3D" );
        if ( !glTexImage3D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glTexSubImage3D = glGetProcAddress( "glTexSubImage3D" );
        if ( !glTexSubImage3D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }

        return status;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

static int
glLoadOpenGL1_3Functions( void )
{
        glActiveTexture = glGetProcAddress( "glActiveTexture" );
        if ( !glActiveTexture ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glClientActiveTexture = glGetProcAddress( "glClientActiveTexture" );
        if ( !glClientActiveTexture ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCompressedTexImage1D = glGetProcAddress( "glCompressedTexImage1D" );
        if ( !glCompressedTexImage1D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCompressedTexImage2D = glGetProcAddress( "glCompressedTexImage2D" );
        if ( !glCompressedTexImage2D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCompressedTexImage3D = glGetProcAddress( "glCompressedTexImage3D" );
        if ( !glCompressedTexImage3D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCompressedTexSubImage1D = glGetProcAddress( "glCompressedTexSubImage1D" );
        if ( !glCompressedTexSubImage1D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCompressedTexSubImage2D = glGetProcAddress( "glCompressedTexSubImage2D" );
        if ( !glCompressedTexSubImage2D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glCompressedTexSubImage3D = glGetProcAddress( "glCompressedTexSubImage3D" );
        if ( !glCompressedTexSubImage3D ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetCompressedTexImage = glGetProcAddress( "glGetCompressedTexImage" );
        if ( !glGetCompressedTexImage ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLoadTransposeMatrixd = glGetProcAddress( "glLoadTransposeMatrixd" );
        if ( !glLoadTransposeMatrixd ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glLoadTransposeMatrixf = glGetProcAddress( "glLoadTransposeMatrixf" );
        if ( !glLoadTransposeMatrixf ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultTransposeMatrixd = glGetProcAddress( "glMultTransposeMatrixd" );
        if ( !glMultTransposeMatrixd ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultTransposeMatrixf = glGetProcAddress( "glMultTransposeMatrixf" );
        if ( !glMultTransposeMatrixf ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1d = glGetProcAddress( "glMultiTexCoord1d" );
        if ( !glMultiTexCoord1d ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1dv = glGetProcAddress( "glMultiTexCoord1dv" );
        if ( !glMultiTexCoord1dv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1f = glGetProcAddress( "glMultiTexCoord1f" );
        if ( !glMultiTexCoord1f ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1fv = glGetProcAddress( "glMultiTexCoord1fv" );
        if ( !glMultiTexCoord1fv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1i = glGetProcAddress( "glMultiTexCoord1i" );
        if ( !glMultiTexCoord1i ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1iv = glGetProcAddress( "glMultiTexCoord1iv" );
        if ( !glMultiTexCoord1iv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1s = glGetProcAddress( "glMultiTexCoord1s" );
        if ( !glMultiTexCoord1s ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord1sv = glGetProcAddress( "glMultiTexCoord1sv" );
        if ( !glMultiTexCoord1sv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord2d = glGetProcAddress( "glMultiTexCoord2d" );
        if ( !glMultiTexCoord2d ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord2dv = glGetProcAddress( "glMultiTexCoord2dv" );
        if ( !glMultiTexCoord2dv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord2f = glGetProcAddress( "glMultiTexCoord2f" );
        if ( !glMultiTexCoord2f ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord2fv = glGetProcAddress( "glMultiTexCoord2fv" );
        if ( !glMultiTexCoord2fv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord2i = glGetProcAddress( "glMultiTexCoord2i" );
        if ( !glMultiTexCoord2i ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; } //////////
        glMultiTexCoord2iv = glGetProcAddress( "glMultiTexCoord2iv" );
        if ( !glMultiTexCoord2iv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord2s = glGetProcAddress( "glMultiTexCoord2s" );
        if ( !glMultiTexCoord2s ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord2sv = glGetProcAddress( "glMultiTexCoord2sv" );
        if ( !glMultiTexCoord2sv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3d = glGetProcAddress( "glMultiTexCoord3d" );
        if ( !glMultiTexCoord3d ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3dv = glGetProcAddress( "glMultiTexCoord3dv" );
        if ( !glMultiTexCoord3dv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3f = glGetProcAddress( "glMultiTexCoord3f" );
        if ( !glMultiTexCoord3f ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3fv = glGetProcAddress( "glMultiTexCoord3fv" );
        if ( !glMultiTexCoord3fv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3i = glGetProcAddress( "glMultiTexCoord3i" );
        if ( !glMultiTexCoord3i ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3iv = glGetProcAddress( "glMultiTexCoord3iv" );
        if ( !glMultiTexCoord3iv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3s = glGetProcAddress( "glMultiTexCoord3s" );
        if ( !glMultiTexCoord3s ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord3sv = glGetProcAddress( "glMultiTexCoord3sv" );
        if ( !glMultiTexCoord3sv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4d = glGetProcAddress( "glMultiTexCoord4d" );
        if ( !glMultiTexCoord4d ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4dv = glGetProcAddress( "glMultiTexCoord4dv" );
        if ( !glMultiTexCoord4dv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4f = glGetProcAddress( "glMultiTexCoord4f" );
        if ( !glMultiTexCoord4f ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4fv = glGetProcAddress( "glMultiTexCoord4fv" );
        if ( !glMultiTexCoord4fv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4i = glGetProcAddress( "glMultiTexCoord4i" );
        if ( !glMultiTexCoord4i ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4iv = glGetProcAddress( "glMultiTexCoord4iv" );
        if ( !glMultiTexCoord4iv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4s = glGetProcAddress( "glMultiTexCoord4s" );
        if ( !glMultiTexCoord4s ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiTexCoord4sv = glGetProcAddress( "glMultiTexCoord4sv" );
        if ( !glMultiTexCoord4sv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSampleCoverage = glGetProcAddress( "glSampleCoverage" );
        if ( !glSampleCoverage ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }

        return status;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

static int
glLoadOpenGL1_4Functions( void )
{
        glBlendColor = glGetProcAddress( "glBlendColor" );
        if ( !glBlendColor ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBlendEquation = glGetProcAddress( "glBlendEquation" );
        if ( !glBlendEquation ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBlendFuncSeparate = glGetProcAddress( "glBlendFuncSeparate" );
        if ( !glBlendFuncSeparate ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogCoordPointer = glGetProcAddress( "glFogCoordPointer" );
        if ( !glFogCoordPointer ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogCoordd = glGetProcAddress( "glFogCoordd" );
        if ( !glFogCoordd ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogCoorddv = glGetProcAddress( "glFogCoorddv" );
        if ( !glFogCoorddv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogCoordf = glGetProcAddress( "glFogCoordf" );
        if ( !glFogCoordf ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glFogCoordfv = glGetProcAddress( "glFogCoordfv" );
        if ( !glFogCoordfv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiDrawArrays = glGetProcAddress( "glMultiDrawArrays" );
        if ( !glMultiDrawArrays ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMultiDrawElements = glGetProcAddress( "glMultiDrawElements" );
        if ( !glMultiDrawElements ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPointParameterf = glGetProcAddress( "glPointParameterf" );
        if ( !glPointParameterf ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glPointParameterfv = glGetProcAddress( "glPointParameterfv" );
        if ( !glPointParameterfv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3b = glGetProcAddress( "glSecondaryColor3b" );
        if ( !glSecondaryColor3b ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3bv = glGetProcAddress( "glSecondaryColor3bv" );
        if ( !glSecondaryColor3bv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3d = glGetProcAddress( "glSecondaryColor3d" );
        if ( !glSecondaryColor3d ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3dv = glGetProcAddress( "glSecondaryColor3dv" );
        if ( !glSecondaryColor3dv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3f = glGetProcAddress( "glSecondaryColor3f" );
        if ( !glSecondaryColor3f ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3fv = glGetProcAddress( "glSecondaryColor3fv" );
        if ( !glSecondaryColor3fv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3i = glGetProcAddress( "glSecondaryColor3i" );
        if ( !glSecondaryColor3i ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3iv = glGetProcAddress( "glSecondaryColor3iv" );
        if ( !glSecondaryColor3iv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3s = glGetProcAddress( "glSecondaryColor3s" );
        if ( !glSecondaryColor3s ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3sv = glGetProcAddress( "glSecondaryColor3sv" );
        if ( !glSecondaryColor3sv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3ub = glGetProcAddress( "glSecondaryColor3ub" );
        if ( !glSecondaryColor3ub ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3ubv = glGetProcAddress( "glSecondaryColor3ubv" );
        if ( !glSecondaryColor3ubv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3ui = glGetProcAddress( "glSecondaryColor3ui" );
        if ( !glSecondaryColor3ui ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3uiv = glGetProcAddress( "glSecondaryColor3uiv" );
        if ( !glSecondaryColor3uiv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3us = glGetProcAddress( "glSecondaryColor3us" );
        if ( !glSecondaryColor3us ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColor3usv = glGetProcAddress( "glSecondaryColor3usv" );
        if ( !glSecondaryColor3usv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glSecondaryColorPointer = glGetProcAddress( "glSecondaryColorPointer" );
        if ( !glSecondaryColorPointer ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2d = glGetProcAddress( "glWindowPos2d" );
        if ( !glWindowPos2d ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2dv = glGetProcAddress( "glWindowPos2dv" );
        if ( !glWindowPos2dv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2f = glGetProcAddress( "glWindowPos2f" );
        if ( !glWindowPos2f ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2fv = glGetProcAddress( "glWindowPos2fv" );
        if ( !glWindowPos2fv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2i = glGetProcAddress( "glWindowPos2i" );
        if ( !glWindowPos2i ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2iv = glGetProcAddress( "glWindowPos2iv" );
        if ( !glWindowPos2iv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2s = glGetProcAddress( "glWindowPos2s" );
        if ( !glWindowPos2s ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos2sv = glGetProcAddress( "glWindowPos2sv" );
        if ( !glWindowPos2sv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3d = glGetProcAddress( "glWindowPos3d" );
        if ( !glWindowPos3d ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3dv = glGetProcAddress( "glWindowPos3dv" );
        if ( !glWindowPos3dv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3f = glGetProcAddress( "glWindowPos3f" );
        if ( !glWindowPos3f ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3fv = glGetProcAddress( "glWindowPos3fv" );
        if ( !glWindowPos3fv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3i = glGetProcAddress( "glWindowPos3i" );
        if ( !glWindowPos3i ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3iv = glGetProcAddress( "glWindowPos3iv" );
        if ( !glWindowPos3iv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3s = glGetProcAddress( "glWindowPos3s" );
        if ( !glWindowPos3s ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glWindowPos3sv = glGetProcAddress( "glWindowPos3sv" );
        if ( !glWindowPos3sv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }

        return status;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

static int
glLoadOpenGL1_5Functions( void )
{
        glBeginQuery = glGetProcAddress( "glBeginQuery" );
        if ( !glBeginQuery ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBindBuffer = glGetProcAddress( "glBindBuffer" );
        if ( !glBindBuffer ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBufferData = glGetProcAddress( "glBufferData" );
        if ( !glBufferData ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glBufferSubData = glGetProcAddress( "glBufferSubData" );
        if ( !glBufferSubData ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDeleteBuffers = glGetProcAddress( "glDeleteBuffers" );
        if ( !glDeleteBuffers ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glDeleteQueries = glGetProcAddress( "glDeleteQueries" );
        if ( !glDeleteQueries ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glEndQuery = glGetProcAddress( "glEndQuery" );
        if ( !glEndQuery ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGenBuffers = glGetProcAddress( "glGenBuffers" );
        if ( !glGenBuffers ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGenQueries = glGetProcAddress( "glGenQueries" );
        if ( !glGenQueries ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetBufferParameteriv = glGetProcAddress( "glGetBufferParameteriv" );
        if ( !glGetBufferParameteriv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetBufferPointerv = glGetProcAddress( "glGetBufferPointerv" );
        if ( !glGetBufferPointerv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetBufferSubData = glGetProcAddress( "glGetBufferSubData" );
        if ( !glGetBufferSubData ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetQueryObjectiv = glGetProcAddress( "glGetQueryObjectiv" );
        if ( !glGetQueryObjectiv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetQueryObjectuiv = glGetProcAddress( "glGetQueryObjectuiv" );
        if ( !glGetQueryObjectuiv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glGetQueryiv = glGetProcAddress( "glGetQueryiv" );
        if ( !glGetQueryiv ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIsBuffer = glGetProcAddress( "glIsBuffer" );
        if ( !glIsBuffer ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glIsQuery = glGetProcAddress( "glIsQuery" );
        if ( !glIsQuery ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glMapBuffer = glGetProcAddress( "glMapBuffer" );
        if ( !glMapBuffer ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }
        glUnmapBuffer = glGetProcAddress( "glUnmapBuffer" );
        if ( !glUnmapBuffer ) { status = GLDRIVER_UNABLE_TO_LINK_FUNCTION; return status; }

        return status;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

/**
 *      Unloads any GL library that may have been loaded with a successfull
 *      call to glLoadDriver(). This will invalidate all the gl* function
 *      pointers !!!.
 */
void
glUnloadDriver( void )
{
        status = GLDRIVER_FALSE;
        free( drvlib );
        drvlib = NULL;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

/**
 *      Function that attempts to link the given library and set up all the
 *      _gl* function pointers. If successfull GLDRIVER_TRUE is returned,
 *      otherwise an error code is returned. Any driver that is already loaded
 *      will be unloaded.
 */
Int32
glLoadDriver( const char *library )
{
        /*
         *      If a driver is already loaded we will unload it first.
         */
        glUnloadDriver();

        /*
         *      Attempt to load the library with the given name.
         */
        status = GLDRIVER_FALSE;
        if ( !library ) { status = GLDRIVER_UNABLE_TO_LOAD_DRIVER; return status; }
        if ( SDL_GL_LoadLibrary( library ) < 0 ) { status = GLDRIVER_UNABLE_TO_LOAD_DRIVER; return status; }

        /*
         *      We will now try to load the OpenGL 1.1 functions.
         *      Version 1.1 is the oldest supported version.
         *      Failure to load any one of the functions is considdered a fatal
         *      error.
         */
        glLoadOpenGL1_1Functions();
        if ( status != GLDRIVER_FALSE ) return status;

        /*
         *      Now we load the rest.
         */
        glversion = 1.1;
        glLoadOpenGL1_2Functions();
        if ( status == GLDRIVER_FALSE )
        {
                glversion = 1.2;
                glLoadOpenGL1_3Functions();
                if ( status == GLDRIVER_FALSE )
                {
                        glversion = 1.3;
                        glLoadOpenGL1_4Functions();
                        if ( status == GLDRIVER_FALSE )
                        {
                                glversion = 1.4;
                                glLoadOpenGL1_5Functions();
                                if ( status == GLDRIVER_FALSE )
                                {
                                        glversion = 1.5;
                                }
                        }
                }
        }

        #ifdef GLDRIVER_DEBUG_MODE
        glErrorHandler( glNoOpErrorHandler );
        #endif /* GLDRIVER_DEBUG_MODE ? */

        /*
         *      Allocate storage and copy the new driver library string.
         */
        free( drvlib );
        drvlib = malloc( strlen( library )+1 );
        strcpy( drvlib, library );

        /*
         *      Return success.
         */
        status = GLDRIVER_TRUE;
        return status;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

#ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING

/**
 *      Returns wheter or not a GL driver has been loaded. If not then the
 *      _gl* function pointers should not be used. GLDRIVER_TRUE indicates true
 *      any other value indicates false and possibly an error that occured
 *      during the last glLoadDriver() attempt.
 */
Int32
glIsDriverLoaded( void )
{
    return status;
}

#endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */

/*-------------------------------------------------------------------------*/

/**
 *      Returns a const pointer to a null-terminated string holding the current
 *      driver library string. This function can return NULL if no driver library
 *      is loaded.
 */
const char*
glDriverLibrary( void )
{
    #ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING
    return drvlib;
    #else
    return "N/A: staticly linked to OpenGL";
    #endif /* GLDRIVER_DYNAMIC_OPENGL_LINKING ? */
}

/*-------------------------------------------------------------------------*/

/**
 *      Returns the version of the OpenGL driver that was loaded.
 *      OpenGL functions will be loaded incrementally from version 1.1 up.
 *      If just one of the functions in a set can't be loaded the library will
 *      be considdered to be of the version belonging to the last successfull
 *      function set load.
 */
GLfloat
glVersion( void )
{
    #ifdef GLDRIVER_DYNAMIC_OPENGL_LINKING
    
    return glversion;
    
    #else

    #ifdef GL_VERSION_1_5
    glversion = 1.5f;
    #else
      #ifdef GL_VERSION_1_4
      glversion = 1.4f;
      #else
        #ifdef GL_VERSION_1_3
        glversion = 1.3f;
        #else
          #ifdef GL_VERSION_1_2
          glversion = 1.2f;
          #else
            #ifdef GL_VERSION_1_1
            glversion = 1.1f;
            #else
              #error "OpenGL versions older then 1.1 are not supported"
            #endif
          #endif
        #endif
      #endif         
    #endif
    
    return glversion;

    #endif
}

/*-------------------------------------------------------------------------*/

#undef GLDRIVER_CALLSPEC_PREFIX
#undef GLDRIVER_CALLSPEC_SUFFIX




