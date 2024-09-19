set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

message("sdk :'$ENV{STEAMRT_SDK_ROOT}'")
if(NOT IS_DIRECTORY "$ENV{STEAMRT_SDK_ROOT}")
  message(FATAL_ERROR "STEAMRT_SDK_ROOT environment variable not set or the directory it points to is not valid. Please define it to point to the steamrt sniper SDK root.")
endif()

set(triplet "x86_64-linux-gnu")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")

# Tell cmake where the sdk root directory is
set(CMAKE_SYSROOT $ENV{STEAMRT_SDK_ROOT})
set(CMAKE_FIND_ROOT_PATH $ENV{STEAMRT_SDK_ROOT})
set(CMAKE_LIBRARY_ARCHITECTURE ${triplet})
set(CMAKE_C_COMPILER_TARGET ${triplet})
set(CMAKE_CXX_COMPILER_TARGET ${triplet})

#Setup pkgconf
#set(ENV{PKG_CONFIG_DIR} "")
#set(ENV{PKG_CONFIG_LIBDIR} "$ENV{STEAMRT_SDK_ROOT}\\usr\\lib\\x86_64-linux-gnu\\pkgconfig")
#file(TO_CMAKE_PATH ${CMAKE_SYSROOT} CMAKE_SYSROOT_CMAKE)
#set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT_CMAKE})

# Configure cmake to only look inside the steamrt sdk, and not the host system.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(FIND_LIBRARY_USE_LIB64_PATHS ON)

set(Vulkan_dxc_EXECUTABLE "D:/mr/cloud/vcpkg/src/packages/directx-dxc_x64-windows/tools/directx-dxc/dxc.exe")
set(PKG_CONFIG_EXECUTABLE "D:/mr/cloud/vcpkg/src/packages/pkgconf_x64-windows/tools/pkgconf/pkgconf.exe")
set(Vulkan_GLSLC_EXECUTABLE "C:/Program Files (x86)/Android/AndroidNDK/android-ndk-r23c/shader-tools/windows-x86_64/glslc.exe")

set(ENV{PKG_CONFIG_PATH} "$ENV{STEAMRT_SDK_ROOT}/usr/lib/pkgconfig;$ENV{STEAMRT_SDK_ROOT}/usr/lib/x86_64-linux-gnu/pkgconfig;$ENV{STEAMRT_SDK_ROOT}/usr/share/pkgconfig")