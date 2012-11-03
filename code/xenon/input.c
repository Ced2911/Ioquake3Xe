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

#include "../client/client.h"
#include "../sys/sys_local.h"

#include <input/input.h>
#include <usb/usbmain.h>
#include <debug.h>

#define TRESHOLD	14000
#define AXIS_MIN 	-32768         /* minimum value for axis coordinate */
#define AXIS_MAX 	32767          /* maximum value for axis coordinate */
#define MAX_AXES 	4              /* each joystick can have up to 5 axes */
#define MAX_HATS 	2


#define HOLD(x)	(ctrl.x)
#define PRESSED(x)	((ctrl.x) && (!old_ctrl.x))
#define RELEASED(x) ((!ctrl.x) && (old_ctrl.x))

enum XE_BUTTONS {
	XE_BTN_UP, XE_BTN_DOWN, XE_BTN_LEFT, XE_BTN_RIGHT,
	XE_BTN_A, XE_BTN_B, XE_BTN_X, XE_BTN_Y,
	XE_BTN_START, XE_BTN_BACK, XE_BTN_LOGO,
	XE_BTN_RB, XE_BTN_LB,
	XE_BTN_RT, XE_BTN_LT,
	XE_BTN_S1Z, XE_BTN_S2Z,
	XE_BTN_MAX
};

struct
{
	qboolean buttons[XE_BTN_MAX];  // !!! FIXME: these might be too many.
	unsigned int oldaxes;
	int oldaaxes[MAX_AXES];
	unsigned int oldhats;
} stick_state;

static cvar_t *in_joystick          = NULL;
static cvar_t *in_joystickDebug     = NULL;
static cvar_t *in_joystickThreshold = NULL;
static cvar_t *in_joystickNo        = NULL;
static cvar_t *in_joystickUseAnalog = NULL;

static struct controller_data_s ctrl, old_ctrl;

extern void Key_SetBinding( int keynum, const char *binding );

static void IN_InitJoystick( void ) {
	
	memset(&ctrl, 0, sizeof(struct controller_data_s));
	memset(&stick_state, '\0', sizeof (stick_state));
	
	in_joystick = Cvar_Get( "in_joystick", "1", CVAR_ARCHIVE|CVAR_LATCH );
	in_joystickDebug = Cvar_Get( "in_joystickDebug", "1", CVAR_TEMP );
	in_joystickThreshold = Cvar_Get( "joy_threshold", "0.35", CVAR_ARCHIVE );
	in_joystickUseAnalog = Cvar_Get( "in_joystickUseAnalog", "1", CVAR_ARCHIVE );
	
	Com_DPrintf( "Joystickopened\n");
	Com_DPrintf( "Use Analog: %s\n", in_joystickUseAnalog->integer ? "Yes" : "No" );
	
	// UP DOWN LEFT RIGHT
	Key_SetBinding(K_JOY1 + XE_BTN_UP, "+moveup");
	Key_SetBinding(K_JOY1 + XE_BTN_DOWN, "+movedown");
	Key_SetBinding(K_JOY1 + XE_BTN_LEFT, "weapprev");
	Key_SetBinding(K_JOY1 + XE_BTN_RIGHT, "weapnext");
		
	// ABXY
	Key_SetBinding(K_JOY1 + XE_BTN_A, "+moveup");
	Key_SetBinding(K_JOY1 + XE_BTN_B, "+movedown");
	Key_SetBinding(K_JOY1 + XE_BTN_X, "weapprev");
	Key_SetBinding(K_JOY1 + XE_BTN_Y, "weapnext");
	
	// LB RB
	Key_SetBinding(K_JOY1 + XE_BTN_LB, "+moveleft");
	Key_SetBinding(K_JOY1 + XE_BTN_RB, "+moveright");	
	
	// LT RT
	Key_SetBinding(K_JOY1 + XE_BTN_LT, "+zoom");
	Key_SetBinding(K_JOY1 + XE_BTN_RT, "+attack");
	
	// START BACK
	Key_SetBinding(K_JOY1 + XE_BTN_START, "+button2");
	Key_SetBinding(K_JOY1 + XE_BTN_BACK, "togglemenu");
	Key_SetBinding(K_JOY1 + XE_BTN_LOGO, "+scores");
}

static int JoyPressed(int btn) {
	switch(btn) {
		case XE_BTN_UP: {
			return PRESSED(up);
		}
		case XE_BTN_DOWN: {
			return PRESSED(down);
		}
		case XE_BTN_LEFT: {
			return PRESSED(left);
		}
		case XE_BTN_RIGHT: {
			return PRESSED(right);
		}		
		
		case XE_BTN_A: {
			return PRESSED(a);
		}
		case XE_BTN_B: {
			return PRESSED(b);
		}
		case XE_BTN_X: {
			return PRESSED(x);
		}
		case XE_BTN_Y: {
			return PRESSED(y);
		}
		
		case XE_BTN_START: {
			return PRESSED(start);
		}
		case XE_BTN_BACK: {
			return PRESSED(back);
		}
		case XE_BTN_LOGO: {
			return PRESSED(logo);
		}
		
		case XE_BTN_RB: {
			return PRESSED(rb);
		}
		case XE_BTN_LB: {
			return PRESSED(lb);
		}
		case XE_BTN_LT: {
			return ctrl.lt>200;
		}
		case XE_BTN_RT: {
			return ctrl.rt>200;
		}
		default:
		return 0;
	}
}

static int JoyReleased(int btn) {
	switch(btn) {
		case XE_BTN_UP: {
			return RELEASED(up);
		}
		case XE_BTN_DOWN: {
			return RELEASED(down);
		}
		case XE_BTN_LEFT: {
			return RELEASED(left);
		}
		case XE_BTN_RIGHT: {
			return RELEASED(right);
		}
		
		case XE_BTN_A: {
			return RELEASED(a);
		}
		case XE_BTN_B: {
			return RELEASED(b);
		}
		case XE_BTN_X: {
			return RELEASED(x);
		}
		case XE_BTN_Y: {
			return RELEASED(y);
		}
		
		case XE_BTN_START: {
			return RELEASED(start);
		}
		case XE_BTN_BACK: {
			return RELEASED(back);
		}		
		case XE_BTN_LOGO: {
			return RELEASED(logo);
		}
		
		case XE_BTN_RB: {
			return RELEASED(rb);
		}
		case XE_BTN_LB: {
			return RELEASED(lb);
		}
		case XE_BTN_LT: {
			return ctrl.lt<200;
		}
		case XE_BTN_RT: {
			return ctrl.rt<200;
		}
		default:
		return 0;
	}
}

static void JoySetAxisValue(int axis, signed short value) {	
	float f = ( (float) abs(value) ) / 32767.0f;	
	if( f < in_joystickThreshold->value ) value = 0;	
	Com_QueueEvent( 0, SE_JOYSTICK_AXIS, axis, value, 0, NULL );	
}

static void IN_JoyMove( void ) {
	int i;
	unsigned int hats = 0;
	unsigned int axes = 0;
	
	// query the stick buttons...
	
	for (i = 0; i < XE_BTN_MAX; i++)
	{
		if (JoyPressed(i)) {			
			Com_QueueEvent( 0, SE_KEY, K_JOY1 + i, qtrue, 0, NULL );
		} else if(JoyReleased(i)) {
			Com_QueueEvent( 0, SE_KEY, K_JOY1 + i, qfalse, 0, NULL );
		}
	}
			
	// finally, look at the axes...
	JoySetAxisValue(0, ctrl.s1_x);
	JoySetAxisValue(1, -ctrl.s1_y);
	JoySetAxisValue(4, ctrl.s2_x);
	JoySetAxisValue(3, -ctrl.s2_y);
}

void IN_Init( void ) {
	IN_InitJoystick();
}

void IN_Frame (void) {
	usb_do_poll();
	
	// get ctrl data
	get_controller_data(&ctrl, 0);
	
	IN_JoyMove( );
	
	// save for next loop
	old_ctrl = ctrl;
}

void IN_Shutdown( void ) {
}

void IN_Restart( void ) {
}

