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

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../sys/sys_local.h"

#include <ppc/timebase.h>
#include <time/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <debug.h>

#include <libfat/fat.h>
#include <console/console.h>
#include <diskio/ata.h>
#include <usb/usbmain.h>
#include <xenos/xenos.h>
#include <xenon_soc/xenon_power.h>
#include <libxtaf/xtaf.h>

qboolean stdinIsATTY;

void http_output_start();

char * dirname(const char *path)
{
	static char dname[MAXPATHLEN];
	size_t len;
	const char *endp;

	/* Empty or NULL string gets treated as "." */
	if (path == NULL || *path == '\0') {
		dname[0] = '.';
		dname[1] = '\0';
		return (dname);
	}

	/* Strip any trailing slashes */
	endp = path + strlen(path) - 1;
	while (endp > path && *endp == '/')
		endp--;

	/* Find the start of the dir */
	while (endp > path && *endp != '/')
		endp--;

	/* Either the dir is "/" or there are no slashes */
	if (endp == path) {
		dname[0] = *endp == '/' ? '/' : '.';
		dname[1] = '\0';
		return (dname);
	} else {
		/* Move forward past the separating slashes */
		do {
			endp--;
		} while (endp > path && *endp == '/');
	}

	len = endp - path + 1;
	if (len >= sizeof(dname)) {
		errno = ENAMETOOLONG;
		return (NULL);
	}
	memcpy(dname, path, len);
	dname[len] = '\0';
	return (dname);
}

// Used to determine where to store user-specific files
static char homePath[ MAX_OSPATH ] = { 0 };

/*
==================
Sys_DefaultHomePath
==================
*/
char *Sys_DefaultHomePath(void)
{
	char *p;

	if( !*homePath && com_homepath != NULL )
	{
		if( ( p = getenv( "HOME" ) ) != NULL )
		{
			Com_sprintf(homePath, sizeof(homePath), "%s%c", p, PATH_SEP);
			if(com_homepath->string[0])
				Q_strcat(homePath, sizeof(homePath), com_homepath->string);
			else
				Q_strcat(homePath, sizeof(homePath), HOMEPATH_NAME_UNIX);
		}
	}

	return homePath;
}

/*
================
Sys_Milliseconds
================
*/
/* base time in seconds, that's our origin
   timeval:tv_sec is an int:
   assuming this wraps every 0x7fffffff - ~68 years since the Epoch (1970) - we're safe till 2038 */
unsigned long sys_timeBase = 0;
/* current time in ms, using sys_timeBase as origin
   NOTE: sys_timeBase*1000 + curtime -> ms since the Epoch
     0x7fffffff ms - ~24 days
   although timeval:tv_usec is an int, I'm not sure wether it is actually used as an unsigned int
     (which would affect the wrap period) */
int curtime;
int Sys_Milliseconds (void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);

	if (!sys_timeBase)
	{
		sys_timeBase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - sys_timeBase)*1000 + tp.tv_usec/1000;

	return curtime;
}

/*
==================
Sys_RandomBytes
==================
*/
qboolean Sys_RandomBytes( byte *string, int len )
{
	int i =0;
	for(i = 0; i <len; i++) {
		string[i] = rand() % 255;
	}
	return qtrue;
}

/*
==================
Sys_GetCurrentUser
==================
*/
char *Sys_GetCurrentUser( void )
{
	return "player";
}

/*
==================
Sys_GetClipboardData
==================
*/
char *Sys_GetClipboardData(void)
{
	return NULL;
}

#define MEM_THRESHOLD 96*1024*1024

const char *basename(const char *path)
{
    char *s;
    s = strrchr(path, '/');
    return s ? s + 1 : path;
}


/*
==================
Sys_LowPhysicalMemory

TODO
==================
*/
qboolean Sys_LowPhysicalMemory( void )
{
	return qfalse;
}

/*
==================
Sys_Basename
==================
*/
const char *Sys_Basename( char *path )
{
	return basename( path );
}

/*
==================
Sys_Dirname
==================
*/
const char *Sys_Dirname( char *path )
{
	return dirname( path );
}

/*
==================
Sys_Mkdir
==================
*/
qboolean Sys_Mkdir( const char *path )
{
	int result = mkdir( path, 0750 );

	if( result != 0 )
		return errno == EEXIST;

	return qtrue;
}

/*
==================
Sys_Mkfifo
==================
*/
FILE *Sys_Mkfifo( const char *ospath )
{
	return NULL;
}

/*
==================
Sys_Cwd
==================
*/
char *Sys_Cwd( void )
{
	static char cwd[MAX_OSPATH];

	char *result = getcwd( cwd, sizeof( cwd ) - 1 );
	if( result != cwd )
		return NULL;

	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define MAX_FOUND_FILES 0x1000

/*
==================
Sys_ListFilteredFiles
==================
*/
void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **list, int *numfiles )
{
	char          search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char          filename[MAX_OSPATH];
	DIR           *fdir;
	struct dirent *d;
	struct stat   st;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if (strlen(subdirs)) {
		Com_sprintf( search, sizeof(search), "%s/%s", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof(search), "%s", basedir );
	}

	if ((fdir = opendir(search)) == NULL) {
		return;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
		if (stat(filename, &st) == -1)
			continue;

		if (st.st_mode & S_IFDIR) {
			if (Q_stricmp(d->d_name, ".") && Q_stricmp(d->d_name, "..")) {
				if (strlen(subdirs)) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
				}
				else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s/%s", subdirs, d->d_name );
		if (!Com_FilterPath( filter, filename, qfalse ))
			continue;
		list[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	}

	closedir(fdir);
}

/*
==================
Sys_ListFiles
==================
*/
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs )
{
	struct dirent *d;
	DIR           *fdir;
	qboolean      dironly = wantsubs;
	char          search[MAX_OSPATH];
	int           nfiles;
	char          **listCopy;
	char          *list[MAX_FOUND_FILES];
	int           i;
	struct stat   st;

	int           extLen;

	if (filter) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = NULL;
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension)
		extension = "";

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		dironly = qtrue;
	}

	extLen = strlen( extension );

	// search
	nfiles = 0;

	if ((fdir = opendir(directory)) == NULL) {
		*numfiles = 0;
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		if (*extension) {
			if ( strlen( d->d_name ) < extLen ||
				Q_stricmp(
					d->d_name + strlen( d->d_name ) - extLen,
					extension ) ) {
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 )
			break;
		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = NULL;

	closedir(fdir);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

/*
==================
Sys_FreeFileList
==================
*/
void Sys_FreeFileList( char **list )
{
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

/*
==================
Sys_Sleep

Block execution for msec or until input is recieved.
==================
*/
void Sys_Sleep( int msec )
{
	if( msec == 0 )
		return;

	// With nothing to select() on, we can't wait indefinitely
	if( msec < 0 )
		msec = 10;

	udelay( msec * 1000 );
}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog( const char *error )
{
	printf("Sys_ErrorDialog : %s\n",error);
}


/*
==============
Sys_Dialog

Display a *nix dialog box
==============
*/
dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title )
{
	printf("Sys_Dialog : %s : %s\n", message, title);
	Com_DPrintf( S_COLOR_YELLOW "WARNING: failed to show a dialog\n" );
	return DR_OK;
}


/*
==============
Sys_GLimpSafeInit

Unix specific "safe" GL implementation initialisation
==============
*/
void Sys_GLimpSafeInit( void )
{
	// NOP
}

/*
==============
Sys_GLimpInit

Unix specific GL implementation initialisation
==============
*/
void Sys_GLimpInit( void )
{
	// NOP
}

void Sys_SetFloatEnv(void)
{

}

/*
==============
Sys_PlatformInit

Unix specific initialisation
==============
*/

static void ListDevices()
{
	int i;
	for (i = 3; i < STD_MAX; i++) {
		if (devoptab_list[i]->structSize) {
			printf("findDevices : %s\r\n", devoptab_list[i]->name);
		}
	}
}

extern void threading_init();
extern void network_init_sys();
extern DISC_INTERFACE usb2mass_ops_0;

void Sys_PlatformInit( void )
{
	// Init libxenon
	xenos_init(VIDEO_MODE_AUTO);

	xenon_make_it_faster(XENON_SPEED_FULL);
	
	http_output_start();
	
#if 0
	threading_init();
	network_init_sys();
#endif	
	
	usb_init();
	usb_do_poll();

	xenon_ata_init();
	xenon_atapi_init();

	// fatInitDefault();
	// XTAFMount();	
	
	// fatInit(16, 1);
	
	fatMount ("uda", &usb2mass_ops_0, 0, 256, 64);
	
	ListDevices();
	
	Sys_SetEnv("HOME", "uda:/");
}

/*
==============
Sys_PlatformExit

Unix specific deinitialisation
==============
*/
void Sys_PlatformExit( void )
{
}

/*
==============
Sys_SetEnv

set/unset environment variables (empty value removes it)
==============
*/

void Sys_SetEnv(const char *name, const char *value)
{
	if(value && *value)
		setenv(name, value, 1);
	else
		unsetenv(name);
}

/*
==============
Sys_PID
==============
*/
int Sys_PID( void )
{
	printf("Sys_PID : %d\n", 1);
	return 1;
}

/*
==============
Sys_PIDIsRunning
==============
*/
qboolean Sys_PIDIsRunning( int pid )
{
	printf("Sys_PIDIsRunning : %d\n", pid);
	return 1;
}



/*
==================
CON_Shutdown
==================
*/
void CON_Shutdown( void )
{
	console_close();
}

/*
==================
CON_Init
==================
*/
void CON_Init( void )
{
	console_init();
}

/*
==================
CON_Input
==================
*/
char *CON_Input( void )
{
	return NULL;
}

/*
==================
CON_Print
==================
*/
void CON_Print( const char *msg )
{
	fputs( msg, stderr );
}

/*
==================
Xenon_LoadLibrary
==================
*/
void *Xenon_LoadLibrary (const char *filename) { TR; return NULL; }; 

/*
==================
Xenon_LibraryError
==================
*/
const char *Xenon_LibraryError(void) {TR; return NULL;}; 

/*
==================
Xenon_LoadFunction
==================
*/
void *Xenon_LoadFunction(void *handle, char *symbol) {TR; return NULL;};

/*
==================
Xenon_UnloadLibrary
==================
*/ 
int Xenon_UnloadLibrary (void *handle) {TR;};
