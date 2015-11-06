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
    size_t input_len;
    const char* input = luaL_checklstring(L, 1, &input_len);
    // see UCL's README
    ucl_uint output_len = input_len + (input_len / 8) + 256;
    void* output = lua_newuserdata(L, output_len);
    ucl_progress_callback_p callback = NULL;
    int level = 9;
    struct ucl_compress_config_p conf = NULL;
    ucl_uintp result = NULL;
    int status = ucl_nrv2b_99_compress(
        input,
        input_len,
        output,
        &output_len,
        callback,
        level,
        conf,
        result
    );
    if (status != UCL_E_OK) {
        // TODO implement all statuses from uclconf.h
        return luaL_error(L, "UCL error occurred");
    }
    lua_pushlstring(L, output, output_len);
    return 1;
}

int luaucl_decompress(lua_State* L) {
    size_t input_len;
    const char* input = luaL_checklstring(L, 1, &input_len);
    // output_len is max possible output
    int output_len = luaL_checkinteger(L, 2);
    void* output = lua_newuserdata(L, output_len);
    ucl_voidp wrkmem = NULL;
    int status = ucl_nrv2b_decompress_8(
        input,
        input_len,
        output,
        &output_len,
        wrkmem
    );
    if (status != UCL_E_OK) {
        // TODO implement all statuses from uclconf.h
        return luaL_error(L, "UCL error occurred");
    }
    lua_pushlstring(L, output, output_len);
    return 1;
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
