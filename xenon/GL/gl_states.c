#include "gl_xenos.h"
#include <debug.h>

GLenum gl_cull_mode = GL_BACK;
static GLboolean gl_cull_enable = GL_FALSE;
GLenum gl_front_face = GL_CCW;

// Missing from libxenon
#define XE_NONE_FRONTFACE_CCW  0x0
#define XE_FRONT_FRONTFACE_CCW 0x1
#define XE_BACK_FRONTFACE_CCW  0x2
#define XE_NONE_FRONTFACE_CW   0x4
#define XE_FRONT_FRONTFACE_CW  0x5
#define XE_BACK_FRONTFACE_CW   0x6

void glShadeModel (GLenum mode)
{
	
}

/***********************************************************************
 * Clear
 ***********************************************************************/
static int scissor_x, scissor_y, scissor_w, scissor_h;

static void updateScissor(int enabled)
{
	//Xe_SetScissor(xe, enabled, scissor_x, scissor_y, scissor_x+scissor_w, scissor_y+scissor_h);
}

void glScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
	scissor_x = x;
	scissor_y = y;
	scissor_w = width;
	scissor_h = height;
	
	//Xe_SetScissor(xe, 1, scissor_x, scissor_y, scissor_x+scissor_w, scissor_y+scissor_h);
}

/***********************************************************************
 * Clear
 ***********************************************************************/
void glClear (GLbitfield mask)
{
	unsigned int flags = 0;

	if (mask & GL_COLOR_BUFFER_BIT) flags |= XE_CLEAR_COLOR;
	if (mask & GL_DEPTH_BUFFER_BIT) flags |= XE_CLEAR_DS;
	if (mask & GL_STENCIL_BUFFER_BIT) flags |= XE_CLEAR_DS;

	Xe_Clear(xe, flags);
}

#define MAKE_COLOR4(a,r,g,b) ((unsigned int)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	unsigned int c = MAKE_COLOR4((int)(red * 255), (int)(green * 255), (int)(blue * 255), (int)(alpha * 255));
	Xe_SetClearColor(xe, c);
}

void glClearDepth(GLclampd depth)
{
	Xe_Clear(xe, XE_CLEAR_DS);
}

void glClearStencil(GLint s)
{
	Xe_Clear(xe, XE_CLEAR_DS);
}

/***********************************************************************
 * Cull
 ***********************************************************************/
static void updateCullMode()
{
#if 1
	if (gl_cull_enable == GL_FALSE)
	{
		// disable culling
		xe_state.cull_mode = XE_CULL_NONE;
		return;
	}
	
	if (gl_cull_mode == GL_FRONT_AND_BACK) {
		printf("GL_FRONT_AND_BACK: not implemented \n");
		return;
	}

	if (gl_front_face == GL_CCW)
	{
		if (gl_cull_mode == GL_BACK)
			xe_state.cull_mode = XE_BACK_FRONTFACE_CCW;
		else if (gl_cull_mode == GL_FRONT)
			xe_state.cull_mode = XE_FRONT_FRONTFACE_CCW;
		else xe_gl_error ("GL_UpdateCull: illegal glCullFace\n");
	}
	else if (gl_front_face == GL_CW)
	{
		if (gl_cull_mode == GL_BACK)
			xe_state.cull_mode = XE_BACK_FRONTFACE_CW;
		else if (gl_cull_mode == GL_FRONT)
			xe_state.cull_mode = XE_FRONT_FRONTFACE_CW;
		else xe_gl_error ("GL_UpdateCull: illegal glCullFace\n");
	}
	else 
		xe_gl_error ("GL_UpdateCull: illegal glFrontFace\n");
#else 
	if (gl_cull_enable == GL_FALSE)
	{
		// disable culling
		Xe_SetCullMode(xe, XE_CULL_NONE);
		return;
	}
	if (gl_front_face == GL_CCW) {
		Xe_SetCullMode(xe, XE_CULL_CW);
	} else {
		Xe_SetCullMode(xe, XE_CULL_CCW);
	}
#endif
}
void glFrontFace (GLenum mode)
{
	gl_front_face = mode;
	updateCullMode();
	xe_state.dirty = 1;
}
void glCullFace (GLenum mode)
{
	gl_cull_mode = mode;
	updateCullMode();
	xe_state.dirty = 1;
}
/***********************************************************************
 * Depth
 ***********************************************************************/
int Gl_Cmp_2_Xe(GLenum mode)
{
	int cmp = 0;
	switch (mode)
	{
	case GL_NEVER:
		cmp = XE_CMP_NEVER;
		break;

	case GL_LESS:
		cmp = XE_CMP_LESS;
		break;

	case GL_LEQUAL:
		cmp = XE_CMP_LESSEQUAL;
		break;

	case GL_EQUAL:
		cmp = XE_CMP_EQUAL;
		break;

	case GL_GREATER:
		cmp = XE_CMP_GREATER;
		break;

	case GL_NOTEQUAL:
		cmp = XE_CMP_NOTEQUAL;
		break;

	case GL_GEQUAL:
		cmp = XE_CMP_GREATEREQUAL;
		break;

	case GL_ALWAYS:
	default:
		cmp = XE_CMP_ALWAYS;
		break;
	}
	return cmp;
} 
int Gl_ZCmp_2_Xe(GLenum mode)
{
	int cmp = 0;
	switch (mode)
	{
	case GL_NEVER:
		cmp = XE_CMP_NEVER;
		break;

	case GL_LESS:
		cmp = XE_CMP_GREATER;
		break;

	case GL_LEQUAL:
		cmp = XE_CMP_GREATEREQUAL;
		break;

	case GL_EQUAL:
		cmp = XE_CMP_EQUAL;
		break;

	case GL_GREATER:
		cmp = XE_CMP_LESS;
		break;

	case GL_NOTEQUAL:
		cmp = XE_CMP_NOTEQUAL;
		break;

	case GL_GEQUAL:
		cmp = XE_CMP_LESSEQUAL;
		break;

	case GL_ALWAYS:
	default:
		cmp = XE_CMP_ALWAYS;
		break;
	}
	return cmp;
} 

void glDepthFunc (GLenum func)
{
	xe_state.z_func = Gl_Cmp_2_Xe(func);
	xe_state.dirty = 1;
}

void glDepthMask (GLboolean flag)
{
	xe_state.z_mask = (flag == GL_TRUE) ? 1 : 0;
	xe_state.dirty = 1;
}

void glAlphaFunc (GLenum func, GLclampf ref)
{
	int cmp = Gl_Cmp_2_Xe(func);
	if (cmp == XE_CMP_GREATEREQUAL) {
		ref -= 1.f/255.f;
		cmp = XE_CMP_GREATER;
	}
	
	xe_state.alpha_test_func = cmp;
	xe_state.alpha_test_ref = ref;
	xe_state.dirty = 1;
}

/***********************************************************************
 * Blend
 ***********************************************************************/
int Gl_Blend_2_Xe(GLenum factor)
{
	int blend = XE_BLEND_ONE;

	switch (factor)
	{
	case GL_ZERO:
		blend = XE_BLEND_ZERO;
		break;

	case GL_ONE:
		blend = XE_BLEND_ONE;
		break;

	case GL_SRC_COLOR:
		blend = XE_BLEND_SRCCOLOR;
		break;

	case GL_ONE_MINUS_SRC_COLOR:
		blend = XE_BLEND_INVSRCCOLOR;
		break;

	case GL_DST_COLOR:
		blend = XE_BLEND_DESTCOLOR;
		break;

	case GL_ONE_MINUS_DST_COLOR:
		blend = XE_BLEND_INVDESTCOLOR;
		break;

	case GL_SRC_ALPHA:
		blend = XE_BLEND_SRCALPHA;
		break;

	case GL_ONE_MINUS_SRC_ALPHA:
		blend = XE_BLEND_INVSRCALPHA;
		break;

	case GL_DST_ALPHA:
		blend = XE_BLEND_DESTALPHA;
		break;

	case GL_ONE_MINUS_DST_ALPHA:
		blend = XE_BLEND_INVDESTALPHA;
		break;

	case GL_SRC_ALPHA_SATURATE:
		blend = XE_BLEND_SRCALPHASAT;
		break;

	default:
		xe_gl_error("glBlendFunc: unknown factor");
	}
	return blend;
}


void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	xe_state.blend_src = Gl_Blend_2_Xe(sfactor);
	xe_state.blend_dst = Gl_Blend_2_Xe(dfactor);
	xe_state.dirty = 1;
}

/***********************************************************************
 * Blend
 ***********************************************************************/
void GlEnableDisable(GLenum cap, int enable)
{
	switch (cap)
	{
	case GL_SCISSOR_TEST:
		updateScissor(enable);
		break;
		
	case GL_BLEND:
		xe_state.blend_enable = enable;
		break;
		
	case GL_ALPHA_TEST:
		xe_state.alpha_test_enabled = enable;
		break;

	case GL_TEXTURE_2D:
		xeTmus[xeCurrentTMU].enabled = enable;
		return;

	case GL_CULL_FACE:
		if (!enable)
			gl_cull_enable = GL_FALSE;
		else
			gl_cull_enable = GL_TRUE;
		updateCullMode();
		break;
		
	case GL_DEPTH_TEST:
		xe_state.z_enable = enable;
		break;
		
	case GL_FOG:
		return;
	case GL_POLYGON_OFFSET_FILL:
	case GL_POLYGON_OFFSET_LINE:
	default:
		return;
	}
	
	xe_state.dirty = 1;
}

void glEnable(GLenum cap)
{
	GlEnableDisable(cap, 1);
}

void glDisable(GLenum cap)
{
	GlEnableDisable(cap, 0);
}

/***********************************************************************
 * Stencil
 ***********************************************************************/
void glStencilFunc(GLenum func,	GLint ref,GLuint mask)
{
	xe_state.dirty = 1;
}

void glStencilMask(GLuint mask)
{
	xe_state.dirty = 1;
}

void glStencilOp(GLenum sfail,	GLenum  dpfail,	GLenum  dppass)
{
	xe_state.dirty = 1;
}
/***********************************************************************
 * Misc
 ***********************************************************************/
void glPolygonMode (GLenum face, GLenum mode)
{
	/*
	int xmode = 0;
	
	if (mode == GL_LINE)
		xmode = XE_FILL_WIREFRAME;
	else if (mode == GL_POINT)
		xmode = XE_FILL_POINT;
	else 
		xmode = XE_FILL_SOLID;
	
	if (face == GL_FRONT)
		xe_state.fill_mode_front = xmode;
	else if (face == GL_BACK)
		xe_state.fill_mode_back = xmode;
	else
		xe_state.fill_mode_front = xe_state.fill_mode_back = xmode;
		
	xe_state.dirty = 1;
	*/ 
}

void glPolygonOffset (GLfloat factor, GLfloat units)
{

}

void glClipPlane(GLenum plane,
                 const GLdouble *equation) {
					 
}

void glLineWidth(GLfloat width)
{
	
}

void glCallList(GLuint list)
{
	
}

void glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	
}

void glReadPixels(GLint x,	GLint y, GLsizei width,	GLsizei height,	GLenum format, GLenum type,
 	GLvoid * data)
{
	
}

void glGetBooleanv(	GLenum pname, GLboolean * params)
{
	 TR
	 glGetIntegerv(pname, (GLint *)params);
}

void glGetDoublev(GLenum pname,	GLdouble * params)
{
	TR
	glGetIntegerv(pname, (GLint *)params);
}

void glGetIntegerv(GLenum pname, GLint * params)
{
	TR;
// here we only bother getting the values that glquake uses
	switch (pname)
	{
	case GL_MAX_TEXTURE_SIZE:
		params[0] = 2048;
		break;

	case GL_VIEWPORT:
		// todo
		break;

	case GL_STENCIL_BITS:
		params[0] = 8; // ?
		break;

	default:
		
		params[0] = 0;
		return;
	}
		
}


/***********************************************************************
 * States Management
 ***********************************************************************/
void XeUpdateStates() {
	if (xe_state.dirty) {
		// blending		
		if (xe_state.blend_enable) {
			Xe_SetBlendControl(xe, xe_state.blend_src, xe_state.blend_op, xe_state.blend_dst, xe_state.blend_src, xe_state.blend_op, xe_state.blend_dst);
		}
		else {
			Xe_SetBlendControl(xe, XE_BLEND_ONE, XE_BLENDOP_ADD, XE_BLEND_ZERO, XE_BLEND_ONE, XE_BLENDOP_ADD, XE_BLEND_ZERO);
		}
		
		// Alpha test		
		Xe_SetAlphaTestEnable(xe, xe_state.alpha_test_enabled);		
		Xe_SetAlphaFunc(xe, xe_state.alpha_test_func);
		Xe_SetAlphaRef(xe, xe_state.alpha_test_ref);
		
		// Depth
		Xe_SetZEnable(xe, xe_state.z_enable);
		Xe_SetZWrite(xe, xe_state.z_mask);		
		Xe_SetZFunc(xe, xe_state.z_func);
		
		// Culling
		Xe_SetCullMode(xe, xe_state.cull_mode);

#if 0
		// Stencil
		Xe_SetStencilEnable(xe, xe_state.stencil_enable);
		Xe_SetStencilFunc(xe, xe_state.stencil_func_b, xe_state.stencil_func);

		/* -1 to leave old value */
		Xe_SetStencilOp(xe, xe_state.stencil_op_b, xe_state.stencil_op_fail, xe_state.stencil_op_zfail, xe_state.stencil_op);

		Xe_SetStencilRef(xe, xe_state.stencil_ref_b, xe_state.stencil_ref);
		Xe_SetStencilMask(xe, xe_state.stencil_mask_b, xe_state.stencil_mask);
		Xe_SetStencilWriteMask(xe, xe_state.stencil_write_b, xe_state.stencil_write);
#endif
		// other		
		//Xe_SetFillMode(xe, xe_state.fill_mode_front, xe_state.fill_mode_back);
		//Xe_SetFillMode(xe, XE_FILL_SOLID, XE_FILL_SOLID);
		
		xe_state.dirty = 0;
	}
}

void XeInitStates() {
	xe_state.fill_mode_back = xe_state.fill_mode_front = XE_FILL_SOLID;	
	xe_state.cull_mode = XE_CULL_NONE;
	
	xe_state.blend_src = XE_BLEND_ONE;
	xe_state.blend_op = XE_BLENDOP_ADD;
	xe_state.blend_dst = XE_BLEND_ZERO;
	
	xe_state.alpha_test_enabled = 0;
	xe_state.alpha_test_func = 0;
	xe_state.alpha_test_ref = 0;
	
	xe_state.z_enable = 0;
	xe_state.z_mask = 0;
	xe_state.z_func = 0;
	
	xe_state.dirty = 1;
}

