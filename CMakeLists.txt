cmake_minimum_required(VERSION 3.25)

project(Clang_tools)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LLVM_DIR "/Users/kanon/Code/llvm-project/build/lib/cmake/llvm")
set(Clang_DIR "/Users/kanon/Code/llvm-project/build/lib/cmake/clang")

list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)
include(HandleLLVMOptions)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED)

include_directories("${LLVM_INCLUDE_DIR}"
"${CLANG_INCLUDE_DIRS}")
include_directories("/Users/kanon/Code/llvm-project/llvm/include")
add_definitions("${LLVM_DEFINITIONS}")
link_directories("${LLVM_LIBRARY_DIR}")
message(STATUS "MESSAGE => ${LLVM_VERSION}")
message(STATUS "MESSAGE => ${CLANG_INCLUDE_DIRS}")
message(STATUS "MESSAGE => ${LLVM_INCLUDE_DIR}")
message(STATUS "MESSAGE => ${LLVM_DEFINITIONS}")
message(STATUS "MESSAGE => ${LLVM_LIBRARY_DIR}")

add_llvm_library(NamingPlugin MODULE src/main.cpp
                       PLUGIN_TOOL clang)

add_llvm_library(VisitPlugin MODULE src/visitor_main.cpp
                       PLUGIN_TOOL clang)

add_llvm_library(StructVisitPlugin MODULE src/struct_visitor.cpp
                      PLUGIN_TOOL clang)

add_llvm_library(MacroPlugin MODULE src/addmacro.cpp
                      PLUGIN_TOOL clang)
# target_link_libraries(MacroPlugin PUBLIC 
#     clangAST clangFrontend clangToolingCore LLVMSupport
# )