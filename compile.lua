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
execute("cd build && CC=clang CXX=clang++ cmake -G Ninja -DENABLE_CPP20_MODULE=OFF ..")

-- Build the project
execute("cd build && ninja")

print("\nBuild completed successfully!")
print("Executable location: build/bin/vulkan_app")
