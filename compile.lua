#!/usr/bin/env lua

local function execute(cmd)
	print("> " .. cmd)
	local result = os.execute(cmd)
	if result ~= 0 and result ~= true then
		print("Error executing: " .. cmd)
		os.exit(1)
	end
end

-- Create build directory if it doesn't exist
execute("mkdir -p build")

-- Configure with CMake using Ninja generator for C++ modules support
print("=== Generating files ===")
execute("cd build && CC=clang CXX=clang++ cmake -G Ninja ..")

-- Build the project
print("=== Compiling project ===")
execute("cd build && ninja")

print("\nBuild completed successfully :3")
