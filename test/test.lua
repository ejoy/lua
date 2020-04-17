local sharetable = require "sharetable"
local multivm = require "multivm"

local vm = multivm.new()

local testp, tablep = vm:dostring [[
	local sharetable = require "sharetable"
	_G.test = { "Hello", "World" }
	local testp = sharetable.mark_shared(_G.test)
	local tablep = sharetable.mark_shared(_G.table)
	return testp, tablep

]]

local shared_test = sharetable.clone(testp)
local shared_table = sharetable.clone(tablep)

-- Use table.unpack in vm to unpack _G.test in vm.
print(shared_table.unpack(shared_test))