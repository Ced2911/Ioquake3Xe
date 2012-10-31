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
void GL_SelectShaders() {
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


void GL_SelectTextures() {
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
}

static void GL_SubmitVertexes()
{	
	// never draw this one
	if (gl_cull_mode == GL_FRONT_AND_BACK)
		return;
	//Xe_SetFillMode(xe, XE_FILL_WIREFRAME, XE_FILL_WIREFRAME);
		
	// update states if dirty
	XeUpdateStates();

	// update if dirty
	XeGlCheckDirtyMatrix(&projection_matrix);
	XeGlCheckDirtyMatrix(&modelview_matrix);
	
    // Xe_SetStreamSource(xe, 0, pVbGL, xe_PrevNumVerts * sizeof(glVerticesFormat_t), 10);
    Xe_SetShader(xe, SHADER_TYPE_VERTEX, pVertexShader, 0);
    
   
	// setup shaders and textures
	GL_SelectShaders();
	GL_SelectTextures();
	
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
		0, 0,
		(xe_NumVerts - xe_PrevNumVerts), xe_PrevNumIndices, (xe_NumIndices - xe_PrevNumIndices));
		
		/*
		Xe_DrawIndexedPrimitive(xe,XE_PRIMTYPE_TRIANGLELIST, 
		0, 0,
		(xe_NumVerts - xe_PrevNumVerts), xe_PrevNumIndices, 2);
		*/ 
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
	xe_CurrentColor.u32 = 0;
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

typedef struct gl_indices_pointer_s
{
	GLenum mode;
	GLenum type;
	GLvoid *pointer;
} gl_indices_pointer_t;

typedef struct gl_vertices_pointer_s
{
	GLint count;
	GLint size;
	GLenum type;
	GLsizei stride;
	GLvoid *pointer;
} gl_vertices_pointer_t;


static gl_indices_pointer_t indexPointer;
static gl_vertices_pointer_t vertexPointer;
static gl_varray_pointer_t colorPointer;
static gl_varray_pointer_t texCoordPointer[XE_MAX_TMUS];
static int vArray_TMU = 0;

/** todo **/
void glClientActiveTexture(GLenum texture)
{
	vArray_TMU = 0;
}

void glDrawBuffer (GLenum mode)
{
	// TR
}
 
void glArrayElement(GLint i)
{
	use_indice_buffer = 1;	
	//*xe_indices++ = i + xe_PrevNumVerts;
	*xe_indices++ = i + xe_NumVerts;
	xe_NumIndices++;
}


void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *	pointer)
{
	if ((type == GL_FLOAT) || (type == GL_UNSIGNED_BYTE)) {	
		// Packed
		if (stride == 0) {
			if ( type == GL_FLOAT )	{
				stride = sizeof(float) * size;
			} else if(type == GL_UNSIGNED_BYTE)	{
				stride = sizeof(char) * size;
			}
		}
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

	// Packed
	if (stride == 0) {
		stride = sizeof(float) * size;
	}

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

void glLockArraysEXT(int first, int count) {
	#if 0
	unsigned char * current_vertice;
	unsigned char * current_textcoord;
	unsigned char * current_color;
	int i;
	int vertice_count = vertexPointer.count;
	vertexPointer.count = count;
	
	// Begin
	xe_PrevNumVerts = xe_NumVerts;
	
	// Fill vertices		
	current_vertice = (unsigned char *) vertexPointer.pointer;
	current_textcoord = (unsigned char *) texCoordPointer[vArray_TMU].pointer;
	current_color = (unsigned char *) colorPointer.pointer;
	
	for(i=0; i<vertice_count; i++) {
		/*
		if (current_color) {
			glColor4ubv((GLfloat*)current_color);
		}
		
		if (current_textcoord) {
			glTexCoord2fv((GLfloat*)current_textcoord);
		}
		*/
		glVertex3fv((GLfloat*)current_vertice);
		
		current_vertice+=vertexPointer.stride;
		/*
		current_textcoord+=texCoordPointer[vArray_TMU].stride;
		current_color+=colorPointer.stride;
		*/ 
	}
	#endif
}

void glUnlockArraysEXT(void) {
	
}

void glDrawElements(GLenum mode, GLsizei indice_count,	GLenum type, const GLvoid *	indices)
{
	#if 1
	int i;
	int vertice_count = vertexPointer.count;
	unsigned short * current_indice;
	
	if ( vertexPointer.pointer == NULL || indices== NULL ) {
		// no vertice, no indices leave
		return;
	}
	
	// Begin
	// xe_PrevNumVerts = xe_NumVerts;
	xe_PrevNumIndices = xe_NumIndices;
	
	// add indices info
	indexPointer.mode = mode;
	indexPointer.type = type;
	indexPointer.pointer = (GLvoid *) indices;
	
	// Debugging
	Xe_SetFillMode(xe, XE_FILL_WIREFRAME, XE_FILL_WIREFRAME);
		
	// update states if dirty
	XeUpdateStates();

	// update if dirty
	XeGlCheckDirtyMatrix(&projection_matrix);
	XeGlCheckDirtyMatrix(&modelview_matrix);
	
    // Xe_SetStreamSource(xe, 0, pVbGL, xe_PrevNumVerts * sizeof(glVerticesFormat_t), 10);
    Xe_SetShader(xe, SHADER_TYPE_VERTEX, pVertexShader, 0);
    
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
		
	// Fill indices
	current_indice = (unsigned short *) indexPointer.pointer;	
	for(i=0; i < indice_count; i++) {
		xe_indices[i+xe_PrevNumIndices] = xe_PrevNumVerts + current_indice[i];
	}
	xe_NumIndices += indice_count;
	
	
	// draw
	/* Xe_DrawIndexedPrimitive(struct XenosDevice *xe, int type, int base_index, int min_index, int num_vertices, int start_index, int primitive_count) */
	Xe_SetIndices(xe, pIbGL);
	
	Xe_DrawIndexedPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 
		xe_PrevNumVerts, 0,
		vertice_count, xe_PrevNumIndices, (indice_count / 3)
	);
	#endif
}

void glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
	TR
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
