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

    it("decompresses large string with small output size",
    function()
        local ucl = require 'ucl'
        local orig = string.rep('ATGC', 1000000)  -- 4 MiB
        local compressed = ucl.compress(orig)
        assert.has_error(function()
            ucl.decompress(compressed, 1)
        end)
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

    local function fromHex(str)
        return (str:gsub('..', function(cc)
            return string.char(tonumber(cc, 16))
        end))
    end

    it("decompresses known text (ucl/examples/simple.c)",
    function()
        local ucl = require 'ucl'
        local compressed_hex = '9200' ..
            string.rep('AAA8C9555464AAAA325555192', 31) ..
            'AAA8C9555464AAAA325555080000000000240FF'
        local compressed = fromHex(compressed_hex)
        local size = 262144
        local decompressed = ucl.decompress(compressed, size)
        local expected = string.char(0x00):rep(size)
        assert.equal(expected, decompressed)
    end)

end)
