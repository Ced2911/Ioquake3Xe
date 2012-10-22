#include "gl_xenos.h"
#include <debug.h>

/*
#define XE_PRIMTYPE_POINTLIST 1
#define XE_PRIMTYPE_LINELIST 2
#define XE_PRIMTYPE_LINESTRIP 3
#define XE_PRIMTYPE_TRIANGLELIST 4
#define XE_PRIMTYPE_TRIANGLEFAN 5
#define XE_PRIMTYPE_TRIANGLESTRIP 6
#define XE_PRIMTYPE_RECTLIST 8
#define XE_PRIMTYPE_QUADLIST 13
*/

extern GLenum gl_cull_mode;
static int use_indice_buffer = 0;

static int Gl_Prim_2_Xe_Prim(GLenum mode)
{
	// default to this
	int ret = XE_PRIMTYPE_TRIANGLELIST;
	switch (xe_PrimitiveMode) {
		case GL_TRIANGLE_FAN:
		case GL_POLYGON:
			ret = XE_PRIMTYPE_TRIANGLEFAN;
			break;
		case GL_TRIANGLES:
			ret = XE_PRIMTYPE_TRIANGLELIST;
			break;
		case GL_TRIANGLE_STRIP:
			ret = XE_PRIMTYPE_TRIANGLESTRIP;
			break;
		case GL_POINTS:
			ret = XE_PRIMTYPE_POINTLIST;
			break;
		case GL_LINES:
			ret = XE_PRIMTYPE_LINELIST;
			break;
		case GL_QUAD_STRIP:
		case GL_QUADS:
			ret = XE_PRIMTYPE_QUADLIST;
			break;
		default:
			xe_gl_error("Unknow prim : %x\n", mode);
			break;
	}
	
	return ret;
}

static int Gl_Prim_2_Size(GLenum mode, int size) {
	// default to this
	int ret = size;
	switch (xe_PrimitiveMode) {
		case GL_TRIANGLES:
			ret = size / 3;
			break;
		case GL_POLYGON:
		case GL_TRIANGLE_FAN:
		case GL_TRIANGLE_STRIP:
			// 2 = first triangle
			ret = size - 2;
			break;
		// don't know ...
		case GL_POINTS:
		case GL_LINES:
			ret = size;
			break;
		case GL_QUAD_STRIP:
		case GL_QUADS:
			ret = size/4;
			break;
	}
	
	return ret;
}

enum {
	XE_ENV_MODE_DISABLED = 0,
	XE_ENV_MODE_REPLACE,
	XE_ENV_MODE_MODULATE,
	XE_ENV_MODE_ADD,		// not implemented
	XE_ENV_MODE_DECAL,		// not implemented
	XE_ENV_MODE_BLEND,		// not implemented
	XE_ENV_MAX
};

typedef union {
	struct {
		unsigned int tmu_env_mode:4;	//	4
	} states[XE_MAX_TMUS];				// 	2 * 4 > 8
	
	unsigned int hash;
} pixel_shader_pipeline_t;

typedef struct pixel_shader_cache_s {
	unsigned int hash;
	void * code;
} pixel_shader_cache_t;

static pixel_shader_cache_t cache[XE_ENV_MAX * XE_MAX_TMUS];


void GL_InitShaderCache() {
	pixel_shader_pipeline_t tmp;
	memset(&cache, 0, (XE_ENV_MAX * XE_MAX_TMUS) * sizeof(pixel_shader_cache_t));
	
	// some know shaders
	
	// SIMPLE COLOR
	tmp.hash = 0;
	cache[0].hash = tmp.hash;
	cache[0].code = (void*)pPixelColorShader;
	
	// MODULATE COLOR * TEX 1
	tmp.hash = 0;
	tmp.states[0].tmu_env_mode = XE_ENV_MODE_MODULATE;
	cache[1].hash = tmp.hash;
	cache[1].code = (void*)pPixelModulateShader;
		
	// REPLACE TEX 1
	tmp.hash = 0;
	tmp.states[0].tmu_env_mode = XE_ENV_MODE_REPLACE;
	cache[2].hash = tmp.hash;
	cache[2].code = (void*)pPixelTextureShader;
	
	// MODULATE COLOR * TEX 1 * TEX 2
	tmp.hash = 0;
	tmp.states[0].tmu_env_mode = XE_ENV_MODE_MODULATE;
	tmp.states[1].tmu_env_mode = XE_ENV_MODE_MODULATE;	
	cache[3].hash = tmp.hash;
	cache[3].code = (void*)pPixelModulateShader2;
}

// GL_TEXTURE_ENV_MODE defaults to GL_MODULATE
static void GL_SelectShaders() {
	int i = 0;
	pixel_shader_pipeline_t shader;
	shader.hash = 0;
	
	for (i=0; i<XE_MAX_TMUS; i++) {    
		// set texture
		if (xeTmus[i].enabled && xeTmus[i].boundtexture) {
			switch(xeTmus[i].texture_env_mode) {
				case GL_REPLACE:
					shader.states[i].tmu_env_mode = XE_ENV_MODE_REPLACE;
					break;
				default:
				case GL_MODULATE:
					shader.states[i].tmu_env_mode = XE_ENV_MODE_MODULATE;
					break;
			}
		}
		// no more texture used
		else {
			break;
		}
	}
	// color only !!!
	if (shader.hash == 0) {
		Xe_SetShader(xe, SHADER_TYPE_PIXEL, pPixelColorShader, 0);
		return;
	}
	
	// look into cache
	for (i=0; i<XE_ENV_MAX * XE_MAX_TMUS; i++) {
		if (cache[i].hash) {
			if (cache[i].hash == shader.hash) {
				Xe_SetShader(xe, SHADER_TYPE_PIXEL, cache[i].code, 0);
				return;
			}
		}
	}
	
	// create it and add it to cache
	/* todo */
	printf("Unknow hash : %d\n", shader.hash);
}


static void GL_SubmitVertexes()
{	
	// never draw this one
	if (gl_cull_mode == GL_FRONT_AND_BACK)
		return;
		
	// Xe_SetFillMode(xe, XE_FILL_WIREFRAME, XE_FILL_WIREFRAME);
		
	// update states if dirty
	XeUpdateStates();

	// update if dirty
	XeGlCheckDirtyMatrix(&projection_matrix);
	XeGlCheckDirtyMatrix(&modelview_matrix);
	
    // Xe_SetStreamSource(xe, 0, pVbGL, xe_PrevNumVerts * sizeof(glVerticesFormat_t), 10);
    Xe_SetShader(xe, SHADER_TYPE_VERTEX, pVertexShader, 0);
    
    int i = 0;
    // setup texture    
    for(i=0; i<XE_MAX_TMUS; i++) {    
		// set texture
		if (xeTmus[i].enabled && xeTmus[i].boundtexture) {
			Xe_SetTexture(xe, i, xeTmus[i].boundtexture->teximg);
		}
		else {
			Xe_SetTexture(xe, i, NULL);
		}
	}
	// setup shaders
	GL_SelectShaders();
	
	// draw
	if (use_indice_buffer == 0)
	{	
		/*  Xe_DrawPrimitive(struct XenosDevice *xe, int type, int start, int primitive_count) */
		Xe_DrawPrimitive(xe, Gl_Prim_2_Xe_Prim(xe_PrimitiveMode), xe_PrevNumVerts, Gl_Prim_2_Size(xe_PrimitiveMode, (xe_NumVerts - xe_PrevNumVerts)));
	}
	else {
		/* Xe_DrawIndexedPrimitive(struct XenosDevice *xe, int type, int base_index, int min_index, int num_vertices, int start_index, int primitive_count) */
		Xe_SetIndices(xe, pIbGL);
		Xe_DrawIndexedPrimitive(xe, Gl_Prim_2_Xe_Prim(xe_PrimitiveMode), 
		xe_PrevNumVerts, 0,
		(xe_NumVerts - xe_PrevNumVerts), xe_PrevNumIndices, Gl_Prim_2_Size(xe_PrimitiveMode, (xe_NumIndices - xe_PrevNumIndices)));
	}
	
	//printBlendValue();
}

void glBegin(GLenum mode)
{
	xe_PrimitiveMode = mode;
	
	xe_PrevNumVerts = xe_NumVerts;
	xe_PrevNumIndices = xe_NumIndices;
}

void glEnd()
{
	// submit vertices
	GL_SubmitVertexes();
	use_indice_buffer = 0;
};


void glVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
	// add a new vertex to the list with the specified xyz and inheriting the current normal, color and texcoords
	// (per spec at http://www.opengl.org/sdk/docs/man/xhtml/glVertex.xml)
#if 0	
	xe_Vertices[xe_NumVerts].x = x;
	xe_Vertices[xe_NumVerts].y = y;
	xe_Vertices[xe_NumVerts].z = z;
	xe_Vertices[xe_NumVerts].w = 1;

	xe_Vertices[xe_NumVerts].color = xe_CurrentColor.u32;

	xe_Vertices[xe_NumVerts].u0 = xe_TextCoord[0].u;
	xe_Vertices[xe_NumVerts].v0 = xe_TextCoord[0].v;
	xe_Vertices[xe_NumVerts].u1 = xe_TextCoord[1].u;
	xe_Vertices[xe_NumVerts].v1 = xe_TextCoord[1].v;	
#else 
	union {
		float f;
		unsigned int u32;
	} c;
	
	c.u32 = xe_CurrentColor.u32;
	
	*xe_Vertices++ = x;
	*xe_Vertices++ = y;
	*xe_Vertices++ = z;
	*xe_Vertices++ = 1;

	/* 
*xe_Vertices++ = xe_TextCoord[0].u;
*xe_Vertices++ = xe_TextCoord[0].v;
*xe_Vertices++ = xe_TextCoord[1].u;
*xe_Vertices++ = xe_TextCoord[1].v;
	*/

	int i = 0;
	for(i = 0; i < XE_MAX_TMUS; i++) {
		*xe_Vertices++ = xe_TextCoord[i].u;
		*xe_Vertices++ = xe_TextCoord[i].v;
	}
	*xe_Vertices++ = c.f;
#endif	
	// next vertex
	xe_NumVerts++;
}

void glVertex2fv (const GLfloat *v)
{
	glVertex3f (v[0], v[1], 0);
}


void glVertex2f (GLfloat x, GLfloat y)
{
	glVertex3f (x, y, 0);
}


void glVertex3fv (const GLfloat *v)
{
	glVertex3f (v[0], v[1], v[2]);
}

void glTexCoord2f (GLfloat s, GLfloat t)
{
	xe_TextCoord[0].u = s;
	xe_TextCoord[0].v = t;
}


void glTexCoord2fv (const GLfloat *v)
{
	xe_TextCoord[0].u = v[0];
	xe_TextCoord[0].v = v[1];
}


void glFinish (void)
{
	
}

GLenum glGetError(){
	return GL_NO_ERROR;
}

void glPointSize(GLfloat size)
{
	
}
void glPointParameterf(	GLenum pname, GLfloat param)
{
	
}

void glPointParameterfv(GLenum pname,const GLfloat *  params)
{
	
}

/***********************************************************************
 * 
 * Batch Rendering
 * 
 **********************************************************************/ 
 
typedef struct gl_varray_pointer_s
{
	GLint size;
	GLenum type;
	GLsizei stride;
	GLvoid *pointer;
} gl_varray_pointer_t;

static gl_varray_pointer_t vertexPointer;
static gl_varray_pointer_t colorPointer;
static gl_varray_pointer_t texCoordPointer[XE_MAX_TMUS];
static int vArray_TMU = 0;

void glDrawBuffer (GLenum mode)
{
	// TR
}
 
void glArrayElement(GLint i)
{
	//TR
	use_indice_buffer = 1;
	xe_indices[0] = i;
	
	xe_indices++;
}

void glDrawElements(GLenum mode, GLsizei count,	GLenum type, const GLvoid *	indices)
{
	TR
}

void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *	pointer)
{
	if ((type == GL_FLOAT) || (type == GL_UNSIGNED_BYTE)) {	
		colorPointer.size = size;
		colorPointer.type = type;
		colorPointer.stride = stride;
		colorPointer.pointer = (GLvoid *) pointer;
	} else {
		xe_gl_error("Unimplemented color pointer type");
	}
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	if (type != GL_FLOAT) 
		xe_gl_error("Unimplemented texcoord pointer type\n");

	texCoordPointer[vArray_TMU].size = size;
	texCoordPointer[vArray_TMU].type = type;
	texCoordPointer[vArray_TMU].stride = stride;
	texCoordPointer[vArray_TMU].pointer = (GLvoid *) pointer;
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	if (type != GL_FLOAT) 
		xe_gl_error("Unimplemented vertex pointer type");

	vertexPointer.size = size;
	vertexPointer.type = type;
	vertexPointer.stride = stride;
	vertexPointer.pointer = (GLvoid *) pointer;
}

void glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
	int i;
	int v;
	int tmu;
	unsigned char *vp;
	unsigned char *cp;
	unsigned char *stp[XE_MAX_TMUS];

	// required by the spec
	if (!vertexPointer.pointer) return;

	vp = ((unsigned char *) vertexPointer.pointer + first);
	cp = ((unsigned char *) colorPointer.pointer + first);

	for (tmu = 0; tmu < XE_MAX_TMUS; tmu++)
	{
		if (texCoordPointer[tmu].pointer)
			stp[tmu] = ((unsigned char *) texCoordPointer[tmu].pointer + first);
		else stp[tmu] = NULL;
	}

	// send through standard begin/end processing
	glBegin (mode);

	for (i = 0, v = first; i < count; i++, v++)
	{
		for (tmu = 0; tmu < XE_MAX_TMUS; tmu++)
		{
			if (stp[tmu])
			{
				xe_TextCoord[tmu].u = ((float *) stp[tmu])[0];
				xe_TextCoord[tmu].v = ((float *) stp[tmu])[1];

				stp[tmu] += texCoordPointer[tmu].stride;
			}
		}
		
		if (colorPointer.size == 4 && colorPointer.type == GL_UNSIGNED_BYTE) {
			glColor4ubv(cp);
		}
		cp += colorPointer.stride;

		if (vertexPointer.size == 2)
			glVertex2fv ((float *) vp);
		else if (vertexPointer.size == 3)
			glVertex3fv ((float *) vp);

		vp += vertexPointer.stride;
	}

	glEnd ();
}

void glEnableClientState(GLenum array)
{
	
}

void glDisableClientState (GLenum array)
{
	// switch the pointer to NULL
	switch (array)
	{
	case GL_VERTEX_ARRAY:
		vertexPointer.pointer = NULL;
		break;

	case GL_COLOR_ARRAY:
		colorPointer.pointer = NULL;
		break;

	case GL_TEXTURE_COORD_ARRAY:
		texCoordPointer[vArray_TMU].pointer = NULL;
		break;

	default:
		xe_gl_error("Invalid Vertex Array Spec...!\n");
	}
}
