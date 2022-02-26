
#include "zaux.h"
#include "zdo.h"

static void* z_alloc(void* ud, void* ptr, size_t osize, size_t nsize) {
    (void)ud;
    (void)osize;

    if(nsize == 0) {
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

void zuaZ_pushfunction(struct zua_State* Z, zua_CFunction f) {
    
}