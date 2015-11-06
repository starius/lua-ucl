package = 'lua-ucl'

version = 'scm-0'

source = {
    url = 'git://github.com/starius/lua-ucl.git'
}

description = {
    summary = 'Lua bindings to UCL, data compression library',
    homepage = 'https://github.com/starius/lua-ucl',
    maintainer = 'Boris Nagaev <bnagaev@gmail.com>',
    license = 'GPL 2'
}

dependencies = {
    'lua >= 5.1'
}

external_dependencies = {
    UCL = {
        header = "ucl/ucl.h",
        library = "ucl",
    },
}

build = {
    type = 'builtin',
    modules = {
        ['ucl'] = {
            sources = {'src/lua-ucl/lua-ucl.c'},
            incdirs = {'$(UCL_INCDIR)'},
            libdirs = {'$(UCL_LIBDIR)'},
            libraries = {'ucl'},
        }
    }
}
