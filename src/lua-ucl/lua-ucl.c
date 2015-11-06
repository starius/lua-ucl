// lua-ucl, Lua bindings to UCL, data compression library
// Copyright (C) 2015 Boris Nagaev
// See the LICENSE file for terms of use.

// UCL is a portable lossless data compression library
// written in ANSI C
// See http://www.oberhumer.com/opensource/ucl/

#include <lua.h>
#include <lauxlib.h>

#include <ucl/ucl.h>

#if LUA_VERSION_NUM == 501
#define luaucl_setfuncs(L, funcs) luaL_register(L, NULL, funcs)
#define luaucl_objlen lua_objlen
#else
#define luaucl_setfuncs(L, funcs) luaL_setfuncs(L, funcs, 0)
#define luaucl_objlen lua_rawlen
#endif

int luaucl_compress(lua_State* L) {
    return 0;
}

int luaucl_decompress(lua_State* L) {
    return 0;
}

luaL_Reg luaucl_functions[] = {
    {"compress", luaucl_compress},
    {"decompress", luaucl_decompress},
    {}
};

int luaopen_ucl(lua_State* L) {
    int nf = sizeof(luaucl_functions) / sizeof(luaL_Reg) - 1;
    lua_createtable(L, 0, nf);
    luaucl_setfuncs(L, luaucl_functions);
    return 1;
}
