// lua-ucl, Lua bindings to UCL, data compression library
// Copyright (C) 2015 Boris Nagaev
// See the LICENSE file for terms of use.

// UCL is a portable lossless data compression library
// written in ANSI C
// See http://www.oberhumer.com/opensource/ucl/

#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include <ucl/ucl.h>
#ifdef UCL_USE_ASM
#  include <ucl/ucl_asm.h>
#endif

// Lua compatibility

#if LUA_VERSION_NUM == 501
#define luaucl_setfuncs(L, funcs) luaL_register(L, NULL, funcs)
#else
#define luaucl_setfuncs(L, funcs) luaL_setfuncs(L, funcs, 0)
#endif

// errors registry

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

int luaucl_error(lua_State* L, int code) {
    const char* text = ucl_name_code(code);
    return luaL_error(L, "UCL error occurred: %s", text);
}

// compression functions registry

// ucl_compress_t from UCL has wrong signature
typedef int
(__UCL_CDECL *luaucl_compress_t)(
    const ucl_bytep src,
    ucl_uint src_len,
    ucl_bytep dst,
    ucl_uintp dst_len,
    ucl_progress_callback_p cb,
    int level,
    const struct ucl_compress_config_p conf,
    ucl_uintp result
);

typedef struct ucl_compress_Reg {
    const char* name;
    luaucl_compress_t func;
} ucl_compress_Reg;

static ucl_compress_Reg ucl_compress_funcs[] = {
    {"nrv2b", ucl_nrv2b_99_compress},
    {"nrv2d", ucl_nrv2d_99_compress},
    {"nrv2e", ucl_nrv2e_99_compress},
    {}
};

luaucl_compress_t ucl_compress_by_name(
    lua_State* L,
    const char* name
) {
    ucl_compress_Reg* i;
    for (i = ucl_compress_funcs; i->name; ++i) {
        if (strcmp(i->name, name) == 0) {
            return i->func;
        }
    }
    return NULL;
}

// compression functions registry

typedef struct ucl_decompress_Reg {
    const char* name;
    ucl_decompress_t func;
} ucl_decompress_Reg;

// *_le16 and *_le32 do not decompress TODO
static ucl_decompress_Reg ucl_decompress_funcs[] = {
#ifdef UCL_USE_ASM
    {"nrv2b", ucl_nrv2b_decompress_asm_safe_8},
    {"nrv2d", ucl_nrv2d_decompress_asm_safe_8},
    {"nrv2e", ucl_nrv2e_decompress_asm_safe_8},
#else
    {"nrv2b", ucl_nrv2b_decompress_safe_8},
    {"nrv2d", ucl_nrv2d_decompress_safe_8},
    {"nrv2e", ucl_nrv2e_decompress_safe_8},
#endif
    {}
};

ucl_decompress_t ucl_decompress_by_name(
    lua_State* L,
    const char* name
) {
    ucl_decompress_Reg* i;
    for (i = ucl_decompress_funcs; i->name; ++i) {
        if (strcmp(i->name, name) == 0) {
            return i->func;
        }
    }
    return NULL;
}

// high level functions

const int MIN_LARGE_SIZE = 512 * 1024;  // 512 KiB
const char* DEFAULT_METHOD = "nrv2d";

int luaucl_compress(lua_State* L) {
    size_t input_len;
    const char* input = luaL_checklstring(L, 1, &input_len);
    const int DEFAULT_LEVEL = 777;
    int level = luaL_optinteger(L, 2, DEFAULT_LEVEL);
    const char* method = luaL_optstring(L, 3, DEFAULT_METHOD);
    luaucl_compress_t func = ucl_compress_by_name(L, method);
    if (!func) {
        return luaL_error(L, "Unknown method: %s", method);
    }
    if (level == DEFAULT_LEVEL) {
        // UPX's default level, see upx(1)
        if (input_len < MIN_LARGE_SIZE) {
            level = 8;
        } else {
            level = 7;
        }
    }
    // see UCL's README
    ucl_uint output_len = input_len + (input_len / 8) + 256;
    void* output = lua_newuserdata(L, output_len);
    ucl_progress_callback_p callback = NULL;
    struct ucl_compress_config_p conf = NULL;
    ucl_uintp result = NULL;
    int status = func(
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
        return luaucl_error(L, status);
    }
    lua_pushlstring(L, output, output_len);
    return 1;
}

int luaucl_decompress(lua_State* L) {
    size_t input_len;
    const char* input = luaL_checklstring(L, 1, &input_len);
    // output_len is max possible output
    ucl_uint output_len = luaL_checkinteger(L, 2);
    const char* method = luaL_optstring(L, 3, DEFAULT_METHOD);
    ucl_decompress_t func = ucl_decompress_by_name(L, method);
    if (!func) {
        return luaL_error(L, "Unknown method: %s", method);
    }
    void* output = lua_newuserdata(L, output_len);
    ucl_voidp wrkmem = NULL;
    int status = func(
        input,
        input_len,
        output,
        &output_len,
        wrkmem
    );
    if (status != UCL_E_OK) {
        return luaucl_error(L, status);
    }
    lua_pushlstring(L, output, output_len);
    return 1;
}

int luaucl_version(lua_State* L) {
    lua_pushinteger(L, ucl_version());
    lua_pushstring(L, ucl_version_string());
    lua_pushstring(L, ucl_version_date());
    return 3;
}

luaL_Reg luaucl_functions[] = {
    {"compress", luaucl_compress},
    {"decompress", luaucl_decompress},
    {"version", luaucl_version},
    {}
};

int luaopen_ucl(lua_State* L) {
    int status = ucl_init();
    if (status != UCL_E_OK) {
        return luaucl_error(L, status);
    }
    int nf = sizeof(luaucl_functions) / sizeof(luaL_Reg) - 1;
    lua_createtable(L, 0, nf);
    luaucl_setfuncs(L, luaucl_functions);
    return 1;
}
