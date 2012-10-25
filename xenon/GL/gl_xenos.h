#pragma once

#include "gl.h"
#include <xenos/xe.h>
#include "Vec.h"
/** based on 
 * http://quakeone.com/mh/gl_fakegl.c
 * http://fossies.org/dox/wine-1.4.1/d3dx9__36_2math_8c_source.html
 **/ 

/***********************************************************************
 * Vertices
 ***********************************************************************/
 
#define XE_MAX_VERTICES	8*1024*1024
 
typedef struct {	
    float x, y, z, w;
    // texture 0
    float u0, v0;
    // texture 1    
    float u1, v1;
    // color
    unsigned int color;
} glVerticesFormat_t;

typedef struct {
	float u, v;
} glTextCoord_t; 

typedef struct {
	union {
		unsigned char a, r, g, b;
	} u8;
	unsigned int u32;
} glColor_t; 

//glVerticesFormat_t * xe_Vertices;
float * xe_Vertices;
int16_t * xe_indices;
int xe_NumVerts;
int xe_PrevNumVerts;
int xe_NumIndices;
int xe_PrevNumIndices;
glColor_t xe_CurrentColor;
glTextCoord_t xe_TextCoord[2];

GLenum xe_PrimitiveMode;

unsigned int Gl_Color_2_Xe (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

/***********************************************************************
 * Textures
 ***********************************************************************/
 #define XE_MAX_TMUS 2
typedef struct glXeSurface_s{
	// OpenGL texture id
	int glnum;
	// Used by glTexSubImage2D
	GLenum internalformat;
	
	struct XenosSurface * teximg;
	
	// refresh texture cache only when need
	int dirty;
} glXeSurface_t;

glXeSurface_t * glXeSurfaces;

typedef struct glXeTmu_s{
	glXeSurface_t * boundtexture;
	int enabled;
	
	int texture_env_mode;

	int texenvdirty;
	int texparamdirty;
} glXeTmu_t;

glXeTmu_t xeTmus[XE_MAX_TMUS];

int xeCurrentTMU;

/***********************************************************************
 * Matrices
 ***********************************************************************/
 #define MAX_MATRIX_STACK 32
 
typedef enum{
	MATMODELVIEW	= 0, // register see opengl.hlsl
	MATPROJECTION	= 8,
} TRANSFORM_TYPE;

typedef struct
{
	int dirty;
	int stackdepth;
	TRANSFORM_TYPE usage;
	matrix4x4 stack[MAX_MATRIX_STACK];
} xe_matrix_t;

xe_matrix_t projection_matrix;
xe_matrix_t modelview_matrix;

void XeGlInitializeMatrix(xe_matrix_t *m);
void XeGlCheckDirtyMatrix(xe_matrix_t *m);

/***********************************************************************
 * States
 ***********************************************************************/
typedef struct {
	// blending
	int blend_enable;
	int blend_op;
	int blend_src;
	int blend_dst;
	
	// Alpha test
	int alpha_test_enabled;
	int alpha_test_func;
	float alpha_test_ref;
	
	// Depth
	int z_enable;
	int z_mask;
	int z_func;
	
	// Culling
	int cull_mode;
	
	// stencil
	int stencil_enable;
	int stencil_func_b;
	int stencil_func;
	int stencil_op_b;
	int stencil_op_fail;
	int stencil_op_zfail;
	int stencil_op;
	int stencil_ref_b;
	int stencil_ref;
	int stencil_mask_b;
	int stencil_mask;
	int stencil_write_b;
	int stencil_write;
	
	// viewport
	int viewport_x;
	int viewport_y;
	int viewport_w;
	int viewport_h;
	int viewport_zn;
	int viewport_zf;
	
	// other
	int fill_mode_front;
	int fill_mode_back;
	
	// Reupload all states
	int dirty;
	
} xe_states_t;

xe_states_t xe_state;
void XeInitStates();
void XeUpdateStates();

/***********************************************************************
 * Global
 ***********************************************************************/
struct XenosDevice _xe, *xe;
struct XenosShader * pVertexShader;
struct XenosShader * pPixelColorShader;
struct XenosShader * pPixelModulateShader;
struct XenosShader * pPixelModulateShader2;
struct XenosShader * pPixelTextureShader;
struct XenosShader * pCurrentPs;
struct XenosShader * pCurrentTexturedPs;
struct XenosVertexBuffer * pVbGL;
struct XenosIndexBuffer * pIbGL;

/***********************************************************************
 * Utils
 ***********************************************************************/
void xe_gl_error(const char * format, ...);
void xe_gl_log(const char * format, ...);
void XenonGLInit();
void XenonGLDisplay();
void XenonBeginGl();
void XenonEndGl();
void XeGLInitTextures();
void GL_InitShaderCache();
void XeRefreshAllDirtyTextures();
void XeGetScreenSize(int * width, int * height);
