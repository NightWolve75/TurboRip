/* inffas86.c is a hand tuned assembler version of
 *
 * inffast.c -- fast decoding
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * Copyright (C) 2003 Chris Anderson <christop@charm.net>
 * Please use the copyright conditions above.
 *
 * Mar-13-2003 -- Most of this is derived from inffast.S which is derived from
 * the gcc -S output of zlib-1.2.0/inffast.c.  Zlib-1.2.0 is in beta release at
 * the moment.  I have successfully compiled and tested this code with gcc2.96,
 * gcc3.2, icc5.0, msvc6.0.  It is very close to the speed of inffast.S
 * compiled with gcc -DNO_MMX, but inffast.S is still faster on the P3 with MMX
 * enabled.  I will attempt to merge the MMX code into this version.  Newer
 * versions of this and inffast.S can be found at
 * http://www.eetbeetee.com/zlib/ and http://www.charm.net/~christop/zlib/
 */
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"

#pragma warning( disable : 4731 )

/* Mark Adler's comments from inffast.c: */

/*
   Decode literal, length, and distance codes and write out the resulting
   literal and match bytes until either not enough input or output is
   available, an end-of-block is encountered, or a data error is encountered.
   When large enough input and output buffers are supplied to inflate(), for
   example, a 16K input buffer and a 64K output buffer, more than 95% of the
   inflate execution time is spent in this routine.

   Entry assumptions:

        state->mode == LEN
        strm->avail_in >= 6
        strm->avail_out >= 258
        start >= strm->avail_out
        state->bits < 8

   On return, state->mode is one of:

        LEN -- ran out of enough output space or enough available input
        TYPE -- reached end of block code, inflate() to interpret next block
        BAD -- error in block data

   Notes:

    - The maximum input bits used by a length/distance pair is 15 bits for the
      length code, 5 bits for the length extra, 15 bits for the distance code,
      and 13 bits for the distance extra.  This totals 48 bits, or six bytes.
      Therefore if strm->avail_in >= 6, then there is enough input to avoid
      checking for available input while decoding.

    - The maximum bytes that a single length/distance pair can output is 258
      bytes, which is the maximum length that can be coded.  inflate_fast()
      requires strm->avail_out >= 258 for each loop to avoid checking for
      output space.
 */
void inflate_fast(
	z_streamp strm,
	unsigned start         /* inflate()'s starting value for strm->avail_out */
)
{
    struct inflate_state FAR *state;
    struct inffast_ar {
      void *esp;                  /* esp save */
      unsigned char FAR *in;      /* local strm->next_in */
      unsigned char FAR *last;    /* while in < last, enough input available */
      unsigned char FAR *out;     /* local strm->next_out */
      unsigned char FAR *beg;     /* inflate()'s initial strm->next_out */
      unsigned char FAR *end;     /* while out < end, enough space available */
      unsigned wsize;             /* window size or zero if not using window */
      unsigned write;             /* window write index */
      unsigned char FAR *window;  /* allocated sliding window, if wsize != 0 */
      unsigned long hold;         /* local strm->hold */
      unsigned bits;              /* local strm->bits */
      code const FAR *lcode;      /* local strm->lencode */
      code const FAR *dcode;      /* local strm->distcode */
      unsigned lmask;             /* mask for first level of length codes */
      unsigned dmask;             /* mask for first level of distance codes */
      unsigned len;               /* match length, unused bytes */
      unsigned dist;              /* match distance */
      unsigned status;            /* this is set when state changes */
    } ar;

    /* copy state to local variables */
    state = (struct inflate_state FAR *)strm->state;
    ar.in = strm->next_in;
    ar.last = ar.in + (strm->avail_in - 5);
    ar.out = strm->next_out;
    ar.beg = ar.out - (start - strm->avail_out);
    ar.end = ar.out + (strm->avail_out - 257);
    ar.wsize = state->wsize;
    ar.write = state->write;
    ar.window = state->window;
    ar.hold = state->hold;
    ar.bits = state->bits;
    ar.lcode = state->lencode;
    ar.dcode = state->distcode;
    ar.lmask = (1U << state->lenbits) - 1;
    ar.dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */

	/* align in on 2 byte boundary */
	if (( (ULONG_PTR)ar.in & 0x1 ) != 0) {
		ar.hold += (unsigned long)*ar.in++ << ar.bits;
		ar.bits += 8;
	}
	__asm {
	lea	eax, ar
	pushfd
	push	ebp
	mov	[eax], esp
	mov	esp, eax
	mov	esi, [esp+4]       /* esi = in */
	mov	edi, [esp+12]      /* edi = out */
	mov	edx, [esp+36]      /* edx = hold */
	mov	ebx, [esp+40]      /* ebx = bits */
	mov	ebp, [esp+44]      /* ebp = lcode */

	cld
	jmp	L_do_loop

L_while_test:
	cmp	[esp+20], edi
	jbe	L_break_loop
	cmp	[esp+8], esi
	jbe	L_break_loop

L_do_loop:
	cmp	bl, 15
	ja	L_get_length_code    /* if (15 < bits) */

	xor	eax, eax
	lodsw                         /* al = *(ushort *)in++ */
	mov	cl, bl            /* cl = bits, needs it for shifting */
	add	bl, 16             /* bits += 16 */
	shl	eax, cl
	or	edx, eax        /* hold |= *((ushort *)in)++ << bits */

L_get_length_code:
	mov	eax, [esp+52]      /* eax = lmask */
	and	eax, edx          /* eax &= hold */
	mov	eax, [ebp+eax*4] /* eax = lcode[hold & lmask] */

L_dolen:
	mov	cl, ah            /* cl = this.bits */
	sub	bl, ah            /* bits -= this.bits */
	shr	edx, cl           /* hold >>= this.bits */

	test	al, al
	jnz	L_test_for_length_base /* if (op != 0) 45.7% */

	shr	eax, 16            /* output this.val char */
	stosb
	jmp	L_while_test

L_test_for_length_base:
	mov	ecx, eax          /* len = this */
	shr	ecx, 16            /* len = this.val */
	mov	[esp+60], ecx      /* len = this */
	mov	cl, al

	test	al, 16
	jz	L_test_for_second_level_length /* if ((op & 16) == 0) 8% */
	and	cl, 15             /* op &= 15 */
	jz	L_decode_distance    /* if (!op) */
	cmp	bl, cl
	jae	L_add_bits_to_len    /* if (op <= bits) */

	mov	ch, cl            /* stash op in ch, freeing cl */
	xor	eax, eax
	lodsw                         /* al = *(ushort *)in++ */
	mov	cl, bl            /* cl = bits, needs it for shifting */
	add	bl, 16             /* bits += 16 */
	shl	eax, cl
	or	edx, eax         /* hold |= *((ushort *)in)++ << bits */
	mov	cl, ch            /* move op back to ecx */

L_add_bits_to_len:
	mov	eax, 1
	shl	eax, cl
	dec	eax
	sub	bl, cl
	and	eax, edx          /* eax &= hold */
	shr	edx, cl
	add	[esp+60], eax      /* len += hold & mask[op] */

L_decode_distance:
	cmp	bl, 15
	ja	L_get_distance_code  /* if (15 < bits) */

	xor	eax, eax
	lodsw                         /* al = *(ushort *)in++ */
	mov	cl, bl            /* cl = bits, needs it for shifting */
	add	bl, 16             /* bits += 16 */
	shl	eax, cl
	or	edx, eax         /* hold |= *((ushort *)in)++ << bits */

L_get_distance_code:
	mov	eax, [esp+56]      /* eax = dmask */
	mov	ecx, [esp+48]      /* ecx = dcode */
	and	eax, edx          /* eax &= hold */
	mov	eax, [ecx+eax*4]/* eax = dcode[hold & dmask] */

L_dodist:
	mov	ebp, eax          /* dist = this */
	shr	ebp, 16            /* dist = this.val */
	mov	cl, ah
	sub	bl, ah            /* bits -= this.bits */
	shr	edx, cl           /* hold >>= this.bits */
	mov	cl, al            /* cl = this.op */

	test	al, 16             /* if ((op & 16) == 0) */
	jz	L_test_for_second_level_dist
	and	cl, 15             /* op &= 15 */
	jz	L_check_dist_one
	cmp	bl, cl
	jae	L_add_bits_to_dist   /* if (op <= bits) 97.6% */

	mov	ch, cl            /* stash op in ch, freeing cl */
	xor	eax, eax
	lodsw                         /* al = *(ushort *)in++ */
	mov	cl, bl            /* cl = bits, needs it for shifting */
	add	bl, 16             /* bits += 16 */
	shl	eax, cl
	or	edx, eax        /* hold |= *((ushort *)in)++ << bits */
	mov	cl, ch            /* move op back to ecx */

L_add_bits_to_dist:
	mov	eax, 1
	shl	eax, cl
	dec	eax                 /* (1 << op) - 1 */
	sub	bl, cl
	and	eax, edx          /* eax &= hold */
	shr	edx, cl
	add	ebp, eax          /* dist += hold & ((1 << op) - 1) */

L_check_window:
	mov	[esp+4], esi       /* save in so from can use it's reg */
	mov	eax, edi
	sub	eax, [esp+16]      /* nbytes = out - beg */

	cmp	eax, ebp
	jb	L_clip_window        /* if (dist > nbytes) 4.2% */

	mov	ecx, [esp+60]
	mov	esi, edi
	sub	esi, ebp          /* from = out - dist */

	sub	ecx, 3             /* copy from to out */
	mov	al, [esi]
	mov	[edi], al
	mov	al, [esi+1]
	mov	ah, [esi+2]
	add	esi, 3
	mov	[edi+1], al
	mov	[edi+2], ah
	add	edi, 3
	rep     movsb

	mov	esi, [esp+4]      /* move in back to %esi, toss from */
	mov	ebp, [esp+44]     /* ebp = lcode */
	jmp	L_while_test

L_check_dist_one:
	cmp	ebp, 1            /* if dist 1, is a memset */
	jne	L_check_window
	cmp	[esp+16], edi
	je	L_check_window

	dec	edi
	mov	ecx, [esp+60]
	mov	al, [edi]
	sub	ecx, 3

	mov	[edi+1], al       /* memset out with from[-1] */
	mov	[edi+2], al
	mov	[edi+3], al
	add	edi, 4
	rep     stosb
	mov	ebp, [esp+44]      /* ebp = lcode */
	jmp	L_while_test

L_test_for_second_level_length:
	test	al, 64
	jnz	L_test_for_end_of_block /* if ((op & 64) != 0) */

	mov	eax, 1
	shl	eax, cl
	dec	eax
	and	eax, edx         /* eax &= hold */
	add	eax, [esp+60]     /* eax += this.val */
	mov	eax, [ebp+eax*4] /* eax = lcode[val+(hold&mask[op])]*/
	jmp	L_dolen

L_test_for_second_level_dist:
	test	al, 64
	jnz	L_invalid_distance_code /* if ((op & 64) != 0) */

	mov	eax, 1
	shl	eax, cl
	dec	eax
	and	eax, edx         /* eax &= hold */
	add	eax, ebp         /* eax += this.val */
	mov	ecx, [esp+48]     /* ecx = dcode */
	mov	eax, [ecx+eax*4] /* eax = dcode[val+(hold&mask[op])]*/
	jmp	L_dodist

L_clip_window:
	mov	ecx, eax
	mov	eax, [esp+24]     /* prepare for dist compare */
	neg	ecx                /* nbytes = -nbytes */
	mov	esi, [esp+32]     /* from = window */

	cmp	eax, ebp
	jb	L_invalid_distance_too_far /* if (dist > wsize) */

	add	ecx, ebp         /* nbytes = dist - nbytes */
	cmp	dword ptr [esp+28], 0
	jne	L_wrap_around_window /* if (write != 0) */

	sub	eax, ecx
	add	esi, eax         /* from += wsize - nbytes */

	mov	eax, [esp+60]
	cmp	eax, ecx
	jbe	L_do_copy1          /* if (nbytes >= len) */

	sub	eax, ecx         /* len -= nbytes */
	rep     movsb
	mov	esi, edi
	sub	esi, ebp         /* from = out - dist */
	jmp	L_do_copy1

	cmp	eax, ecx
	jbe	L_do_copy1          /* if (nbytes >= len) */

	sub	eax, ecx         /* len -= nbytes */
	rep     movsb
	mov	esi, edi
	sub	esi, ebp         /* from = out - dist */
	jmp	L_do_copy1

L_wrap_around_window:
	mov	eax, [esp+28]
	cmp	ecx, eax
	jbe	L_contiguous_in_window /* if (write >= nbytes) */

	add	esi, [esp+24]
	add	esi, eax
	sub	esi, ecx         /* from += wsize + write - nbytes */
	sub	ecx, eax         /* nbytes -= write */

	mov	eax, [esp+60]
	cmp	eax, ecx
	jbe	L_do_copy1          /* if (nbytes >= len) */

	sub	eax, ecx         /* len -= nbytes */
	rep     movsb
	mov	esi, [esp+32]     /* from = window */
	mov	ecx, [esp+28]     /* nbytes = write */
	cmp	eax, ecx
	jbe	L_do_copy1          /* if (nbytes >= len) */

	sub	eax, ecx         /* len -= nbytes */
	rep     movsb
	mov	esi, edi
	sub	esi, ebp         /* from = out - dist */
	jmp	L_do_copy1

L_contiguous_in_window:
	add	esi, eax
	sub	esi, ecx         /* from += write - nbytes */

	mov	eax, [esp+60]
	cmp	eax, ecx
	jbe	L_do_copy1          /* if (nbytes >= len) */

	sub	eax, ecx         /* len -= nbytes */
	rep     movsb
	mov	esi, edi
	sub	esi, ebp         /* from = out - dist */

L_do_copy1:
	mov	ecx, eax
	rep     movsb

	mov	esi, [esp+4]      /* move in back to %esi, toss from */
	mov	ebp, [esp+44]     /* ebp = lcode */
	jmp	L_while_test

L_test_for_end_of_block:
	test	al, 32
	jz	L_invalid_literal_length_code
	mov	dword ptr [esp+68], 1
	jmp	L_break_loop_with_status

L_invalid_literal_length_code:
	mov	dword ptr [esp+68], 2
	jmp	L_break_loop_with_status

L_invalid_distance_code:
	mov	dword ptr [esp+68], 3
	jmp	L_break_loop_with_status

L_invalid_distance_too_far:
	mov	esi, [esp+4]
	mov	dword ptr [esp+68], 4
	jmp	L_break_loop_with_status

L_break_loop:
	mov	dword ptr [esp+68], 0

L_break_loop_with_status:
/* put in, out, bits, and hold back into ar and pop esp */
	mov	[esp+4], esi
	mov	[esp+12], edi
	mov	[esp+40], ebx
	mov	[esp+36], edx
	mov	esp, [esp]
	pop	ebp
	popfd
    }

    if (ar.status > 1) {
        if (ar.status == 2)
            strm->msg = "invalid literal/length code";
        else if (ar.status == 3)
            strm->msg = "invalid distance code";
        else
            strm->msg = "invalid distance too far back";
        state->mode = BAD;
    }
    else if ( ar.status == 1 ) {
        state->mode = TYPE;
    }

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    ar.len = ar.bits >> 3;
    ar.in -= ar.len;
    ar.bits -= ar.len << 3;
    ar.hold &= (1U << ar.bits) - 1;

    /* update state and return */
    strm->next_in = ar.in;
    strm->next_out = ar.out;
    strm->avail_in = (unsigned)(ar.in < ar.last ? 5 + (ar.last - ar.in) :
                                                  5 - (ar.in - ar.last));
    strm->avail_out = (unsigned)(ar.out < ar.end ? 257 + (ar.end - ar.out) :
                                                   257 - (ar.out - ar.end));
    state->hold = ar.hold;
    state->bits = ar.bits;
}
