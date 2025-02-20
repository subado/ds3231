cmake_minimum_required(VERSION 3.30)

file(GLOB ds3231_sources src/*.c)

idf_component_register(
  SRCS ${ds3231_sources}
  INCLUDE_DIRS "include"
  REQUIRES "driver"
)
