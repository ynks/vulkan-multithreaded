#!/usr/bin/env lua

local function execute(cmd)
	print("> " .. cmd)
	local result = os.execute(cmd)
	if result ~= 0 and result ~= true then
		print("Error executing: " .. cmd)
		os.exit(1)
	end
end

execute("slangc shaders/triangle.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o slang.spv");
