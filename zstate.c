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
    Z->stack = (StkId)zuaM_realloc(Z, NULL, 0, ZUA_STACKSIZE * sizeof(TValue));
    Z->stack_size = ZUA_STACKSIZE;
    Z->stack_last = Z->stack + ZUA_STACKSIZE - ZUA_EXTRASTACK; // atttention not extra space it were extra stacl
    Z->next = Z->previous = NULL;
    Z->status = ZUA_OK;
    Z->errorjump = NULL;
    Z->top = Z->stack;
    Z->errorfunc = 0;
    
    int i;
    for (i = 0; i < Z->stack_size; i++) {
        setnilvalue(Z->stack + i);
    }
    Z->top++;

    Z->ci = &(Z->base_ci); /*TODO need reconfirm*/
    Z->ci->func = Z->stack; // the call func position in stack 
    Z->ci->top = Z->stack + ZUA_MINSTACK; // the call func stack top position.
    Z->ci->previous = Z->ci->next = NULL;

}

// the ud means user define
struct zua_State* zua_newstate(zua_Alloc frealloc, void* ud) {
    struct global_State* g;
    struct zua_State* Z;

    struct ZG *zg = (struct ZG *)(*frealloc)(ud, NULL, ZUA_TTHREAD, sizeof(struct ZG));
    if(!zg) return NULL;

    // global state config
    g = &(zg->g); /*TODO need confirm*/
    g->ud = ud;
    g->frealloc = frealloc;
    g->panic = NULL;

    Z = &(zg->zx.z);
    // z.globalstate = g
    G(Z) = g;
    g->mainthread = Z;

    stack_init(Z);

    return Z;
}

#define fromstate(Z) (cast(ZX *, cast(zua_Byte, (Z)) - offsetof(ZX, z)))

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
    while(ci->next) {
        struct CallInfo* next = ci->next->next;
        struct CallInfo* free_ci = ci->next;

        (*g->frealloc)(g->ud, free_ci, sizeof(struct CallInfo), 0);
        ci = next;
    }
    
    free_stack(Z1);
    (*g->frealloc)(g->ud, fromstate(Z1), sizeof(ZG), 0);
}

void setivalue(StkId target, int integer) {
    target->value_.i = integer;
    target->tt_ = ZUA_NUMINT;
}

//  set the stack element as a function
void setfvalue(StkId target, zua_CFunction f) {
    target->value_.f = f;
    target->tt_ = ZUA_TLCF;
}

// set the stack element as a float variable (number )
void setfltvalue(StkId target, float number) {
    target->value_.n = number;
    target->tt_ = ZUA_NUMFLT;
}

// set the stack elemeent as a bool variable 
void setbvalue(StkId target, bool b) {
    target->value_.b = b ? 1 : 0;
    target->tt_ = ZUA_TBOOLEAN;
}

// set the stack element as nil
    /*
        a nil value means the type of Tvalue were NIL
        TValue -->|--> _tt = NIL
                  |--> Value
        
    */
void setnilvalue(StkId target) {
    target->tt_ = ZUA_TNIL;
}

// set the stack element as lightuser data , a void* ptr
void setpvalue(StkId target, void* p) {
    target->value_.p = p;
    target->tt_ = ZUA_TLIGHTUSERDATA;
}

void setobj(StkId target, StkId value) {
    target->value_ = value->value_;
    target->tt_ = value->tt_;
}

void increase_top(struct zua_State* Z) {
    Z->top++;
    assert(Z->top <= Z->stack_last);
}

void zua_pushcfunction(struct zua_State* Z, zua_CFunction f) {
    setfvalue(Z->top, f);
    increase_top(Z);
}

void zua_pushinteger(struct zua_State* Z, int i){
    setivalue(Z->top, i);
    increase_top(Z);
}

void zua_pushnumber(struct zua_State *Z, float n){
    setfltvalue(Z->top, n);
    increase_top(Z);
}

void zua_pushboolean(struct zua_State* Z, bool b){
    setbvalue(Z->top, b);
    increase_top(Z);
}

void zua_pushnil(struct zua_State* Z){
    setnilvalue(Z->top);
    increase_top(Z);
}

void zua_pushlightuserdata(struct zua_State* Z, void* p){
    setpvalue(Z->top, p);
    increase_top(Z);
}

/*
    based on the current index return the addresss which located in Z stack
*/
static TValue* index2addr(struct zua_State* Z, int idx) {
    if (idx >= 0) {
        assert(Z->ci->func + idx < Z->ci->top); /*the idx were offset of each caller inferface in Z stack*/
        return Z->ci->func + idx;
    }
    else {
        assert(Z->top + idx > Z->ci->func);
        return Z->top + idx;
    }
}

/*conver a number like 20.0 to 20*/
zua_Integer zua_tointegerx(struct zua_State* Z, int idx, int* isnum) {
    zua_Integer ret = 0;
    TValue* addr = index2addr(Z, idx);
    if(addr->tt_ == ZUA_NUMINT) {
        ret = addr->value_.i;
        *isnum = 1;
    } else {
        *isnum = 0;
        ZUA_ERROR(Z,"Cannot conver to integer\n");
    }

    return ret;
}

zua_Number zua_tonumberx(struct zua_State* Z, int idx, int* isnum) {
    zua_Number ret = 0.0f;
    TValue* addr = index2addr(Z, idx);
    if (addr->tt_ == ZUA_NUMFLT) {
        *isnum = 1;
        ret = addr->value_.n;
    }
    else
    {
        ZUA_ERROR(Z, "Cannot conver to number\n");
        *isnum = 0;
    }
    return ret;
}

bool zua_toboolean(struct zua_State* Z, int idx) {
    TValue* addr = index2addr(Z, idx);
    /* if element is NIL || element is boolean then return true*/
    return !(addr->tt_ == ZUA_TNIL || addr->value_.b == 0);
}

int zua_isnil(struct zua_State* Z, int idx) {
    TValue* addr = index2addr(Z, idx);
    return addr->tt_ == ZUA_TNIL;
}

/* how many space do we have from current func to stack top*/
int zua_gettop(struct zua_State* Z) {
    return cast(int, Z->top - (Z->ci->func + 1));
}

void zua_settop(struct zua_State *Z, int idx) {
    StkId func = Z->ci->func;
    if (idx >= 0) {
        assert(idx <= Z->stack_last - (func + 1));
        while (Z->top < (func + 1) + idx)
        {
            setnilvalue(Z->top++);
        }
        Z->top = func + 1 + idx;
    }
    /* when the idx were negtive then we shrink the stack*/
    else {
        assert(Z->top + idx > func);
        Z->top = Z->top + idx;
    }
}

void zua_pop(struct zua_State *Z) {
    zua_settop(Z, -1);
}
