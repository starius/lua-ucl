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
#else
#define luaucl_setfuncs(L, funcs) luaL_setfuncs(L, funcs, 0)
#endif

typedef struct ucl_code_Reg {
    const char* name;
    int code;
} ucl_code_Reg;

#define UCL_CODE(code) {#code, code}
static ucl_code_Reg ucl_codes[] = {
    UCL_CODE(UCL_E_OK),
    UCL_CODE(UCL_E_ERROR),
    UCL_CODE(UCL_E_INVALID_ARGUMENT),
    UCL_CODE(UCL_E_OUT_OF_MEMORY),
    UCL_CODE(UCL_E_NOT_COMPRESSIBLE),
    UCL_CODE(UCL_E_INPUT_OVERRUN),
    UCL_CODE(UCL_E_OUTPUT_OVERRUN),
    UCL_CODE(UCL_E_LOOKBEHIND_OVERRUN),
    UCL_CODE(UCL_E_EOF_NOT_FOUND),
    UCL_CODE(UCL_E_INPUT_NOT_CONSUMED),
    UCL_CODE(UCL_E_OVERLAP_OVERRUN),
    {}
};
#undef UCL_CODE

const char* ucl_name_code(int code) {
    const char* name = "Unknown error";
    ucl_code_Reg* i;
    for (i = ucl_codes; i->name; ++i) {
        if (i->code == code) {
            name = i->name;
            break;
        }
    }
    return name;
}

const int MIN_LARGE_SIZE = 512 * 1024;  // 512 KiB

int luaucl_compress(lua_State* L) {
    size_t input_len;
    const char* input = luaL_checklstring(L, 1, &input_len);
    const int DEFAULT_LEVEL = 777;
    int level = luaL_optinteger(L, 2, DEFAULT_LEVEL);
    if (level == DEFAULT_LEVEL) {
        // UPX's default level, see upx(1)
        if (input_len < MIN_LARGE_SIZE) {
            level = 8;
        } else {
            level = 7;
        }
    }
    luaL_argcheck(L, level >= 1, 2, "level must be >= 1");
    luaL_argcheck(L, level <= 10, 2, "level must be <= 10");
    // see UCL's README
    ucl_uint output_len = input_len + (input_len / 8) + 256;
    void* output = lua_newuserdata(L, output_len);
    ucl_progress_callback_p callback = NULL;
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
        const char* text = ucl_name_code(status);
        return luaL_error(L, "UCL error occurred: %s", text);
    }
    lua_pushlstring(L, output, output_len);
    return 1;
}

int luaucl_decompress(lua_State* L) {
    size_t input_len;
    const char* input = luaL_checklstring(L, 1, &input_len);
    // output_len is max possible output
    ucl_uint output_len = luaL_checkinteger(L, 2);
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
        const char* text = ucl_name_code(status);
        return luaL_error(L, "UCL error occurred: %s", text);
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
