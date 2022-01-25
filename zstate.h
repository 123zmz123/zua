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
    zua_Byte status;
    struct lua_State* next;
    struct lua_State* previous;
    struct CallInfo base_ci;
    struct CallInfo* ci;
    struct global_State* l_G; // it points to a global state which will manage all the states
    ptrdiff_t errorfunc;
    int ncalls;
}zua_State;

typedef struct global_State {
    struct zua_State* mainthread;
    zua_Alloc frealloc;
    void* ud;
    zua_CFunction panic;
}global_State;

void setnilvalue(StkId target);
#endif // !_zua_sate_h