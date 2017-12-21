cmake_minimum_required(VERSION 3.6)
project(CryptoPart)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.c crypto.h belt.c belt.h crypto.c serpent.c
        serpent.h ec.c ec.h mpi.c mpi.h tzi.h tzi_bign.c tzi_lib.c tzi_lib.h tzi_u128.h tzi_extra.h wrap.c)
add_executable(CryptoPart ${SOURCE_FILES})