|
| Simple AmigaOS Hello World
|
| Public Domain
|

	.text 0

	movea.l	4.w,a6		| AbsExecBase
	lea	dosname,a1	| Library name
	moveq	#0,d0		| Any version
	jsr	-552(a6)	| _LVOOpenLibrary
	tst.l	d0		| Result in D0
	beq.s	quit		| Quit if it failed
	movea.l	d0,a6		| DOS base in A6
	jsr	-60(a6)		| _LVOOutput
	move.l	d0,d1		| File handle in D1
	move.l	#txt,d2		| Data ptr in D2
	move.l	#13,d3		| Length in D3
	jsr	-48(a6)		| _LVOWrite
	movea.l	a6,a1		| DOS base to A1
	movea.l	4.w,a6		| AbsExecBase
	jsr	-414(a6)	| _LVOCloseLibrary
	moveq	#0,d0		| Exit code
quit:	rts

	.data 1

dosname:
	.asciz "dos.library"

txt:	.asciz "Hello World!\n"
