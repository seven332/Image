.fpu  neon


.macro func name

.macro endfunc
        .size   \name, .-\name
        .endfunc
        .purgem endfunc
.endm

        .text
        .align  2
        .global \name
        .type   \name, %function
        .func   \name
.endm


// void rgba8888_to_rgb565_neon(uint8_t* dst, const uint8_t* src, uint32_t count);
func    rgba8888_to_rgb565_neon
dst     .req    r0
src     .req    r1
count   .req    r2
temp    .req    r3
red     .req    d0
green   .req    d1
blue    .req    d2
alpha   .req    d3
rgba8888_to_rgb565_neon:
        lsr         count,  #3              // count >>= 3 // count /= 8
        teq         count,  #0              //
        beq         .loop_end               // if (count == 0) goto loop_end

.loop_start:
        vld4.u8     {red, green, blue, alpha}, [src]!    // Load r, g, b, a from src

        vmov.u8     alpha, red              //
        vsri.u8     alpha, green,  #5       // a = (r & b11111000) | (g >> 5)
        vshl.u8     green, green,  #3       //
        vsri.u8     green, blue,   #3       // g = (g << 3) | (b >> 3)

        vst2.8      {green, alpha}, [dst]!

        subs        r2,     r2,     #1      // count -= 1
        bne         .loop_start             // if (count != 0) goto loop_start
.loop_end:

        bx          lr
endfunc
