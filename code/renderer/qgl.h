/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
/*
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__

#ifndef XENON
	#ifdef USE_LOCAL_HEADERS
	#	include "SDL_opengl.h"
	#else
	#	include <SDL_opengl.h>
	#endif
#else
	#	include <GL/gl.h>
	#define APIENTRY
	#define APIENTRYP APIENTRY *
#endif

extern void (APIENTRYP qglActiveTextureARB) (GLenum texture);
extern void (APIENTRYP qglClientActiveTextureARB) (GLenum texture);
extern void (APIENTRYP qglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);

extern void (APIENTRYP qglLockArraysEXT) (GLint first, GLsizei count);
extern void (APIENTRYP qglUnlockArraysEXT) (void);


//===========================================================================

#define qglAccum glAccum
#define qglAlphaFunc glAlphaFunc
#define qglAreTexturesResident glAreTexturesResident
#define qglArrayElement glArrayElement
#define qglBegin glBegin
#define qglBindTexture glBindTexture
#define qglBitmap glBitmap
#define qglBlendFunc glBlendFunc
#define qglCallList glCallList
#define qglCallLists glCallLists
#define qglClear glClear
#define qglClearAccum glClearAccum
#define qglClearColor glClearColor
#define qglClearDepth glClearDepth
#define qglClearIndex glClearIndex
#define qglClearStencil glClearStencil
#define qglClipPlane glClipPlane
#define qglColor3b glColor3b
#define qglColor3bv glColor3bv
#define qglColor3d glColor3d
#define qglColor3dv glColor3dv
#define qglColor3f glColor3f
#define qglColor3fv glColor3fv
#define qglColor3i glColor3i
#define qglColor3iv glColor3iv
#define qglColor3s glColor3s
#define qglColor3sv glColor3sv
#define qglColor3ub glColor3ub
#define qglColor3ubv glColor3ubv
#define qglColor3ui glColor3ui
#define qglColor3uiv glColor3uiv
#define qglColor3us glColor3us
#define qglColor3usv glColor3usv
#define qglColor4b glColor4b
#define qglColor4bv glColor4bv
#define qglColor4d glColor4d
#define qglColor4dv glColor4dv
#define qglColor4f glColor4f
#define qglColor4fv glColor4fv
#define qglColor4i glColor4i
#define qglColor4iv glColor4iv
#define qglColor4s glColor4s
#define qglColor4sv glColor4sv
#define qglColor4ub glColor4ub
#define qglColor4ubv glColor4ubv
#define qglColor4ui glColor4ui
#define qglColor4uiv glColor4uiv
#define qglColor4us glColor4us
#define qglColor4usv glColor4usv
#define qglColorMask glColorMask
#define qglColorMaterial glColorMaterial
#define qglColorPointer glColorPointer
#define qglCopyPixels glCopyPixels
#define qglCopyTexImage1D glCopyTexImage1D
#define qglCopyTexImage2D glCopyTexImage2D
#define qglCopyTexSubImage1D glCopyTexSubImage1D
#define qglCopyTexSubImage2D glCopyTexSubImage2D
#define qglCullFace glCullFace
#define qglDeleteLists glDeleteLists
#define qglDeleteTextures glDeleteTextures
#define qglDepthFunc glDepthFunc
#define qglDepthMask glDepthMask
#define qglDepthRange glDepthRange
#define qglDisable glDisable
#define qglDisableClientState glDisableClientState
#define qglDrawArrays glDrawArrays
#define qglDrawBuffer glDrawBuffer
#define qglDrawElements glDrawElements
#define qglDrawPixels glDrawPixels
#define qglEdgeFlag glEdgeFlag
#define qglEdgeFlagPointer glEdgeFlagPointer
#define qglEdgeFlagv glEdgeFlagv
#define qglEnable glEnable
#define qglEnableClientState glEnableClientState
#define qglEnd glEnd
#define qglEndList glEndList
#define qglEvalCoord1d glEvalCoord1d
#define qglEvalCoord1dv glEvalCoord1dv
#define qglEvalCoord1f glEvalCoord1f
#define qglEvalCoord1fv glEvalCoord1fv
#define qglEvalCoord2d glEvalCoord2d
#define qglEvalCoord2dv glEvalCoord2dv
#define qglEvalCoord2f glEvalCoord2f
#define qglEvalCoord2fv glEvalCoord2fv
#define qglEvalMesh1 glEvalMesh1
#define qglEvalMesh2 glEvalMesh2
#define qglEvalPoint1 glEvalPoint1
#define qglEvalPoint2 glEvalPoint2
#define qglFeedbackBuffer glFeedbackBuffer
#define qglFinish glFinish
#define qglFlush glFlush
#define qglFogf glFogf
#define qglFogfv glFogfv
#define qglFogi glFogi
#define qglFogiv glFogiv
#define qglFrontFace glFrontFace
#define qglFrustum glFrustum
#define qglGenLists glGenLists
#define qglGenTextures glGenTextures
#define qglGetBooleanv glGetBooleanv
#define qglGetClipPlane glGetClipPlane
#define qglGetDoublev glGetDoublev
#define qglGetError glGetError
#define qglGetFloatv glGetFloatv
#define qglGetIntegerv glGetIntegerv
#define qglGetLightfv glGetLightfv
#define qglGetLightiv glGetLightiv
#define qglGetMapdv glGetMapdv
#define qglGetMapfv glGetMapfv
#define qglGetMapiv glGetMapiv
#define qglGetMaterialfv glGetMaterialfv
#define qglGetMaterialiv glGetMaterialiv
#define qglGetPixelMapfv glGetPixelMapfv
#define qglGetPixelMapuiv glGetPixelMapuiv
#define qglGetPixelMapusv glGetPixelMapusv
#define qglGetPointerv glGetPointerv
#define qglGetPolygonStipple glGetPolygonStipple
#define qglGetString glGetString
#define qglGetTexGendv glGetTexGendv
#define qglGetTexGenfv glGetTexGenfv
#define qglGetTexGeniv glGetTexGeniv
#define qglGetTexImage glGetTexImage
#define qglGetTexLevelParameterfv glGetTexLevelParameterfv
#define qglGetTexLevelParameteriv glGetTexLevelParameteriv
#define qglGetTexParameterfv glGetTexParameterfv
#define qglGetTexParameteriv glGetTexParameteriv
#define qglHint glHint
#define qglIndexMask glIndexMask
#define qglIndexPointer glIndexPointer
#define qglIndexd glIndexd
#define qglIndexdv glIndexdv
#define qglIndexf glIndexf
#define qglIndexfv glIndexfv
#define qglIndexi glIndexi
#define qglIndexiv glIndexiv
#define qglIndexs glIndexs
#define qglIndexsv glIndexsv
#define qglIndexub glIndexub
#define qglIndexubv glIndexubv
#define qglInitNames glInitNames
#define qglInterleavedArrays glInterleavedArrays
#define qglIsEnabled glIsEnabled
#define qglIsList glIsList
#define qglIsTexture glIsTexture
#define qglLightModelf glLightModelf
#define qglLightModelfv glLightModelfv
#define qglLightModeli glLightModeli
#define qglLightModeliv glLightModeliv
#define qglLightf glLightf
#define qglLightfv glLightfv
#define qglLighti glLighti
#define qglLightiv glLightiv
#define qglLineStipple glLineStipple
#define qglLineWidth glLineWidth
#define qglListBase glListBase
#define qglLoadIdentity glLoadIdentity
#define qglLoadMatrixd glLoadMatrixd
#define qglLoadMatrixf glLoadMatrixf
#define qglLoadName glLoadName
#define qglLogicOp glLogicOp
#define qglMap1d glMap1d
#define qglMap1f glMap1f
#define qglMap2d glMap2d
#define qglMap2f glMap2f
#define qglMapGrid1d glMapGrid1d
#define qglMapGrid1f glMapGrid1f
#define qglMapGrid2d glMapGrid2d
#define qglMapGrid2f glMapGrid2f
#define qglMaterialf glMaterialf
#define qglMaterialfv glMaterialfv
#define qglMateriali glMateriali
#define qglMaterialiv glMaterialiv
#define qglMatrixMode glMatrixMode
#define qglMultMatrixd glMultMatrixd
#define qglMultMatrixf glMultMatrixf
#define qglNewList glNewList
#define qglNormal3b glNormal3b
#define qglNormal3bv glNormal3bv
#define qglNormal3d glNormal3d
#define qglNormal3dv glNormal3dv
#define qglNormal3f glNormal3f
#define qglNormal3fv glNormal3fv
#define qglNormal3i glNormal3i
#define qglNormal3iv glNormal3iv
#define qglNormal3s glNormal3s
#define qglNormal3sv glNormal3sv
#define qglNormalPointer glNormalPointer
#define qglOrtho glOrtho
#define qglPassThrough glPassThrough
#define qglPixelMapfv glPixelMapfv
#define qglPixelMapuiv glPixelMapuiv
#define qglPixelMapusv glPixelMapusv
#define qglPixelStoref glPixelStoref
#define qglPixelStorei glPixelStorei
#define qglPixelTransferf glPixelTransferf
#define qglPixelTransferi glPixelTransferi
#define qglPixelZoom glPixelZoom
#define qglPointSize glPointSize
#define qglPolygonMode glPolygonMode
#define qglPolygonOffset glPolygonOffset
#define qglPolygonStipple glPolygonStipple
#define qglPopAttrib glPopAttrib
#define qglPopClientAttrib glPopClientAttrib
#define qglPopMatrix glPopMatrix
#define qglPopName glPopName
#define qglPrioritizeTextures glPrioritizeTextures
#define qglPushAttrib glPushAttrib
#define qglPushClientAttrib glPushClientAttrib
#define qglPushMatrix glPushMatrix
#define qglPushName glPushName
#define qglRasterPos2d glRasterPos2d
#define qglRasterPos2dv glRasterPos2dv
#define qglRasterPos2f glRasterPos2f
#define qglRasterPos2fv glRasterPos2fv
#define qglRasterPos2i glRasterPos2i
#define qglRasterPos2iv glRasterPos2iv
#define qglRasterPos2s glRasterPos2s
#define qglRasterPos2sv glRasterPos2sv
#define qglRasterPos3d glRasterPos3d
#define qglRasterPos3dv glRasterPos3dv
#define qglRasterPos3f glRasterPos3f
#define qglRasterPos3fv glRasterPos3fv
#define qglRasterPos3i glRasterPos3i
#define qglRasterPos3iv glRasterPos3iv
#define qglRasterPos3s glRasterPos3s
#define qglRasterPos3sv glRasterPos3sv
#define qglRasterPos4d glRasterPos4d
#define qglRasterPos4dv glRasterPos4dv
#define qglRasterPos4f glRasterPos4f
#define qglRasterPos4fv glRasterPos4fv
#define qglRasterPos4i glRasterPos4i
#define qglRasterPos4iv glRasterPos4iv
#define qglRasterPos4s glRasterPos4s
#define qglRasterPos4sv glRasterPos4sv
#define qglReadBuffer glReadBuffer
#define qglReadPixels glReadPixels
#define qglRectd glRectd
#define qglRectdv glRectdv
#define qglRectf glRectf
#define qglRectfv glRectfv
#define qglRecti glRecti
#define qglRectiv glRectiv
#define qglRects glRects
#define qglRectsv glRectsv
#define qglRenderMode glRenderMode
#define qglRotated glRotated
#define qglRotatef glRotatef
#define qglScaled glScaled
#define qglScalef glScalef
#define qglScissor glScissor
#define qglSelectBuffer glSelectBuffer
#define qglShadeModel glShadeModel
#define qglStencilFunc glStencilFunc
#define qglStencilMask glStencilMask
#define qglStencilOp glStencilOp
#define qglTexCoord1d glTexCoord1d
#define qglTexCoord1dv glTexCoord1dv
#define qglTexCoord1f glTexCoord1f
#define qglTexCoord1fv glTexCoord1fv
#define qglTexCoord1i glTexCoord1i
#define qglTexCoord1iv glTexCoord1iv
#define qglTexCoord1s glTexCoord1s
#define qglTexCoord1sv glTexCoord1sv
#define qglTexCoord2d glTexCoord2d
#define qglTexCoord2dv glTexCoord2dv
#define qglTexCoord2f glTexCoord2f
#define qglTexCoord2fv glTexCoord2fv
#define qglTexCoord2i glTexCoord2i
#define qglTexCoord2iv glTexCoord2iv
#define qglTexCoord2s glTexCoord2s
#define qglTexCoord2sv glTexCoord2sv
#define qglTexCoord3d glTexCoord3d
#define qglTexCoord3dv glTexCoord3dv
#define qglTexCoord3f glTexCoord3f
#define qglTexCoord3fv glTexCoord3fv
#define qglTexCoord3i glTexCoord3i
#define qglTexCoord3iv glTexCoord3iv
#define qglTexCoord3s glTexCoord3s
#define qglTexCoord3sv glTexCoord3sv
#define qglTexCoord4d glTexCoord4d
#define qglTexCoord4dv glTexCoord4dv
#define qglTexCoord4f glTexCoord4f
#define qglTexCoord4fv glTexCoord4fv
#define qglTexCoord4i glTexCoord4i
#define qglTexCoord4iv glTexCoord4iv
#define qglTexCoord4s glTexCoord4s
#define qglTexCoord4sv glTexCoord4sv
#define qglTexCoordPointer glTexCoordPointer
#define qglTexEnvf glTexEnvf
#define qglTexEnvfv glTexEnvfv
#define qglTexEnvi glTexEnvi
#define qglTexEnviv glTexEnviv
#define qglTexGend glTexGend
#define qglTexGendv glTexGendv
#define qglTexGenf glTexGenf
#define qglTexGenfv glTexGenfv
#define qglTexGeni glTexGeni
#define qglTexGeniv glTexGeniv
#define qglTexImage1D glTexImage1D
#define qglTexImage2D glTexImage2D
#define qglTexParameterf glTexParameterf
#define qglTexParameterfv glTexParameterfv
#define qglTexParameteri glTexParameteri
#define qglTexParameteriv glTexParameteriv
#define qglTexSubImage1D glTexSubImage1D
#define qglTexSubImage2D glTexSubImage2D
#define qglTranslated glTranslated
#define qglTranslatef glTranslatef
#define qglVertex2d glVertex2d
#define qglVertex2dv glVertex2dv
#define qglVertex2f glVertex2f
#define qglVertex2fv glVertex2fv
#define qglVertex2i glVertex2i
#define qglVertex2iv glVertex2iv
#define qglVertex2s glVertex2s
#define qglVertex2sv glVertex2sv
#define qglVertex3d glVertex3d
#define qglVertex3dv glVertex3dv
#define qglVertex3f glVertex3f
#define qglVertex3fv glVertex3fv
#define qglVertex3i glVertex3i
#define qglVertex3iv glVertex3iv
#define qglVertex3s glVertex3s
#define qglVertex3sv glVertex3sv
#define qglVertex4d glVertex4d
#define qglVertex4dv glVertex4dv
#define qglVertex4f glVertex4f
#define qglVertex4fv glVertex4fv
#define qglVertex4i glVertex4i
#define qglVertex4iv glVertex4iv
#define qglVertex4s glVertex4s
#define qglVertex4sv glVertex4sv
#define qglVertexPointer glVertexPointer
#define qglViewport glViewport

#endif
