cmake_minimum_required(VERSION 3.13)

project(gocompiler C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "-Wall -Wno-unused-function -Wpedantic")

add_custom_target(FLEX COMMAND flex ${PROJECT_SOURCE_DIR}/gocompiler.l)

add_custom_target(YACC COMMAND yacc -d ${PROJECT_SOURCE_DIR}/gocompiler.y)

add_library(AST gocompiler_ast.h gocompiler_ast.c)

add_library(SYMTAB gocompiler_symtab.h gocompiler_symtab.c)

add_library(STRUCTURES gocompiler_structures.h gocompiler_structures.c)

add_library(SEMANTICS gocompiler_semantics.h gocompiler_semantics.c)

add_library(CODE_GEN gocompiler_code_gen.h gocompiler_code_gen.c)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(gocompiler ${PROJECT_SOURCE_DIR}/lex.yy.c ${PROJECT_SOURCE_DIR}/y.tab.c)

target_link_libraries(gocompiler AST SYMTAB STRUCTURES SEMANTICS CODE_GEN)

add_dependencies(gocompiler FLEX YACC AST SYMTAB STRUCTURES SEMANTICS CODE_GEN)
