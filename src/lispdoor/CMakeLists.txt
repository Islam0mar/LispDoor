cmake_minimum_required(VERSION 3.1)

set(MY_RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${MY_TARGET} PUBLIC
  ${MY_RELATIVE_PATH}/objects.c
  ${MY_RELATIVE_PATH}/memorylayout.c
  ${MY_RELATIVE_PATH}/read.c
  ${MY_RELATIVE_PATH}/gc.c
  ${MY_RELATIVE_PATH}/utils.c
  ${MY_RELATIVE_PATH}/symboltree.c
  ${MY_RELATIVE_PATH}/print.c
  ${MY_RELATIVE_PATH}/eval.c
  ${MY_RELATIVE_PATH}/functions.c
  )

