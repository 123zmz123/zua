#include "zaux.h"
#include "zdo.h"

static void* z_alloc(void* ud, void* ptr, size_t osize, size_t nsize) {
    (void)ud;
    (void)osize;


    if (nsize == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, nsize);
}

struct zua_State* zuaL_newstate() {
    struct zua_State* Z = zua_newstate(&z_alloc, NULL); // this is the entry of the everything
    return Z;
}

void zuaZ_close(struct zua_State* Z) {
    zua_close(Z);
}

void zuaZ_pushinteger(struct zua_State* Z, int integer) {
    zua_pushinteger(Z, integer);
}

void zuaZ_pushnumber(struct zua_State* Z, float number) {
    zua_pushnumber(Z, number);
}

void zuaZ_pushlightuserdata(struct zua_State* Z, void* userdata) {
    zua_pushlightuserdata(Z, userdata);
}

void zuaZ_pushnil(struct zua_State* Z) {
    zua_pushnil(Z);
}

void zuaZ_pushcfunction(struct zua_State* Z, zua_CFunction f) {
    zua_pushcfunction(Z, f);
}

void zuaZ_pushboolean(struct zua_State* Z, bool boolean) {
    zua_pushboolean(Z, boolean);
}


typedef struct CallS {
    StkId func;
    int nresult;
} CallS;

static int f_call(zua_State* Z, void* ud) {
    CallS* c = cast(CallS*, ud);
    zuaD_call(Z, c->func, c->nresult);
    return ZUA_OK;
}

int zuaZ_pcall(struct zua_State* Z, int narg, int nresult) {
    int status = ZUA_OK;
    CallS c;
    c.func = Z->top - (narg + 1); // func were reside below the input patameter
    c.nresult = nresult;
    // savestack = Z->top - Z->stack
    status = zuaD_pcall(Z, &f_call, &c, savestack(Z, Z->top), 0); 
    return status;
}

bool zuaZ_checkinteger(struct zua_State* Z, int idx) {
    int isnum = 0;
    zua_tointegerx(Z, idx, &isnum); // try to convert it to integer
    if (isnum) {
        return true;
    }
    else {
        return false;
    }
}

zua_Integer zuaZ_tointeger(struct zua_State* Z, int idx) {
    int isnum = 0;
    zua_Integer ret = zua_tointegerx(Z, idx, &isnum);
    return ret;
}

zua_Number zuaZ_tonumber(struct zua_State* Z, int idx) {
    int isnum = 0;
    zua_Number ret = zua_tonumberx(Z, idx, &isnum);
    return ret;
}

void *zuaZ_touserdata(struct zua_State* Z, int idx) {
    return NULL;
}

bool zuaZ_toboolean(struct zua_State *Z, int idx) {
    return zua_toboolean(Z, idx);
}

int zuaZ_isnil(struct zua_State* Z, int idx) {
    return zua_isnil(Z, idx);
}

void zuaZ_pop(struct zua_State* Z) {
    zua_pop(Z);
}

int zuaZ_stacksize(struct zua_State* Z) { // how many free space do we have 
    return zua_gettop(Z);
}