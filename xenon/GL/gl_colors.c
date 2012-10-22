#include "gl_xenos.h"

#define BYTE_CLAMP(i) (int) ((((i) > 255) ? 255 : (((i) < 0) ? 0 : (i))))

#define COLOR_ARGB(a,b,g,r) ((unsigned int)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

unsigned int Gl_Color_2_Xe (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	return COLOR_ARGB
	(
		BYTE_CLAMP(alpha),
		BYTE_CLAMP(red),
		BYTE_CLAMP(green),
		BYTE_CLAMP(blue)
	);
}

void GL_SetColor (float red, float green, float blue, float alpha)
{
	// overwrite color incase verts set it
	xe_CurrentColor.u32 = Gl_Color_2_Xe(red, green, blue, alpha);
}

/*
void GL_SetColor (float red, float green, float blue, float alpha)
{
	xe_CurrentColor.a = alpha;
	xe_CurrentColor.r = red;
	xe_CurrentColor.g = green;
	xe_CurrentColor.b = blue;
}
*/
void glColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
	GL_SetColor (red * 255, green * 255, blue * 255, 255);
}


void glColor3fv (const GLfloat *v)
{
	GL_SetColor (v[0] * 255, v[1] * 255, v[2] * 255, 255);
}


void glColor3ubv (const GLubyte *v)
{
	GL_SetColor (v[0], v[1], v[2], 255);
}


void glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	GL_SetColor (red * 255, green * 255, blue * 255, alpha * 255);
}


void glColor4fv (const GLfloat *v)
{
	GL_SetColor (v[0] * 255, v[1] * 255, v[2] * 255, v[3] * 255);
}

void glColor4ubv (const GLubyte *v)
{
	GL_SetColor (v[0], v[1], v[2], v[3]);
}


void glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	GL_SetColor (red, green, blue, alpha);
}

