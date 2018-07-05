.fpu  neon


.macro func name

.macro endfunc
        .size   \name, .-\name
        .purgem endfunc
.endm

        .text
        .align  2
        .global \name
        .type   \name, %function
.endm


/**
* Implement vpadd.u8
* x1: dst
* x2: src1
* x3: src2
* x4: dn, n is even number
* x5: d(n+1)
* x6: q(n/2)
**/
.macro vpadd_u8 x1, x2, x3, x4, x5, x6
        vpaddl.u8   \x4,    \x2
        vshr.u16    \x4,    \x4,    #1
        vpaddl.u8   \x5,    \x3
        vshr.u16    \x5,    \x5,    #1
        vmovn.i16   \x1,    \x6
.endm


/**
* Simple version vpadd_u8
* temp1, temp2, temp3
**/
.macro vpadd_u8_s x1, x2
        vpadd_u8 \x1, \x1, \x2, temp1, temp2, temp3
.endm


/**
* x1: red
* x2: green
* x3: blue
* x4: src
**/
.macro read_rgb555 x1, x2, x3, x4
        vld2.u8     {temp1, temp2}, [\x4]!  // Load data from src1
        vand.i8     \x3,    temp1,  temp4   // blue = temp1 & b00011111
        vand.i8     \x2,    temp2,  temp5   // green = temp2 & b00000111
        vshl.u8     \x2,    \x2,    #3      // green = green << 3
        vsri.u8     \x2,    temp1,  #5      // green = (green & b11111000) | (temp1 >> 5)
        vshr.u8     \x1,    temp2,  #3      // red1  = temp2 >> 3
.endm


// void rgba8888_to_rgba8888_neon_2(uint8_t* dst, uint8_t* src1, uint8_t* src2, uint32_t count);
func    rgba8888_to_rgba8888_neon_2
dst     .req    r0
src1    .req    r1
src2    .req    r2
count   .req    r3
red1    .req    d0
green1  .req    d1
blue1   .req    d2
alpha1  .req    d3
red2    .req    d4
green2  .req    d5
blue2   .req    d6
alpha2  .req    d7
red3    .req    d8
green3  .req    d9
blue3   .req    d10
alpha3  .req    d11
temp1   .req    d12
temp2   .req    d13
temp3   .req    q6
rgba8888_to_rgba8888_neon_2:
        lsr         count,  #3              // count >>= 3 // count /= 8
        teq         count,  #0              //
        beq         .loop_end2              // if (count == 0) goto loop_end2

.loop_start2:
        vld4.u8     {red1, green1, blue1, alpha1}, [src1]!    // Load r, g, b, a from src
        vld4.u8     {red2, green2, blue2, alpha2}, [src1]!    // Load r, g, b, a from src
        vpadd_u8_s  red1,   red2
        vpadd_u8_s  green1, green2
        vpadd_u8_s  blue1,  blue2
        vpadd_u8_s  alpha1, alpha2

        vld4.u8     {red2, green2, blue2, alpha2}, [src2]!    // Load r, g, b, a from src
        vld4.u8     {red3, green3, blue3, alpha3}, [src2]!    // Load r, g, b, a from src
        vpadd_u8_s  red2,   red3
        vpadd_u8_s  green2, green3
        vpadd_u8_s  blue2,  blue3
        vpadd_u8_s  alpha2, alpha3

        vhadd.u8    red1,   red1,   red2
        vhadd.u8    green1, green1, green2
        vhadd.u8    blue1,  blue1,  blue2
        vhadd.u8    alpha1, alpha1, alpha2

        vst4.8      {red1, green1, blue1, alpha1}, [dst]!

        subs        count, count,  #1      // count -= 1
        bne         .loop_start2           // if (count != 0) goto loop_start2
.loop_end2:

        bx          lr
.unreq  dst
.unreq  src1
.unreq  src2
.unreq  count
.unreq  red1
.unreq  green1
.unreq  blue1
.unreq  alpha1
.unreq  red2
.unreq  green2
.unreq  blue2
.unreq  alpha2
.unreq  red3
.unreq  green3
.unreq  blue3
.unreq  alpha3
.unreq  temp1
.unreq  temp2
.unreq  temp3
endfunc


// void rgba8888_to_rgb565_neon(uint8_t* dst, const uint8_t* src, uint32_t count);
func    rgba8888_to_rgb565_neon
dst     .req    r0
src     .req    r1
count   .req    r2
red     .req    d0
green   .req    d1
blue    .req    d2
alpha   .req    d3
rgba8888_to_rgb565_neon:
        lsr         count,  #3              // count >>= 3 // count /= 8
        teq         count,  #0              //
        beq         .loop_end1              // if (count == 0) goto loop_end1

.loop_start1:
        vld4.u8     {red, green, blue, alpha}, [src]!    // Load r, g, b, a from src

        vmov.u8     alpha, red              //
        vsri.u8     alpha, green,  #5       // a = (r & b11111000) | (g >> 5)
        vshl.u8     green, green,  #3       //
        vsri.u8     green, blue,   #3       // g = (g << 3) | (b >> 3)

        vst2.8      {green, alpha}, [dst]!

        subs        count, count,  #1      // count -= 1
        bne         .loop_start1           // if (count != 0) goto loop_start1
.loop_end1:

        bx          lr
.unreq  dst
.unreq  src
.unreq  count
.unreq  red
.unreq  green
.unreq  blue
.unreq  alpha
endfunc


// void rgb565_to_rgb565_neon_2(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, uint32_t count);
func    rgb565_to_rgb565_neon_2
dst     .req    r0
src1    .req    r1
src2    .req    r2
count   .req    r3
blue1   .req    d0
green1  .req    d1
red1    .req    d2
blue2   .req    d3
green2  .req    d4
red2    .req    d5
red3    .req    d6
green3  .req    d7
blue3   .req    d8
temp1   .req    d10
temp2   .req    d11
temp3   .req    q5
temp4   .req    d12
temp5   .req    d13
rgb565_to_rgb565_neon_2:
        push        {r4}

        lsr         count,  #3              // count >>= 3 // count /= 8
        teq         count,  #0              //
        beq         .loop_end3              // if (count == 0) goto loop_end3

        mov         r4,     #0x1F
        vdup.8      temp4,  r4
        mov         r4,     #0x07
        vdup.8      temp5,  r4

.loop_start3:
        read_rgb555 red1,   green1, blue1,  src1
        read_rgb555 red2,   green2, blue2,  src1
        vpadd_u8_s  red1,   red2
        vpadd_u8_s  green1, green2
        vpadd_u8_s  blue1,  blue2

        read_rgb555 red2,   green2, blue2,  src2
        read_rgb555 red3,   green3, blue3,  src2
        vpadd_u8_s  red2,   red3
        vpadd_u8_s  green2, green3
        vpadd_u8_s  blue2,  blue3

        vhadd.u8    red1,   red1,   red2
        vhadd.u8    green1, green1, green2
        vhadd.u8    blue1,  blue1,  blue2

        vsli.u8     blue1,  green1,  #5
        vshr.u8     green1, green1,  #3
        vsli.u8     green1, red1,    #3

        vst2.8      {blue1, green1}, [dst]!

        subs        count,  count,   #1      // count -= 1
        bne         .loop_start3           // if (count != 0) goto loop_start3
.loop_end3:

        pop         {r4}
        bx          lr
.unreq  dst
.unreq  src1
.unreq  src2
.unreq  count
.unreq  red1
.unreq  green1
.unreq  blue1
.unreq  red2
.unreq  green2
.unreq  blue2
.unreq  red3
.unreq  green3
.unreq  blue3
.unreq  temp1
.unreq  temp2
.unreq  temp3
.unreq  temp4
.unreq  temp5
endfunc
