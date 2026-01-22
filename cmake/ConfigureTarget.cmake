cmake_minimum_required(VERSION 3.5)

# Configure a target to set the correct properties for 32/64-bit builds
function(configure_target target wordsize)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${wordsize} EQUAL 32)
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-m32")
      set_target_properties(${target} PROPERTIES LINK_FLAGS "-m32")
    else()
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-m64")
      set_target_properties(${target} PROPERTIES LINK_FLAGS "-m64")
    endif()
    target_link_libraries(${target} dl)

    if(WITH_GUI)
      target_link_libraries(${target} x11)
    endif()

  elseif(APPLE)
    if(${wordsize} EQUAL 32)
      set_target_properties(${target} PROPERTIES OSX_ARCHITECTURES "i386")
    else()
      set_target_properties(${target} PROPERTIES OSX_ARCHITECTURES "x86_64")
    endif()

  elseif(MSVC)
    if(${wordsize} EQUAL 32)
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/DWIN32=1")
    else()
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/DWIN64=1")
    endif()
  endif()

  target_compile_definitions(${target} PUBLIC PLATFORM_BITS=${wordsize})
  
  # Set C++14 standard for VST3 support (if needed)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_target_properties(${target} PROPERTIES
      CXX_STANDARD 14
      CXX_STANDARD_REQUIRED ON
    )
  endif()
endfunction()
