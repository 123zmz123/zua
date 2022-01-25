#include "zstate.h"
#include "zmem.h"

typedef struct ZX {
    zua_Byte extra_[ZUA_EXTRASPACE];
    zua_State z;
} ZX;

typedef struct ZG {
    ZX zx;
    global_State g;
} ZG;

static void stack_init(struct zua_State* Z) {
    // obj size size as 0, new size were ZUA_STACKSIZE*sizeof(TValue)
    Z->stack = (StkId) zuaM_realloc(Z,NULL,0,ZUA_STACKSIZE*sizeof(TValue));
    Z->stack_size = ZUA_STACKSIZE;
    Z->stack_last = Z->stack + ZUA_STACKSIZE - ZUA_EXTRASTACK; // atttention not extra space it were extra stacl
    Z->next = Z->previous = NULL;
    Z->status = ZUA_OK;
    Z->errorjump = NULL;
    Z->top = Z->stack;
    Z->errorfunc = 0;
    
    int i;
    for (i=0; i < Z->stack_size; i++) {
        setnilvalue(Z->stack+i);
    }
    Z->top++;
    Z->ci = &(Z->base_ci);
    Z->ci->func = Z->stack; // the call func position in stack 
    Z->ci->top = Z->stack + ZUA_MINSTACK; // the call func stack top position.
    Z->ci->previous = Z->ci->next = NULL;

}

// the ud means user define
struct zua_State* zua_newstate(zua_Alloc frealloc, void* ud)
{
    struct global_State* g;
    struct zua_State* Z;
    struct ZG* zg = (struct ZG*)(*frealloc)(ud,NULL,ZUA_THREAD,sizeof(struct ZG));
    if(!zg) return NULL;

    // global state config
    g = &(zg->g);
    g->ud = ud;
    g->frealloc = frealloc;
    g->panic = NULL;

    Z = &(zg->zx.z);
    // z.globalstate = g
    G(Z)=g;
    g->mainthread = Z;
    stack_init(Z);

    return Z;
}

#define fromstate(Z) cast(ZX*,cast(zua_Byte,(Z)) - offsetof(ZX,z))

static void free_stack(struct zua_State* Z) {
    global_State* g = G(Z);
    (*g->frealloc)(g->ud, Z->stack, sizeof(TValue), 0); // if the nsize were 0 then we will free it.
    Z->stack = Z->stack_last = Z->top = NULL;
    Z->stack_size = 0;
}

void zua_close(struct zua_State* Z) {
    struct global_State* g = G(Z);
    struct zua_State* Z1 = g->mainthread;

    struct CallInfo* ci = &(Z1->base_ci);

    // free every ci (CallInfo struct) until ci->next were empty
    while (ci->next)
    {
        struct CallInfo* next = ci->next->next;
        struct CallInfo* free_ci = ci->next;
        (*g->frealloc)(g->ud,free_ci,sizeof(struct CallInfo),0);
        ci = next;
    }
    
    free_stack(Z1);
    (*g->frealloc)(g->ud, fromstate(Z1), sizeof(ZG), 0);
}

void setivalue(StkId target, int integer) {
    target->value_.i = integer;
    target->tt_ = ZUA_NUMINT;
}

//  the stack element were a function
void setfvalue(StkId target, zua_CFunction f) {
    target->value_.f = f;
    target->tt_ = ZUA_TLCF;
}

// the stack element were a float variable
void setfltvalue(StkId target, float number) {
    target->value_.n = number;
    target->tt_ = ZUA_NUMFLT;
}

void setnilvalue (StkId target) {
    /*
        a nil value means the type of Tvalue were NIL
        TValue -->|--> _tt = NIL
                  |--> Value
        
    */
    target->tt_ = ZUA_TNIL; 
}