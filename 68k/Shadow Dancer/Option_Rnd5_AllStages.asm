	CPU 68000
	padding off	; we don't want AS padding out dc.b instructions
	PAGE 0

	org	$00BB84

	btst	#7, d7
	bne.w	$00BBF2
	
	move.b	$B(a5), d0
	cmp.b	#4, d0
	seq	d0
	and.b	#6-3, d0
	addq.b	#3, d0	; Stage 5 -> D0 = 3; Stage 1-4 -> D0 = 3
	move.b	$C(a5), d1
	
	btst	#2, D7	; left
	bne.s	opt_stg_left
	btst	#3, D7	; right
	bne.s	opt_stg_right
	rts
	
opt_stg_right:
	addq.b	#1, d1
	sub.b	d0, d1
	bra.s	opt_stg_common
opt_stg_left:
	subq.b	#1, d1
opt_stg_common:
	bpl.s	opt_stg_fin
	add.b	d0, d1	; D1 < 0 (underflow) -> add stage limit
opt_stg_fin:
	move.b	d1, $C(a5)
	bra.w	$00BAB4

    while * < $00BBCE
	dc.w	$AAAA
    endm

	; fix round selection to prevent invalid stages
	org	$00BB50
	rts
	nop

	org	$00BC2E
    while * < $00BC38
	nop	; dummy out code used to reset Stage when Round 5 is selected
    endm

	org	$00BB62
	bra.s	$00BBCE

	org	$00BB6A
	bra.s	$00BBCE

	org	$00BB7C
	bra.s	$00BBCE

	org	$00BB82
	bra.s	$00BBCE
