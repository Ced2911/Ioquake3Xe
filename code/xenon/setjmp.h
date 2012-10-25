#pragma once
#ifdef __cplusplus 
extern "C" { 
#endif

#define JBLEN	(336)
typedef int jmp_buf[JBLEN];

extern int my_setjmp(jmp_buf env);
extern void my_longjmp(jmp_buf env, int val);

#define setjmp my_setjmp
#define longjmp my_longjmp

 #ifdef __cplusplus 
 } 
 #endif
