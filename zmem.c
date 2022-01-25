#include"zmem.h"
/*
    osize stack old size
    nsize stack new size
*/
void* zuaM_realloc(zua_State* Z, void* ptr, size_t osize, size_t nsize){
    struct global_State* g = G(Z);
    int oldsize = ptr? osize: 0;
    void* ret = g->frealloc(g->ud,ptr,osize,nsize);
    if(ret==NULL){
        //TODO need implement a throw error;
    }
    return ret;
}