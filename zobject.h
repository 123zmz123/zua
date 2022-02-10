#ifndef _zobject_h
#define _zobject_h

#include "zua.h"

typedef struct zua_State zua_State;

typedef ZUA_INTEGER zua_Integer;
typedef ZUA_NUMBER zua_Number;
typedef unsigned char zua_Byte;
typedef int (*zua_CFunction)(zua_State* L);
typedef void* (*zua_Alloc)(void* ud, void* ptr, size_t osize, size_t nsize);

// zua number type 
#define ZUA_NUMINT (ZUA_TNUMBER | (0 << 4))
#define ZUA_NUMFLT (ZUA_TNUMBER | (1 << 4))

// zua function type
#define ZUA_TLCL (ZUA_TFUNCTION | (0 << 4))
#define ZUA_TLCF (ZUA_TFUNCTION | (1 << 4))
#define ZUA_TCCL (ZUA_TFUNCTION | (2 << 4))

// string type
#define ZUA_LNGSTR (ZUA_TSTRING | (0 << 4))
#define ZUA_SHRSTR (ZUA_TSTRING | (1 << 4))

typedef union zua_Value {
	void* p;
	int b;
	zua_Integer i;
	zua_Number n;
	zua_CFunction f;
}Value;

typedef struct zua_TValue {
	Value value_;
	int tt_;
} TValue;

#endif //
