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

    it("compresses large string", function()
        local ucl = require 'ucl'
        local orig = string.rep('ATGC', 1000000)  -- 4 MiB
        local compressed = ucl.compress(orig)
        assert.truthy(#compressed < #orig)
        local decompressed = ucl.decompress(compressed, #orig)
        assert.equal(decompressed, orig)
    end)

    it("uses custom compression level", function()
        local ucl = require 'ucl'
        local orig = string.rep('A', 1000)
        for level = 1, 9 do
            local compressed = ucl.compress(orig, level)
            local decomp = ucl.decompress(compressed, #orig)
            assert.equal(decomp, orig)
        end
    end)

end)
