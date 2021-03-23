cmake_minimum_required(VERSION 3.17)
project(ccnc VERSION 0.0.1 DESCRIPTION "C Computer Numerical Control")

set(CMAKE_C_STANDARD 11)

include_directories("src")

add_library(ccnc SHARED
    ccnc.c    
)

set_target_properties(ccnc PROPERTIES PUBLIC_HEADER ccnc.h)