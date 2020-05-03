cmake_minimum_required(VERSION 3.1)

set(MY_RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${MY_TARGET} PUBLIC
  ${MY_RELATIVE_PATH}/bsp.c
  )

