// from arch/ppc64/xmon/setjmp.c

int my_setjmp(long *buf)  /* NOTE: assert(sizeof(buf) > 184) */
{
        /* XXX should save fp regs as well */
        asm volatile (
        "mflr 0; std 0,0(%0)\n\
         std    1,8(%0)\n\
         std    2,16(%0)\n\
         mfcr 0; std 0,24(%0)\n\
         std    13,32(%0)\n\
         std    14,40(%0)\n\
         std    15,48(%0)\n\
         std    16,56(%0)\n\
         std    17,64(%0)\n\
         std    18,72(%0)\n\
         std    19,80(%0)\n\
         std    20,88(%0)\n\
         std    21,96(%0)\n\
         std    22,104(%0)\n\
         std    23,112(%0)\n\
         std    24,120(%0)\n\
         std    25,128(%0)\n\
         std    26,136(%0)\n\
         std    27,144(%0)\n\
         std    28,152(%0)\n\
         std    29,160(%0)\n\
         std    30,168(%0)\n\
         std    31,176(%0)\n\
         stfd   14,184(%0)\n\
         stfd   15,192(%0)\n\
         stfd   16,200(%0)\n\
         stfd   17,208(%0)\n\
         stfd   18,216(%0)\n\
         stfd   19,232(%0)\n\
         stfd   20,240(%0)\n\
         stfd   21,248(%0)\n\         
         stfd   22,256(%0)\n\
         stfd   23,264(%0)\n\
         stfd   24,272(%0)\n\
         stfd   25,280(%0)\n\         
         stfd   26,288(%0)\n\
         stfd   27,296(%0)\n\
         stfd   28,304(%0)\n\
         stfd   29,312(%0)\n\         
         stfd   30,320(%0)\n\
         stfd   31,328(%0)\n\
         " : : "r" (buf));
    return 0;
}

void my_longjmp(long *buf, int val)
{
        if (val == 0)
                val = 1;
        asm volatile (
        "ld     13,32(%0)\n\
         ld     14,40(%0)\n\
         ld     15,48(%0)\n\
         ld     16,56(%0)\n\
         ld     17,64(%0)\n\
         ld     18,72(%0)\n\
         ld     19,80(%0)\n\
         ld     20,88(%0)\n\
         ld     21,96(%0)\n\
         ld     22,104(%0)\n\
         ld     23,112(%0)\n\
         ld     24,120(%0)\n\
         ld     25,128(%0)\n\
         ld     26,136(%0)\n\
         ld     27,144(%0)\n\
         ld     28,152(%0)\n\
         ld     29,160(%0)\n\
         ld     30,168(%0)\n\
         ld     31,176(%0)\n\
         lfd    14,184(%0)\n\
         lfd    15,192(%0)\n\
         lfd    16,200(%0)\n\
         lfd    17,208(%0)\n\
         lfd    18,216(%0)\n\
         lfd    19,232(%0)\n\
         lfd    20,240(%0)\n\
         lfd    21,248(%0)\n\         
         lfd    22,256(%0)\n\
         lfd    23,264(%0)\n\
         lfd    24,272(%0)\n\
         lfd    25,280(%0)\n\         
         lfd    26,288(%0)\n\
         lfd    27,296(%0)\n\
         lfd    28,304(%0)\n\
         lfd    29,312(%0)\n\         
         lfd    30,320(%0)\n\
         lfd    31,328(%0)\n\
         ld     0,24(%0)\n\
         mtcrf  0x38,0\n\
         ld     0,0(%0)\n\
         ld     1,8(%0)\n\
         ld     2,16(%0)\n\
         mtlr   0\n\
         mr     3,%1\n\
         " : : "r" (buf), "r" (val));
}
