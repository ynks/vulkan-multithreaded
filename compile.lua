#!/usr/bin/env lua

local function execute(cmd)
    print("Executing: " .. cmd)
    local result = os.execute(cmd)
    if result ~= 0 and result ~= true then
        print("Error executing: " .. cmd)
        os.exit(1)
    end
end

-- Create build directory if it doesn't exist
execute("mkdir -p build")

-- Configure with CMake using Ninja generator for C++ modules support
print("echo \"=== Generating files ===\"")
print("> cd build && cc=clang cxx=clang++ cmake -g ninja -denable_cpp20_module=off ..")
execute("cd build && CC=clang CXX=clang++ cmake -G Ninja -DENABLE_CPP20_MODULE=OFF ..")

-- Build the project
print("echo \"=== Compiling project ===\"")
print("> cd build && ninja")
execute("cd build && ninja")

print("\nBuild completed successfully :3")
