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
#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "../sys/sys_local.h"

#include <GL/gl_xenos.h>

qboolean ( * qwglSwapIntervalEXT)( int interval );
void ( * qglMultiTexCoord2fARB )( GLenum texture, float s, float t );
void ( * qglActiveTextureARB )( GLenum texture );
void ( * qglClientActiveTextureARB )( GLenum texture );

void ( * qglLockArraysEXT)( int, int);
void ( * qglUnlockArraysEXT) ( void );

void		GLimp_EndFrame( void ) {	
	// don't flip if drawing to front buffer
	if ( Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	{		
		XenonGLDisplay();
	}
}

void 		GLimp_Init( void )
{
	static int gl_initilised = 0;
	
	if (gl_initilised == 0) {
		XenonGLInit();
	}
	gl_initilised = 1;
		
	glConfig.textureCompression = TC_NONE;
	glConfig.textureEnvAddAvailable = qfalse;
	
	 // This values force the UI to disable driver selection
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;
	glConfig.deviceSupportsGamma = 0;

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, (char *) qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, (char *) qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
	if (*glConfig.renderer_string && glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] == '\n')
			glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] = 0;
	Q_strncpyz( glConfig.version_string, (char *) qglGetString (GL_VERSION), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, (char *) qglGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );
		
	XeGetScreenSize(&glConfig.vidWidth, &glConfig.vidHeight);
	/*
	glConfig.vidWidth = 640;
	glConfig.vidHeight = 480;
	*/
	glConfig.isFullscreen = qtrue;
	/*
	glConfig.colorBits = 24;
	glConfig.depthBits = 8;
	glConfig.stencilBits = 8;
	*/
	IN_Init( );
}

void		GLimp_Shutdown( void ) {
}

void		GLimp_EnableLogging( qboolean enable ) {
}

void GLimp_LogComment( char *comment ) {
	// printf("[GL Log] %s\n", comment);
}

qboolean QGL_Init( const char *dllname ) {
	
	qwglSwapIntervalEXT = NULL;
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	qglLockArraysEXT = NULL;
	qglUnlockArraysEXT = NULL;
	
	return qtrue;
}

void		QGL_Shutdown( void ) {
}

// No SMP - stubs
void GLimp_RenderThreadWrapper( void *arg )
{
}

qboolean GLimp_SpawnRenderThread( void (*function)( void ) )
{
	ri.Printf( PRINT_WARNING, "ERROR: SMP support was disabled at compile time\n");
	return qfalse;
}

void *GLimp_RendererSleep( void )
{
	return NULL;
}

void GLimp_FrontEndSleep( void )
{
}

void GLimp_WakeRenderer( void *data )
{
}


void GLimp_Minimize(void)
{
	
}
