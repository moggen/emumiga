/*
bootstrap.s - Start up code for emumiga

Copyright (C) 2010-2011  Magnus Öberg

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
	.text 1

	movem.l	d0/a0,-(sp)	| Save argument information
	movea.l	4.w,a6		| AbsExecBase
	lea	dosname(pc),a1	| Library name
	moveq	#0,d0		| Any version
	jsr	-552(a6)	| _LVOOpenLibrary
	tst.l	d0		| Result in D0
	bne.s	ok1		| Skip if ok
	.word	0xFD80,1,1	| Emulator hook, load error
ok1:	movea.l	d0,a6
	movea.l	a5,a0		| Exe file name from A5
	move.l	a0,d1		| Put in d1
	jsr	-150(a6)	| _LVOLoadSeg
	move.l	d0,d1		| Save in d1
	bne.s	ok2		| Skip if ok
	.word	0xFD80,1,2	| Emulator hook, load error
ok2:	lsl.l	#2,d0		| BCPL ptr -> APTR
	addq.l	#4,d0		| Skip segment entry header = 4 bytes
	movea.l	d0,a1		| Beginning of code in A1
	movem.l (sp)+,d0/a0	| Restore argument information
	movem.l	d1/a6,-(sp)	| Save registers
	jsr	(a1)		| Call code
	movem.l	(sp)+,d1/a6	| Restore registers, d1 is segment handle
	move.l	d0,-(sp)	| Save return code
	jsr	-156(a6)	| _LVOUnLoadSeg
	movea.l	a6,a1		| dos library ptr to a1
	movea.l	4.w,a6		| AbsExecBase
	jsr	-414(a6)	| _LVOCloseLibrary
	move.l	(sp)+,d0	| Restore return code
	.word	0xFD80,1,0	| Emulator hook, end process
|	rts			| Ends this process

	.balign	4
dosname:
	.byte	'd','o','s','.','l','i','b','r','a','r','y',0

	.balign	4
