#include "gl_xenos.h"
#include <string.h>
#include <malloc.h>
#include <debug.h>
#include <assert.h>

#define XE_MAX_TEXTURE 50000

glXeSurface_t * glXeSurfaces = NULL;

#define TEXTURE_SLOT_EMPTY -1

#define TEXMIN 128

// #define USE_TILED_TEXTURE 1

static struct XenosSurface * MyXeCreateTexture(struct XenosDevice *xe, unsigned int width, unsigned int height, unsigned int levels, int format, int tiled)
{
	return Xe_CreateTexture(xe, width, height, levels, format, tiled);
}

static void * MyXeSurfaceLockRect(struct XenosDevice *xe, struct XenosSurface *surface, int x, int y, int w, int h, int flags)
{
	return Xe_Surface_LockRect(xe, surface, x, y, w, h, flags);
}

static void MyXeSurfaceUnlock(struct XenosDevice *xe, struct XenosSurface *surface)
{
	return Xe_Surface_Unlock(xe, surface);
}
#if 0
void XeRefreshTexture(glXeSurface_t * surf) {
	if (surf->dirty) {
		Xe_Surface_LockRect(xe, surf->teximg, 0, 0, 0, 0, XE_LOCK_WRITE);
		Xe_Surface_Unlock(xe, surf->teximg);
		// surface is now clean
		surf->dirty = 0;
	}
}

void XeRefreshAllDirtyTextures() {
	int i = 0;
	for (i = 0; i< XE_MAX_TEXTURE; i++)
	{
		XeRefreshTexture(&glXeSurfaces[i]);
	}
}
#endif

#ifndef USE_TILED_TEXTURE
static inline void handle_small_surface_u8(struct XenosSurface * surf, void * buffer)
{
	int width;
	int height;
	int wpitch;
	int hpitch;
	int yp,xp,y,x;
	uint8_t * surf_data;
	uint8_t * data;
	uint8_t * src;	

	// don't handle big texture
	if (surf->width>TEXMIN && surf->height>TEXMIN) {
		return;
	}	

	width = surf->width;
	height = surf->height;
	wpitch = surf->wpitch;
	hpitch = surf->hpitch;	

	if(buffer)
        surf_data = (uint8_t *)buffer;
    else
        surf_data = (uint8_t *)MyXeSurfaceLockRect(xe, surf, 0, 0, 0, 0, XE_LOCK_WRITE);

	src = data = surf_data;

	for(yp=height; yp<hpitch;yp+=height) {
		int max_h = height;
		if (yp + height> hpitch)
				max_h = hpitch % height;
		for(y = 0; y<max_h; y++){
			//x order
			for(xp = width;xp<wpitch;xp+=width) {
				int max_w = width;
				if (xp + width> wpitch)
					max_w = wpitch % width;

				for(x = 0; x<max_w; x++) {
					data[x+xp + ((y+yp)*wpitch)]=src[x+ (y*wpitch)];
				}
			}
		}
	}

    if(!buffer)
        MyXeSurfaceUnlock(xe, surf);
}

static inline void handle_small_surface(struct XenosSurface * surf, void * buffer){
	int width;
	int height;
	int wpitch;
	int hpitch;
	int yp,xp,y,x;
	uint32_t * surf_data;
	uint32_t * data;
	uint32_t * src;	

	// don't handle big texture
	if (surf->width>TEXMIN && surf->height>TEXMIN) {
		return;
	}
	if (surf->bypp == 1) {
		handle_small_surface_u8(surf, buffer);
		return;
	} 
	if (surf->bypp != 4) {
		return;
	}
	

	width = surf->width;
	height = surf->height;
	wpitch = surf->wpitch / 4;
	hpitch = surf->hpitch;	

	if(buffer)
        surf_data = (uint32_t *)buffer;
    else
        surf_data = (uint32_t *)MyXeSurfaceLockRect(xe, surf, 0, 0, 0, 0, XE_LOCK_WRITE);

	src = data = surf_data;

	for(yp=height; yp<hpitch;yp+=height) {
		int max_h = height;
		if (yp + height> hpitch)
				max_h = hpitch % height;
		for(y = 0; y<max_h; y++){
			//x order
			for(xp = width;xp<wpitch;xp+=width) {
				int max_w = width;
				if (xp + width> wpitch)
					max_w = wpitch % width;

				for(x = 0; x<max_w; x++) {
					data[x+xp + ((y+yp)*wpitch)]=src[x+ (y*wpitch)];
				}
			}
		}
	}

    if(!buffer)
        MyXeSurfaceUnlock(xe, surf);
} 
#endif

/***********************************************************************
 * Texture environnement
 ***********************************************************************/

void glTexEnvf (GLenum target, GLenum pname, GLfloat param)
{
	if (target != GL_TEXTURE_ENV) {
		xe_gl_error("glTexEnvf: unimplemented target\n");
		return;
	}
	
	if (!xeTmus[xeCurrentTMU].boundtexture) return;
	if (!xeTmus[xeCurrentTMU].boundtexture->teximg ) return;
	/** Do shader work here !! **/
	switch (pname)
	{
		case GL_TEXTURE_ENV_MODE:
			switch ((GLint)param)
			{
				case GL_REPLACE:
					xeTmus[xeCurrentTMU].texture_env_mode = (int)GL_REPLACE;
					break;

				case GL_MODULATE:
					xeTmus[xeCurrentTMU].texture_env_mode = (int)GL_MODULATE;			
					break;
				
				default:
					xe_gl_error("glTexEnvf: unimplemented param\n");
					break;
			}
			break;
		default:
			xe_gl_error("glTexEnvf: unimplemented pname\n");
			break;
	}
}

void glTexEnvi (GLenum target, GLenum pname, GLint param)
{
	glTexEnvf(target, pname, (GLfloat)param);
}

/***********************************************************************
 * Create/Gen/Delete images
 ***********************************************************************/
void XeGLInitTextures()
{
	int i = 0;
	glXeSurfaces = (glXeSurface_t *)memalign(128, sizeof(glXeSurface_t) * XE_MAX_TEXTURE);
	memset(glXeSurfaces, 0, sizeof(glXeSurface_t) * XE_MAX_TEXTURE);
	
	glXeSurface_t *tex;
	for (i = 0; i< XE_MAX_TEXTURE; i++)
	{
		tex = &glXeSurfaces[i];
		tex->glnum = TEXTURE_SLOT_EMPTY;
	}
}

int xeCurrentTMU = 0;

static void Xe_InitTexture(glXeSurface_t *tex)
{
	// slot is empty
	tex->glnum = TEXTURE_SLOT_EMPTY;

	// destroy texture
	if (tex->teximg)
	{
		Xe_DestroyTexture(xe, tex->teximg);
		tex->teximg = NULL;
	}
}

static glXeSurface_t *Xe_AllocTexture(void)
{
	int i = 0;
	glXeSurface_t *tex;

	// find a free texture
	for (i = 0; i< XE_MAX_TEXTURE; i++)
	{
		tex = &glXeSurfaces[i];
		// no texture
		if (!tex->teximg)
		{
			Xe_InitTexture(tex);
			return tex;
		}
		// free slot
		if (tex->glnum == TEXTURE_SLOT_EMPTY)
		{
			Xe_InitTexture(tex);
			return tex;
		}
	}

	xe_gl_error("Xe_AllocTexture: out of textures!!!\n");
	return NULL;
}


static void Xe_ReleaseTextures (void)
{
	glXeSurface_t *tex;
	int i;

	// explicitly NULL all textures and force texparams to dirty
	for (i = 0; i< XE_MAX_TEXTURE; i++)
	{
		tex = &glXeSurfaces[i];
		Xe_InitTexture(tex);
	}
} 
 
void glDeleteTextures(GLsizei n, const GLuint *textures)
{
	TR
	int i;
	glXeSurface_t *tex;
	
	for (i = 0; i < n; i++) {
		for (i = 0; i< XE_MAX_TEXTURE; i++)
		{
			tex = &glXeSurfaces[i];
			if (tex->glnum == textures[i]) {
				Xe_InitTexture(tex);
				break;
			}
		}
	}
}

static int texture_count = 1;

void glGenTextures(GLsizei n, GLuint *textures)
{
	int i;	
	
	for(i = 0; i < n; i++)
	{
		glXeSurface_t *tex = Xe_AllocTexture();
		tex->glnum = textures[i] = texture_count++;
	}
}

void glBindTexture(GLenum target, GLuint texture)
{
	int i;
	
	glXeSurface_t *tex;
	if (target != GL_TEXTURE_2D) 
		return;

	xeTmus[xeCurrentTMU].boundtexture = NULL;
	
	// find a texture
	for (i = 0; i< XE_MAX_TEXTURE; i++)
	{
		tex = &glXeSurfaces[i];
		
		if (tex && tex->glnum == texture)
		{
			xeTmus[xeCurrentTMU].boundtexture = tex;
			break;
		}
	}
	
	// did we find it?
	if (!xeTmus[xeCurrentTMU].boundtexture)
	{
		// nope, so fill in a new one (this will make it work with texture_extension_number)
		// (i don't know if the spec formally allows this but id seem to have gotten away with it...)
		xeTmus[xeCurrentTMU].boundtexture = Xe_AllocTexture();

		// reserve this slot
		xeTmus[xeCurrentTMU].boundtexture->glnum = texture;

		// ensure that it won't be reused
		if (texture > texture_count)
			texture_count = texture;
	}
	
	// this should never happen
	if (!xeTmus[xeCurrentTMU].boundtexture) 
		xe_gl_error("glBindTexture: out of textures!!!\n");
		
	// dirty the params
	xeTmus[xeCurrentTMU].texparamdirty = TRUE;
}

/***********************************************************************
 * Images manipulation
 ***********************************************************************/

void check_format(int srcbytes, int dstbytes) {
	int d = 0;
	
	if (srcbytes == 4) {
		if (dstbytes == 4) {
			d++;
		} else if(dstbytes == 1) {
			d++;
		}
	} else if (srcbytes == 3) {
		if (dstbytes == 4) {
			d++;
		} else if (dstbytes == 1) {
			d++;
		}
	} else if (srcbytes == 1) {
		if (dstbytes == 1) {
			d++;
		} else if (dstbytes == 4) {
			d++;
		}
	}
				
	if (d == 0) {
		printf("Error check that src = %d dst = %d\n", srcbytes, dstbytes);
	}
}

static inline int src_format_to_bypp(GLenum format)
{
	int ret = 0;
	if (format == 1 || format == GL_LUMINANCE)
		ret = 1;
	else if (format == 3 || format == GL_RGB)
		ret = 3;
	else if (format == 4 || format == GL_RGBA || format == GL_RGB8)
		ret = 4;
	else 
		xe_gl_error ("D3D_FillTextureLevel: illegal format");
	if (ret != 4)
		printf("src_format_to_bypp %d\n", ret);
	return ret;
}

static inline int dst_format_to_bypp(GLenum format)
{
	int ret = 0;
	if (format == 1 || format == GL_LUMINANCE)
		ret = 1;
	else if (format == 3 || format == GL_RGB)
		ret = 4;
	else if (format == 4 || format == GL_RGBA || format == GL_RGB8)
		ret = 4;
	else
		xe_gl_error ("D3D_FillTextureLevel: illegal format");
		
	if (ret != 4)
		printf("dst_format_to_bypp %d\n", ret);
	return ret;
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels)
{
	if (level > 0)
		return;
		
	if (!xeTmus[xeCurrentTMU].boundtexture) {
		printf("Not texture binded\n");
		return;
	}
		
	struct XenosSurface * surf = NULL;
	
	if (xeTmus[xeCurrentTMU].boundtexture && xeTmus[xeCurrentTMU].boundtexture->teximg ) {
		surf = xeTmus[xeCurrentTMU].boundtexture->teximg;
	}
	
	if (surf) {
		int srcbytes = src_format_to_bypp(format);
		int dstbytes = dst_format_to_bypp(xeTmus[xeCurrentTMU].boundtexture->internalformat);
		
		uint8_t * surfbuf = (uint8_t*) MyXeSurfaceLockRect(xe, surf, 0, 0, 0, 0, XE_LOCK_WRITE);
		uint8_t * srcdata = (uint8_t*) pixels;
		uint8_t * dstdata = surfbuf;
		
		int y, x;

		int pitch = (width * dstbytes);
		int offset = 0;
		
		check_format(srcbytes, dstbytes);

		for (y = yoffset; y < (yoffset + height); y++) {
			offset = (y * pitch)+(xoffset * dstbytes);
			dstdata = surfbuf + offset;
			
			for (x = xoffset; x < (xoffset + width); x++) {
				if (srcbytes == 4) {
					if (dstbytes == 4) {
						dstdata[0] = srcdata[3];
						dstdata[3] = srcdata[2];
						dstdata[2] = srcdata[1];
						dstdata[1] = srcdata[0];
					} else if(dstbytes == 1) {
						dstdata[0] = ((int) srcdata[0] + (int) srcdata[1] + (int) srcdata[2] + (int) srcdata[3]) / 3;
					}
						
					srcdata += srcbytes;
					dstdata += dstbytes;
					
				} else if (srcbytes == 3) {
					if (dstbytes == 4) {					
						dstdata[0] = 0xff;
						dstdata[3] = srcdata[2];
						dstdata[2] = srcdata[1];
						dstdata[1] = srcdata[0];
					} else if (dstbytes == 1) {
						dstdata[0] = ((int) srcdata[0] + (int) srcdata[1] + (int) srcdata[2]) / 3;
					}
						
					srcdata += srcbytes;
					dstdata += dstbytes;
				} else if (srcbytes == 1) {
					if (dstbytes == 1) {
						dstdata[0] = srcdata[0];
					} else if (dstbytes == 4) {
						dstdata[0] = srcdata[0];
						dstdata[1] = srcdata[0];
						dstdata[2] = srcdata[0];
						dstdata[3] = srcdata[0];
					}
					srcdata += srcbytes;
					dstdata += dstbytes;
				}
			}
		}
	
		MyXeSurfaceUnlock(xe, surf);		
		
		xeTmus[xeCurrentTMU].boundtexture->dirty = 1;

#ifndef USE_TILED_TEXTURE	
		handle_small_surface(surf, NULL);
#endif		
	}
}

void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum pixelformat, GLenum type, const GLvoid *pixels)
{
	int format;
	int need_texture = 1;
	struct XenosSurface * surf = NULL;
	
	if (type != GL_UNSIGNED_BYTE) {		
		xe_gl_error("glTexImage2D: Unrecognised pixel format\n");
		
		return;
	}
	if (target != GL_TEXTURE_2D)
		return;
		
	if (!xeTmus[xeCurrentTMU].boundtexture) {
		printf("No texture bound\n");
		return;
	}
	
	if (level > 0)
		return;
		
#if 1
	int srcbytes = src_format_to_bypp(pixelformat);
	int dstbytes = 4;
	
	// validate format
	switch (internalformat)
	{
	case 1:
	case GL_LUMINANCE:
		dstbytes = 1;
		format = XE_FMT_8;
		break;
	case 3:
	case GL_RGB:
		dstbytes = 4;
		format = XE_FMT_ARGB|XE_FMT_8888;
		break;
	case 4:
	case GL_RGB8:
	case GL_RGBA:
		dstbytes = 4;
		format = XE_FMT_ARGB|XE_FMT_8888;
		break;
	default:
		printf("%X internalformat\n" , internalformat);
		xe_gl_error ("invalid texture internal format\n");
		return;
	}
	
	if (xeTmus[xeCurrentTMU].boundtexture->teximg) {
		surf = xeTmus[xeCurrentTMU].boundtexture->teximg;
		if ((surf->width != width) || (surf->height != height) || (surf->format != format)) {
			need_texture = 1;
			// destroy texture
			Xe_DestroyTexture(xe, xeTmus[xeCurrentTMU].boundtexture->teximg);
		}
		else {
			need_texture = 0;
		}
	}
	
	if (need_texture) {	
#ifdef USE_TILED_TEXTURE				
		surf = MyXeCreateTexture(xe, width, height, 0, format, 1);
#else
		surf = MyXeCreateTexture(xe, width, height, 0, format, 0);
#endif
		xeTmus[xeCurrentTMU].boundtexture->teximg = surf;
	}
	
	memset(surf->base, 0xFF, surf->wpitch * surf->hpitch);
	xeTmus[xeCurrentTMU].boundtexture->internalformat = internalformat;
		
	uint8_t * surfbuf = (uint8_t*) MyXeSurfaceLockRect(xe, surf, 0, 0, 0, 0, XE_LOCK_WRITE);
	uint8_t * srcdata = (uint8_t*) pixels;
	uint8_t * dstdata = surfbuf;
	int y, x;
	
	check_format(srcbytes, dstbytes);

	for (y = 0; y <height; y++) {
		dstdata = surfbuf + ((y * width) * dstbytes);
		for (x = 0; x < width; x++) {
			if (srcbytes == 4) {
				if (dstbytes == 4) {
					dstdata[0] = srcdata[3];
					dstdata[3] = srcdata[2];
					dstdata[2] = srcdata[1];
					dstdata[1] = srcdata[0];
				} else if(dstbytes == 1) {
					dstdata[0] = ((int) srcdata[0] + (int) srcdata[1] + (int) srcdata[2] + (int) srcdata[3]) / 3;
				}
					
				srcdata += srcbytes;
				dstdata += dstbytes;
				
			} else if (srcbytes == 3) {
				if (dstbytes == 4) {		
					dstdata[0] = 0xff;
					dstdata[3] = srcdata[2];
					dstdata[2] = srcdata[1];
					dstdata[1] = srcdata[0];
				} else if (dstbytes == 1) {
					dstdata[0] = ((int) srcdata[0] + (int) srcdata[1] + (int) srcdata[2]) / 3;
				}
					
				srcdata += srcbytes;
				dstdata += dstbytes;
			} else if (srcbytes == 1) {
				if (dstbytes == 1) {
					dstdata[0] = srcdata[0];
				} else if (dstbytes == 4) {
					dstdata[0] = srcdata[0];
					dstdata[1] = srcdata[0];
					dstdata[2] = srcdata[0];
					dstdata[3] = srcdata[0];
				}
				srcdata += srcbytes;
				dstdata += dstbytes;
			}
		}
	}
	MyXeSurfaceUnlock(xe, surf);


	xeTmus[xeCurrentTMU].boundtexture->dirty = 1;
#endif	
//	handle_small_surface(surf, NULL);
}

/** Not used in QII */
void glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
	
}
/***********************************************************************
 * 
 * Multi texturing
 * 
 ***********************************************************************/

static int getTmu(GLenum texture)
{
	switch(texture) {
		case GL_TEXTURE0_SGIS:
		case GL_TEXTURE0:
			return 0;
		case GL_TEXTURE1_SGIS:
		case GL_TEXTURE1:
			return 1;
			/*			
		case GL_TEXTURE2:
			return 2;
		case GL_TEXTURE3:
			return 3;
			*/
		default:
			TR
			return 0;
	}
}

void glActiveTexture(GLenum texture)
{
	xeCurrentTMU = getTmu(texture);
}

void glMultiTexCoord1f(GLenum target, GLfloat s)
{
	int tmu = getTmu(target);
	xe_TextCoord[tmu].u = s;
	xe_TextCoord[tmu].v = 0;
}

void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
	int tmu = getTmu(target);
	xe_TextCoord[tmu].u = s;
	xe_TextCoord[tmu].v = t;
}
/***********************************************************************
 * 
 * Images parameters
 * 
 ***********************************************************************/
 
#define XE_TEXF_NONE	2
#define XE_TEXF_POINT 0
#define XE_TEXF_LINEAR 1
#define XE_TEXF_ANISOTROPIC 4

void glTexParameterfv(	GLenum  	target,
 	GLenum  	pname,
 	const GLfloat *  	params){
	TR
}


void glTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
	if (!xeTmus[xeCurrentTMU].boundtexture) return;
	if (!xeTmus[xeCurrentTMU].boundtexture->teximg ) return;
	if (target != GL_TEXTURE_2D) return;

	xeTmus[xeCurrentTMU].texparamdirty = TRUE;

	switch (pname)
	{
#if 1
	case GL_TEXTURE_MIN_FILTER:
		if ((int) param == GL_NEAREST_MIPMAP_NEAREST)
		{
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_POINT;
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_POINT;
		}
		else if ((int) param == GL_LINEAR_MIPMAP_NEAREST)
		{
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_LINEAR;
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_POINT;
		}
		else if ((int) param == GL_NEAREST_MIPMAP_LINEAR)
		{
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_POINT;
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_LINEAR;
		}
		else if ((int) param == GL_LINEAR_MIPMAP_LINEAR)
		{
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_LINEAR;
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_LINEAR;
		}
		else if ((int) param == GL_LINEAR)
		{
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_LINEAR;
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_NONE;
		}
		else
		{
			// GL_NEAREST
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_POINT;
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_NONE;
		}
		break;

	case GL_TEXTURE_MAG_FILTER:
		if ((int) param == GL_LINEAR)
			xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_LINEAR;
		else xeTmus[xeCurrentTMU].boundtexture->teximg->use_filtering = XE_TEXF_POINT;
		break;
#endif
	case GL_TEXTURE_WRAP_S:
		if ((int) param == GL_CLAMP)
			xeTmus[xeCurrentTMU].boundtexture->teximg->u_addressing = XE_TEXADDR_CLAMP;
		else xeTmus[xeCurrentTMU].boundtexture->teximg->u_addressing = XE_TEXADDR_WRAP;
		break;

	case GL_TEXTURE_WRAP_T:
		if ((int) param == GL_CLAMP)
			xeTmus[xeCurrentTMU].boundtexture->teximg->v_addressing = XE_TEXADDR_CLAMP;
		else xeTmus[xeCurrentTMU].boundtexture->teximg->v_addressing = XE_TEXADDR_WRAP;
		break;

	default:
		break;
	}
}

 
void glTexParameteri (GLenum target, GLenum pname, GLint param)
{
	glTexParameterf (target, pname, param);
}

void glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params)
{
	
}



