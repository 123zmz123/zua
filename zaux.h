#ifndef _ZAUX_H
#define _ZUAX_H

#include "zstate.h"

struct zua_State* zuaL_newstate();
void zuaZ_close(struct zua_State* Z);

void zuaZ_pushinteger(struct zua_State* Z, int integer);
void zuaZ_pushnumber(struct zua_State* Z, float number);
void zuaZ_pushlightuserdata(struct zua_State* Z, void* userdata);
void zuaZ_pushnil(struct zua_State* Z);
void zuaZ_pushcfunction(struct zua_State* Z, zua_CFunction f);
void zuaZ_pushboolean(struct zua_State* Z, bool boolean);
int zuaZ_pcall(struct zua_State* Z, int narg, int nresult);

bool zuaZ_checkinteger(struct zua_State* Z, int idx);
zua_Integer zuaZ_tointeger(struct zua_State* Z, int idx);
zua_Number zuaZ_tonumber(struct zua_State* Z, int idx);
void* zuaZ_touserdata(struct zua_State* Z, int idx);
bool zuaZ_toboolean(struct zua_State* Z, int idx);
int zuaZ_isnil(struct zua_State* Z, int idx);

void zuaZ_pop(struct zua_State* Z);
int zuaZ_stacksize(struct zua_State* Z);

#endif // !ZAUX_H