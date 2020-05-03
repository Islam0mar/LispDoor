cmake_minimum_required(VERSION 3.1)

set(MY_RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${MY_TARGET} PUBLIC
  ${MY_RELATIVE_PATH}/main.c
  ${MY_RELATIVE_PATH}/startup_stm32f103_gnu.c
  )
target_include_directories(${MY_TARGET} PUBLIC
  ${MY_RELATIVE_PATH}
  )
add_subdirectory(lispdoor)
add_subdirectory(hal)

