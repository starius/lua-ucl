-- lua-ucl, Lua bindings to UCL, data compression library
-- Copyright (C) 2015 Boris Nagaev
-- See the LICENSE file for terms of use.

-- UCL is a portable lossless data compression library
-- written in ANSI C
-- See http://www.oberhumer.com/opensource/ucl/

describe("lua-ucl", function()

    it("loads module 'ucl'", function()
        local ucl = require 'ucl'
    end)

    it("compresses a string", function()
        local ucl = require 'ucl'
        local orig = string.rep('A', 1000)
        local compressed = ucl.compress(orig)
        assert.truthy(#compressed < #orig)
        local decompressed = ucl.decompress(compressed, #orig)
        assert.equal(decompressed, orig)
    end)

end)
