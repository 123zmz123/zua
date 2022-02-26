#include "zdo.h"
#include "zmem.h"

#define ZUA_TRY(Z, c, a) if (_setjmp((c)->b) == 0) { a }

#define ZUA_THROW(c) longjmp((c)->b, 1)

struct zua_longjmp {
    struct zua_longjmp* previous;
    jmp_buf b;
    int status;
};

void seterrobj(struct zua_State* Z, int error) {
    zua_pushinteger(Z, error);
}

void zuaD_checkstack(struct zua_State* Z, int need) {
    if (Z->top + need > Z->stack_last) {
        zuaD_growstack(Z, need);
    }
}

void zuaD_growstack(struct zua_State* Z, int size) {
    if(Z->stack_size > ZUA_MAXSTACK) {
        zuaD_throw(Z, ZUA_ERRERR);
    }

    int stack_size = Z->stack_size * 2;
    int need_size = cast(int, Z->top - Z->stack) + size + ZUA_EXTRASTACK;
    if (stack_size < need_size) {
        stack_size = need_size;
    }

    if (stack_size > ZUA_MAXSTACK) {
        stack_size = ZUA_MAXSTACK + ZUA_ERRORSTACK;
        ZUA_ERROR(Z, "stack overflow");
    }

    TValue* old_stack = Z->stack;
    Z->stack = zuaM_realloc(Z, Z->stack, Z->stack_size, stack_size * sizeof(TValue));
    Z->stack_size = stack_size;
    Z->stack_last = Z->stack + stack_size - ZUA_EXTRASTACK;
    int top_diff = cast(int, Z->top - old_stack);
    Z->top = restorestack(Z, top_diff);

    struct CallInfo* ci;
    ci = &Z->base_ci;
    while(ci) {
        int func_diff = cast(int, ci->func - old_stack);
        int top_diff = cast(int, ci->top - old_stack);
        ci->func = restorestack(Z, func_diff);
        ci->top = restorestack(Z, top_diff);

        ci = ci->next;
    }
}

void zuaD_throw(struct zua_State* Z, int error) {
    struct global_State* g = G(Z);
    if (Z->errorjump) {
        Z->errorjump->status = error;
        ZUA_THROW(Z->errorjump);
    }
    else {
       if (g->panic) {
           (*g->panic)(Z);
       }
       abort();
    }
}

//TODO what's the purpose of it?
int zuaD_rawrunprotected(struct zua_State* Z, Pfunc f, void* ud) {
    int old_ncalls = Z->ncalls;
    struct zua_longjmp lj;
    lj.previous = Z->errorjump;
    lj.status = ZUA_OK;
    Z->errorjump = &lj;

    ZUA_TRY(
        Z,
        Z->errorjump,
        (*f)(Z, ud);
    )

    Z->errorjump = lj.previous;
    Z->ncalls = old_ncalls;
    return lj.status;
}

// seems like apply for a new ci
static struct CallInfo* next_ci(struct zua_State* Z, StkId func, int nresult) {
    struct global_State* g = G(Z);
    struct CallInfo* ci;
    ci = zuaM_realloc(Z, NULL, 0, sizeof(struct CallInfo));
    ci->next = NULL;
    ci->previous = Z->ci;
    Z->ci->next = ci;
    ci->nresult = nresult;
    ci->callstatus = ZUA_OK;
    ci->func = func;
    ci->top = Z->top + ZUA_MINSTACK;
    Z->ci = ci;

    return ci;
}

// prepare for function call. 
// if we call a c function, just directly call it
// if we call a lua function, just prepare for call it
int zuaD_precall(struct zua_State* Z, StkId func, int nresult) {
    switch(func->tt_) {
        case ZUA_TLCF: {
            zua_CFunction f = func->value_.f;

            ptrdiff_t func_diff = savestack(Z, func);
            zuaD_checkstack(Z, ZUA_MINSTACK);
            func = restorestack(Z, func_diff);

            next_ci(Z, func, nresult);
            int n = (*f)(Z);
            assert(Z->ci->func + n <= Z->ci->top);
            zuaD_poscall(Z, Z->top - n, n);
        return 1;
    } break;
    default:break;
    }

    return 0;
}

int zuaD_poscall(struct zua_State* Z, StkId first_result, int nresult) {
    StkId func = Z->ci->func;
    int nwant = Z->ci->nresult;

    switch(nwant) {
    case 0: {
        Z->top = Z->ci->func;
    } break;
    case 1: {
        if (nresult == 0) {
            first_result->value_.p = NULL;
            first_result->tt_ = ZUA_TNIL;
        }
        setobj(func, first_result); // func point the function address in stack
        first_result->value_.p = NULL;
        first_result->tt_ = ZUA_TNIL;

        Z->top = func + nwant;
    } break;

    case ZUA_MULRET: {
        int nres = cast(int, Z->top - first_result); // top point to the current top of the stack not the boundary of top
        int i;
        for (i = 0; i < nres; i++) {
            StkId current = first_result + i;
            setobj(func + i, current); // store the return value to ci struct
            current->value_.p = NULL;
            current->tt_ = ZUA_TNIL;
        }
        Z->top = func + nres;
    } break;
    default: {
        if (nwant > nresult) { // nwant: expected return result, nresult: real result number
            int i;
            for (i = 0; i < nwant; i++) {
                if (i < nresult) {
                    StkId current = first_result + i;
                    setobj(func + i, current); // store the return value to ci struct
                    current->value_.p = NULL;
                    current->tt_ = ZUA_TNIL;
                }
                else {
                    StkId stack = func + i;
                    stack->tt_ = ZUA_TNIL;
                }
            }
            Z->top = func + nwant;
        }
        else { // if returned result were more than expected result
            int i;
            for (i = 0; i < nresult; i++) {
                if (i < nwant) { // put the expected result to ci
                    StkId current = first_result + i;
                    setobj(func + i, current);
                    current->value_.p = NULL;
                    current->tt_ = ZUA_TNIL;
                }
                else { // abandon the unexpected result
                    StkId stack = func + i;
                    stack->value_.p = NULL;
                    stack->tt_ = ZUA_TNIL;
                }
            }
            Z->top = func + nresult;
        }
    } break;
    }

    struct CallInfo* ci = Z->ci;
    Z->ci = ci->previous;
    Z->ci->next = NULL;

    struct global_State* g = G(Z);
    (*g->frealloc)(g->ud, ci, sizeof(struct CallInfo), 0);

    return ZUA_OK;
}

int zuaD_call(struct zua_State* Z, StkId func, int nresult) {
    if(++Z->ncalls > ZUA_MAXCALLS) {
        zuaD_throw(Z, 0);
    }

    if(!zuaD_precall(Z, func,nresult)) {
        //TODO implement zuaV_execute(Z);
    }
    Z->ncalls--;
    return ZUA_OK;
}
// in range old_top ~ current top , we clear the ptr->memory on each stack element
static void reset_unuse_stack(struct zua_State* Z, ptrdiff_t old_top) {
    struct global_State* g = G(Z);
    StkId top = restorestack(Z, old_top);
    for (; top < Z->top; top++) {
        if (top->value_.p) {
            (*g->frealloc)(g->ud, top->value_.p, sizeof(top->value_.p), 0);
            top->value_.p = NULL;
        }
        top->tt_ = ZUA_TNIL;
    }
}

int zuaD_pcall(struct zua_State* Z, Pfunc f, void* ud, ptrdiff_t oldtop, ptrdiff_t ef) {
    int status;
    struct CallInfo* old_ci = Z->ci;
    ptrdiff_t old_errorfunc = Z->errorfunc;

    status = zuaD_rawrunprotected(Z, f, ud);
    if (status != ZUA_OK) {
        struct global_State* g = G(Z);
        struct CallInfo* free_ci = Z->ci;
        while(free_ci) { 
            if(free_ci == old_ci) { // when the ci were beyond the old_ci we need free it
                free_ci = free_ci->next;
                continue;
            }

            struct CallInfo* previous = free_ci->previous;
            previous->next = NULL;

            struct CallInfo* next = free_ci->next;
            (*g->frealloc)(g->ud, free_ci, sizeof(struct CallInfo), 0);
            free_ci = next;
        }

        reset_unuse_stack(Z, oldtop);
        Z->ci = old_ci;
        Z->top = restorestack(Z, oldtop);
        seterrobj(Z, status);
    }

    Z->errorfunc = old_errorfunc;
    return status;
}



















