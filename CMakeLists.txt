set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -fno-strict-aliasing -Wextra")
cmake_minimum_required(VERSION 2.8)


find_package( PkgConfig REQUIRED )
pkg_check_modules( GLESv2 REQUIRED glesv2 )
pkg_check_modules( EGL REQUIRED egl )
message(STATUS "GLESv2 ldflags: ${GLESv2_LDFLAGS}")
message(STATUS "EGL ldflags: ${EGL_LDFLAGS}")

add_library(eglapp STATIC
  eglapp.c
  util.c
)
target_link_libraries(eglapp
  mirclient
  ${EGL_LDFLAGS}
  ${GLESv2_LDFLAGS}
  "-lm"
)
add_executable(hello_mir_gles2
  eglflash.c
)
target_link_libraries(hello_mir_gles2
  eglapp
)

file(COPY ${PROJECT_SOURCE_DIR}/hello1.tga DESTINATION "${PROJECT_BINARY_DIR}")
file(COPY ${PROJECT_SOURCE_DIR}/hello2.tga DESTINATION "${PROJECT_BINARY_DIR}")
file(COPY ${PROJECT_SOURCE_DIR}/hello-gl.v.glsl DESTINATION "${PROJECT_BINARY_DIR}")
file(COPY ${PROJECT_SOURCE_DIR}/hello-gl.f.glsl DESTINATION "${PROJECT_BINARY_DIR}")

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${GLESv2_INCLUDE_DIRS}
)

