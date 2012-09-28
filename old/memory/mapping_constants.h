/*
memory/mapping_constants.h - Memory mapping constants

Copyright (C) 2010  Magnus Öberg

The license for this program is dual license:
  AROS Public License, version 1.1 or later (APL1.1+)
AND
  GNU Lesser General Public License, version 2.1 or later (LGPL2.1+)

Dual licensing means for this program that anyone that wants to use,
modify or distribute all or parts of this program can choose the best
suiting license of APL1.1+ or LGPL2.1+ and must follow the terms described
in that license. Choosing only one license disables the other license and
references to the disabled license in code and documentation may be removed.
This text paragraph should be removed at the same time. It is also permitted
to keep this exact dual licensing. The copyrights are not affected by
selecting only one license and remain in full.

-- APL conditions --

  The contents of this file are subject to the AROS Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at

  http://www.aros.org/license.html

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.

-- LGPL conditions --

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301  USA

--
*/

#ifndef _MEMORY_MAPPING_CONSTANTS_H_
#define _MEMORY_MAPPING_CONSTANTS_H_

// MMU Entry types/prios

// Priority levels, system objects
#define MMU_PRIO_0	0x00000000
#define MMU_PRIO_P1	0x01000000
// The highest system object prio
#define MMU_PRIO_SYSTEM	MMU_PRIO_P1

// Priority levels, ordinary memory
#define MMU_PRIO_MEM	0x02000000
#define MMU_PRIO_ROOT	0x03000000

#define MMU_PRIO_LEVELS 4
#define MMU_PRIO_GET_LEVEL(x) ((x)>>24)

// Basic memory types
#define MMU_TYPE_UNINITIALIZED			( 0	| MMU_PRIO_ROOT)

#define MMU_TYPE_VALLOCMEM			( 10	| MMU_PRIO_MEM)
#define MMU_TYPE_VALLOCMEM_ROM			( 11	| MMU_PRIO_MEM)
#define MMU_TYPE_VALLOCMEM_RAM			( 12	| MMU_PRIO_MEM)

// System objects

// exec.library
#define MMU_TYPE_EXEC_EXECBASE			( 100	| MMU_PRIO_0 )
#define MMU_TYPE_EXEC_VECTORS			( 101	| MMU_PRIO_0 )
#define MMU_TYPE_EXEC_ALLOCMEM			( 110	| MMU_PRIO_MEM )
#define MMU_TYPE_EXEC_S_MSGPORT			( 120	| MMU_PRIO_0 )
#define MMU_TYPE_EXEC_S_MESSAGE			( 121	| MMU_PRIO_0 )
#define MMU_TYPE_EXEC_S_IOREQUEST		( 122	| MMU_PRIO_0 )

/* dos.library */
#define MMU_TYPE_DOS_DOSBASE			( 200	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_VECTORS			( 201	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_COMMANDLINEINTERFACE	( 220	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_PROCESS			( 221	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_FILEHANDLE		( 222	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_FILELOCK			( 223	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_RDARGS			( 224	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_ANCHORPATH		( 225	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_ACHAIN			( 226	| MMU_PRIO_0 )
#define MMU_TYPE_DOS_S_DATESTAMP		( 227	| MMU_PRIO_0 )

/* intuition.library */
#define MMU_TYPE_INTUITION_INTUITIONBASE	( 300	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_VECTORS		( 301	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_S_SCREEN		( 320	| MMU_PRIO_P1 )
#define MMU_TYPE_INTUITION_S_DRAWINFO		( 321	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_S_WINDOW		( 322	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_S_MENU		( 323	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_S_MENUITEM		( 324	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_S_INTUIMESSAGE	( 325	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_S_GADGET		( 326	| MMU_PRIO_0 )
#define MMU_TYPE_INTUITION_S_REQUESTER		( 327	| MMU_PRIO_0 )

/* graphics.library */
#define MMU_TYPE_GRAPHICS_GRAPHICSBASE		( 400	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_VECTORS		( 401	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_TEXTFONT		( 420	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_TEXTATTR		( 421	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_VIEWPORT		( 422	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_DISPLAYINFOHANDLE	( 423	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_DISPLAYINFO		( 424	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_RASTPORT		( 425	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_TEXTEXTENT		( 426	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_LAYER		( 427	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_BITMAP		( 428	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_TMPRAS		( 429	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_AREAINFO		( 430	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_GELSINFO		( 431	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_LAYER_INFO		( 432	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_REGION		( 433	| MMU_PRIO_0 )
#define MMU_TYPE_GRAPHICS_S_RECTANGLE		( 434	| MMU_PRIO_0 )

/* utility.library */
#define MMU_TYPE_UTILITY_UTILITYBASE		( 500	| MMU_PRIO_0 )
#define MMU_TYPE_UTILITY_VECTORS		( 501	| MMU_PRIO_0 )
#define MMU_TYPE_UTILITY_S_HOOK			( 520	| MMU_PRIO_0 )

/* gadtools.library */
#define MMU_TYPE_GADTOOLS_GADTOOLSBASE		( 600	| MMU_PRIO_0 )
#define MMU_TYPE_GADTOOLS_VECTORS		( 601	| MMU_PRIO_0 )
#define MMU_TYPE_GADTOOLS_S_VISUALINFO		( 620	| MMU_PRIO_0 )
#define MMU_TYPE_GADTOOLS_S_NEWMENU		( 621	| MMU_PRIO_0 )
#define MMU_TYPE_GADTOOLS_S_NEWGADGET		( 622	| MMU_PRIO_0 )

/* icon.library */
#define MMU_TYPE_ICON_ICONBASE			( 700	| MMU_PRIO_0 )
#define MMU_TYPE_ICON_VECTORS			( 701	| MMU_PRIO_0 )
#define MMU_TYPE_ICON_S_DISKOBJECT		( 720	| MMU_PRIO_0 )

/* layers.library */
#define MMU_TYPE_LAYERS_LAYERSBASE		( 800	| MMU_PRIO_0 )
#define MMU_TYPE_LAYERS_VECTORS			( 801	| MMU_PRIO_0 )

/* locale.library */
#define MMU_TYPE_LOCALE_LOCALEBASE		( 900	| MMU_PRIO_0 )
#define MMU_TYPE_LOCALE_VECTORS			( 901	| MMU_PRIO_0 )
#define MMU_TYPE_LOCALE_S_LOCALE		( 920	| MMU_PRIO_0 )
#define MMU_TYPE_LOCALE_S_CATALOG		( 921	| MMU_PRIO_0 )

/* timer.device */
#define MMU_TYPE_TIMER_TIMERBASE		( 1000	| MMU_PRIO_0 )
#define MMU_TYPE_TIMER_VECTORS			( 1001	| MMU_PRIO_0 )
#define MMU_TYPE_TIMER_S_TIMEREQUEST		( 1020	| MMU_PRIO_0 )

/* audio.device */
#define MMU_TYPE_AUDIO_AUDIOBASE		( 1100	| MMU_PRIO_0 )
#define MMU_TYPE_AUDIO_VECTORS			( 1101	| MMU_PRIO_0 )
#define MMU_TYPE_AUDIO_S_IOAUDIO		( 1120	| MMU_PRIO_0 )


#endif
