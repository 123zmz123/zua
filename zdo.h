#ifndef _ZDO_H
#define _ZDO_H

#include "zstate.h"

typedef int (*Pfunc)(struct zua_State* Z, void* ud); // ud as user define

void seterrobj(struct zua_State* Z, int error);
void zuaD_checkstack(struct zua_State* Z, int need);
void zuaD_growstack(struct zua_State* Z, int size);
void zuaD_throw(struct zua_State* Z, int error);

int zuaD_rawrunprotected(struct zua_State* Z, Pfunc f, void* ud);
int zuaD_precall(struct zua_State* Z, StkId func, int nresult);
int zuaD_poscall(struct zua_State* Z, StkId first_result, int nresult);
int zuaD_call(struct zua_State* Z, StkId func, int nresult);
int zuaD_pcall(struct zua_State* Z, Pfunc f, void* ud, ptrdiff_t oldtop, ptrdiff_t ef);

#endif // DEBUG