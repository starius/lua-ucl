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

    it("gets a version and a date", function()
        local ucl = require 'ucl'
        local ver_int, ver_str, ver_date = ucl.version()
        assert.equal("number", type(ver_int))
        assert.equal("string", type(ver_str))
        assert.equal("string", type(ver_date))
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
        local de = ucl.decompress(compressed, size, "nrv2b")
        local expected = string.char(0x00):rep(size)
        assert.equal(expected, de)
    end)

    it("compress throws error on bad compression level",
    function()
        local ucl = require 'ucl'
        assert.has_error(function()
            ucl.compress('text', 0)
        end)
        assert.has_error(function()
            ucl.compress('text', 11)
        end)
        assert.has_error(function()
            ucl.compress('text', 'foo')
        end)
        assert.has_not_error(function()
            ucl.compress('text', '5')
        end)
    end)

    it("decompress throws error if no output size", function()
        local ucl = require 'ucl'
        local compressed = ucl.compress('text')
        assert.has_error(function()
            ucl.decompress(compressed)
        end)
    end)

    it("decompress throws error if output size is too small",
    function()
        local ucl = require 'ucl'
        local compressed = ucl.compress('text')
        assert.has_error(function()
            ucl.decompress(compressed, 1)
        end)
    end)

    it("decompress throws error if method mismatches",
    function()
        local ucl = require 'ucl'
        local compressed = ucl.compress('text', 5, "nrv2b")
        assert.has_error(function()
            ucl.decompress(compressed, 100, "nrv2d")
        end)
    end)

    it("uses methods", function()
        local ucl = require 'ucl'
        for _, method in ipairs {"nrv2b", "nrv2d", "nrv2e"} do
            local comp = ucl.compress('text', 5, method)
            local decomp = ucl.decompress(comp, 4, method)
            assert.equal('text', decomp)
        end
    end)

    it("throws on unknown (de)compression method", function()
        local ucl = require 'ucl'
        assert.has_error(function()
            ucl.compress('text', 1, "foo")
        end)
        assert.has_error(function()
            ucl.compress('text', 1, {})
        end)
        assert.has_error(function()
            ucl.decompress('text', 100, "foo")
        end)
        assert.has_error(function()
            ucl.decompress('text', 100, {})
        end)
    end)

end)
