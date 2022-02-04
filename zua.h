#ifndef _zua_h
#define _zua_h
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#if defined(LLONG_MAX)
#define ZUA_INTEGER long
#define ZUA_NUMBER double
#else
#define ZUA_INTEGER int
#define ZUA_NUMBER float
#endif

#define ZUA_OK 0
#define ZUA_ERRERR 1
#define ZUA_ERRMEM 2
#define ZUA_ERRRUN 3

#define cast(t, exp) ((t)(exp))
#define savestack(Z,o) ((o) - (Z)->stack)
#define restorestack(Z,o) ((Z)->stack + (o))


#define ZUA_TNUMBER 1
#define ZUA_TLIGHTUSERDATA 2
#define ZUA_TBOOLEAN 3
#define ZUA_TSTRING 4
#define ZUA_TNIL 5
#define ZUA_TTABLE 6
#define ZUA_TFUNCTION 7
#define ZUA_TTHREAD 8
#define ZUA_TNONE 9

#define ZUA_MINSTACK 20
#define ZUA_STACKSIZE (2*ZUA_MINSTACK)
#define ZUA_EXTRASTACK 5
#define ZUA_MAXSTACK 15000
#define ZUA_ERRORSTACK 200
#define ZUA_MULRET -1
#define ZUA_MAXCALLS 200

#define ZUA_ERROR(L,s) printf("ZUA ERROR:%s",s);



#endif // !_zua_h
