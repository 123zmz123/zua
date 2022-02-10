#ifndef _zsate_h
#define _zsate_h

#include "zobject.h"

#define ZUA_EXTRASPACE sizeof(void*)
#define G(L) ((L)->l_G)

typedef TValue* StkId;

struct CallInfo {
    StkId func; // that the called func location in stack
    StkId top; // the called func stack top 
    int nresult; // return number
    int callstatus; // the function status 
    struct CallInfo* next; // the next caller
    struct CallInfo* previous; // the previous caller
};

typedef struct zua_State {
    StkId stack; // stack, points to a memory region
    StkId stack_last; // stack boundry
    StkId top; // top of the stack
    int stack_size; 
    struct zua_longjmp* errorjump;
    int status;
    struct lua_State* next;
    struct lua_State* previous;
    struct CallInfo base_ci;
    struct CallInfo* ci;
    struct global_State* l_G; // it points to a global state which will manage all the states
    ptrdiff_t errorfunc;
    int ncalls;
} zua_State;

typedef struct global_State {
    struct zua_State* mainthread;
    zua_Alloc frealloc;
    void* ud;
    zua_CFunction panic;
} global_State;

struct zua_State* zua_newstate(zua_Alloc alloc, void* ud);
void setnilvalue(StkId target);
void zua_close(struct zua_State* Z); 

void setivalue(StkId target, int integer);
void setfvalue(StkId target, zua_CFunction f);
void setfltvalue(StkId target, float number);
void setbvalue(StkId target, bool b);
void setnilvalue(StkId target);
void setpvalue(StkId target, void* p);

void setobj(StkId target, StkId value);

void increase_top(struct zua_State* Z);
void zua_pushcfunction(struct zua_State* Z, zua_CFunction f);
void zua_pushinteger(struct zua_State* Z, int i);
void zua_pushnumber(struct zua_State *Z, float n);
void zua_pushboolean(struct zua_State* Z, bool b);
void zua_pushnil(struct zua_State* Z);
void zua_pushlightuserdata(struct zua_State* Z, void* p);

zua_Integer zua_tointegerx(struct zua_State* Z, int idx, int* isnum);
zua_Number zua_tonumberx(struct zua_State* Z, int idx, int* isnum);
bool zua_toboolean(struct zua_State* Z, int idx);
int zua_isnil(struct zua_State* Z, int idx);

void zua_settop(struct zua_State *Z, int idx);
int zua_gettop(struct zua_State* Z);
void zua_pop(struct zua_State *Z);

#endif // !_zua_sate_h