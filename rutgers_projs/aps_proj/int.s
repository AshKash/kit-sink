	.file	"int.c"
	.arch avr2
__SREG__ = 0x3f
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__tmp_reg__ = 0
__zero_reg__ = 1
_PC_ = 2
gcc2_compiled.:
	.text
.global	main
	.type	main,@function
main:
/* prologue: frame size=12 */
	ldi r28,lo8(__stack - 12)
	ldi r29,hi8(__stack - 12)
	out __SP_H__,r29
	out __SP_L__,r28
/* prologue end (size=4) */
	ldd r18,Y+5
	ldd r19,Y+6
	ldd r20,Y+7
	ldd r21,Y+8
	ldd r22,Y+1
	ldd r23,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	rcall __addsf3
	mov r27,r25
	mov r26,r24
	mov r25,r23
	mov r24,r22
	std Y+9,r24
	std Y+10,r25
	std Y+11,r26
	std Y+12,r27
	ldi r24,lo8(0x3f9c28f6)
	ldi r25,hi8(0x3f9c28f6)
	ldi r26,hlo8(0x3f9c28f6)
	ldi r27,hhi8(0x3f9c28f6)
	std Y+1,r24
	std Y+2,r25
	std Y+3,r26
	std Y+4,r27
	ldi r24,lo8(0x4059999a)
	ldi r25,hi8(0x4059999a)
	ldi r26,hlo8(0x4059999a)
	ldi r27,hhi8(0x4059999a)
	std Y+5,r24
	std Y+6,r25
	std Y+7,r26
	std Y+8,r27
	ldd r18,Y+5
	ldd r19,Y+6
	ldd r20,Y+7
	ldd r21,Y+8
	ldd r22,Y+1
	ldd r23,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	rcall __mulsf3
	mov r27,r25
	mov r26,r24
	mov r25,r23
	mov r24,r22
	std Y+9,r24
	std Y+10,r25
	std Y+11,r26
	std Y+12,r27
	ldi r24,lo8(0x4091999a)
	ldi r25,hi8(0x4091999a)
	ldi r26,hlo8(0x4091999a)
	ldi r27,hhi8(0x4091999a)
	std Y+1,r24
	std Y+2,r25
	std Y+3,r26
	std Y+4,r27
	ldi r24,lo8(0x43a1451f)
	ldi r25,hi8(0x43a1451f)
	ldi r26,hlo8(0x43a1451f)
	ldi r27,hhi8(0x43a1451f)
	std Y+5,r24
	std Y+6,r25
	std Y+7,r26
	std Y+8,r27
	ldd r18,Y+5
	ldd r19,Y+6
	ldd r20,Y+7
	ldd r21,Y+8
	ldd r22,Y+1
	ldd r23,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	rcall __divsf3
	mov r27,r25
	mov r26,r24
	mov r25,r23
	mov r24,r22
	std Y+9,r24
	std Y+10,r25
	std Y+11,r26
	std Y+12,r27
	ldi r24,lo8(0x3fa8f5c3)
	ldi r25,hi8(0x3fa8f5c3)
	ldi r26,hlo8(0x3fa8f5c3)
	ldi r27,hhi8(0x3fa8f5c3)
	std Y+1,r24
	std Y+2,r25
	std Y+3,r26
	std Y+4,r27
	ldi r24,lo8(0x41070a3d)
	ldi r25,hi8(0x41070a3d)
	ldi r26,hlo8(0x41070a3d)
	ldi r27,hhi8(0x41070a3d)
	std Y+5,r24
	std Y+6,r25
	std Y+7,r26
	std Y+8,r27
	ldd r18,Y+1
	ldd r19,Y+2
	ldd r20,Y+3
	ldd r21,Y+4
	ldd r22,Y+1
	ldd r23,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	rcall __mulsf3
	mov r14,r22
	mov r15,r23
	mov r16,r24
	mov r17,r25
	ldd r18,Y+5
	ldd r19,Y+6
	ldd r20,Y+7
	ldd r21,Y+8
	ldd r22,Y+5
	ldd r23,Y+6
	ldd r24,Y+7
	ldd r25,Y+8
	rcall __mulsf3
	mov r27,r25
	mov r26,r24
	mov r25,r23
	mov r24,r22
	mov r18,r24
	mov r19,r25
	mov r20,r26
	mov r21,r27
	mov r25,r17
	mov r24,r16
	mov r23,r15
	mov r22,r14
	rcall __addsf3
	mov r27,r25
	mov r26,r24
	mov r25,r23
	mov r24,r22
	mov r22,r24
	mov r23,r25
	mov r24,r26
	mov r25,r27
	rcall sqrt
	mov r27,r25
	mov r26,r24
	mov r25,r23
	mov r24,r22
	std Y+9,r24
	std Y+10,r25
	std Y+11,r26
	std Y+12,r27
/* epilogue: frame size=12 */
__stop_progIi__:
	rjmp __stop_progIi__
/* epilogue end (size=1) */
/* function main size 156 (151) */
.Lfe1:
	.size	main,.Lfe1-main
/* File int.c: code  156 = 0x009c ( 151), prologues   4, epilogues   1 */
