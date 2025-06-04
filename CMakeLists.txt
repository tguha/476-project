cmake_minimum_required(VERSION 3.10...3.23)
include("${CMAKE_SOURCE_DIR}/FindGfxLibs.cmake")

# Name of the project
project(LibraryGame)

# Enforce use of C++17
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD 17)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/ext/*/*.cpp" "${CMAKE_SOURCE_DIR}/ext/glad/src/*.c" "${CMAKE_SOURCE_DIR}/particles/*.cpp")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "particles/*.h")
file(GLOB_RECURSE GLSL "resources/*.glsl")

include_directories("ext")
include_directories("ext/glad/include")

# Set up Assimp external dependency
include(ExternalProject)

# Set Assimp version to use
set(ASSIMP_VERSION "v5.4.3")
# Set ext directory for storing dependencies
set(ASSIMP_INSTALL_DIR "${CMAKE_SOURCE_DIR}/ext/assimp")

# FreeType Git tag and install path
set(FREETYPE_VERSION "VER-2-13-2")
set(FREETYPE_INSTALL_DIR "${CMAKE_SOURCE_DIR}/ext/freetype")


# Configure Assimp build options
set(ASSIMP_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${ASSIMP_INSTALL_DIR}
    -DASSIMP_BUILD_TESTS=OFF
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
    -DBUILD_SHARED_LIBS=ON
    -DASSIMP_NO_EXPORT=OFF
)

# configure FreeType build options
set(FREETYPE_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${FREETYPE_INSTALL_DIR}
    -DBUILD_SHARED_LIBS=ON
    -DFT_DISABLE_HARFBUZZ=ON
    -DFT_DISABLE_BROTLI=ON
    -DFT_DISABLE_ZLIB=ON
)

# Check if Assimp is already installed at our desired location
if(EXISTS "${ASSIMP_INSTALL_DIR}/include/assimp/Importer.hpp")
    message(STATUS "Found previously built Assimp at ${ASSIMP_INSTALL_DIR}")
    set(ASSIMP_ALREADY_BUILT TRUE)
else()
    set(ASSIMP_ALREADY_BUILT FALSE)

    # Add Assimp as an external project
    ExternalProject_Add(
        assimp_external
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG ${ASSIMP_VERSION}
        CMAKE_ARGS ${ASSIMP_CMAKE_ARGS}
        PREFIX "${CMAKE_BINARY_DIR}/assimp-build"
        UPDATE_COMMAND ""
    )
endif()

# Check if already built
if(EXISTS "${FREETYPE_INSTALL_DIR}/include/freetype2/freetype/freetype.h")
    message(STATUS "Found previously built FreeType at ${FREETYPE_INSTALL_DIR}")
    set(FREETYPE_ALREADY_BUILT TRUE)
else()
    set(FREETYPE_ALREADY_BUILT FALSE)

    ExternalProject_Add(
    freetype_external
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG ${FREETYPE_VERSION}
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${FREETYPE_INSTALL_DIR}
        -DBUILD_SHARED_LIBS=ON
        -DFT_DISABLE_HARFBUZZ=ON
        -DFT_DISABLE_BROTLI=ON
        -DFT_DISABLE_ZLIB=ON
    CONFIGURE_HANDLED_BY_BUILD ON
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Debug &&
                  ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --install . --config Debug &&
                    ${CMAKE_COMMAND} --install . --config Release
    PREFIX "${CMAKE_BINARY_DIR}/freetype-build"
    UPDATE_COMMAND ""
)
endif()

# Set include and library directories for Assimp
set(ASSIMP_INCLUDE_DIRS "${ASSIMP_INSTALL_DIR}/include")

# Include and library paths
set(FREETYPE_INCLUDE_DIRS "${FREETYPE_INSTALL_DIR}/include" "${FREETYPE_INSTALL_DIR}/include/freetype2")

# Set the appropriate library name based on platform
if(WIN32)
    set(ASSIMP_LIBRARIES "${ASSIMP_INSTALL_DIR}/lib/assimp-vc143-mtd.lib")
    set(ASSIMP_DLL "${ASSIMP_INSTALL_DIR}/bin/assimp-vc143-mtd.dll")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(FREETYPE_LIBRARIES "${FREETYPE_INSTALL_DIR}/lib/freetyped.lib")
        set(FREETYPE_DLL "${FREETYPE_INSTALL_DIR}/bin/freetyped.dll")
    else()
        set(FREETYPE_LIBRARIES "${FREETYPE_INSTALL_DIR}/lib/freetype.lib")
        set(FREETYPE_DLL "${FREETYPE_INSTALL_DIR}/bin/freetype.dll")
    endif()
elseif(APPLE)
    set(ASSIMP_LIBRARIES "${ASSIMP_INSTALL_DIR}/lib/libassimp.dylib")
    set(FREETYPE_LIBRARIES "${FREETYPE_INSTALL_DIR}/lib/libfreetype.dylib")
else()
    set(ASSIMP_LIBRARIES "${ASSIMP_INSTALL_DIR}/lib/libassimp.so")
    set(FREETYPE_LIBRARIES "${FREETYPE_INSTALL_DIR}/lib/libfreetype.so")
endif()

# Create a directory for includes (needed before build)
file(MAKE_DIRECTORY ${ASSIMP_INCLUDE_DIRS})

# Add include directories for Assimp
include_directories(${ASSIMP_INCLUDE_DIRS})

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

# If we need to build Assimp, add dependency
if(NOT ASSIMP_ALREADY_BUILT)
    add_dependencies(${CMAKE_PROJECT_NAME} assimp_external)
endif()

# Link with Assimp library
target_link_libraries(${CMAKE_PROJECT_NAME} ${ASSIMP_LIBRARIES})

# Add include directories
include_directories(${FREETYPE_INCLUDE_DIRS})

# Add dependency if needed
if(NOT FREETYPE_ALREADY_BUILT)
    add_dependencies(${CMAKE_PROJECT_NAME} freetype_external)
endif()

# Link FreeType
target_link_libraries(${CMAKE_PROJECT_NAME} ${FREETYPE_LIBRARIES})

# Helper function included from FindGfxLibs.cmake
findGLFW3(${CMAKE_PROJECT_NAME})
findGLM(${CMAKE_PROJECT_NAME})

# OS specific options and libraries
if(NOT WIN32)
  message(STATUS "Adding GCC style compiler flags")
  add_compile_options("-Wall")
  add_compile_options("-pedantic")
  add_compile_options("-Werror=return-type")

  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    # Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL" "dl")
  endif()
else()
  # Link OpenGL on Windows
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)

  # Copy DLL to output directory
  add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${ASSIMP_DLL}"
      $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
  # Copy FreeType DLL to output directory
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FREETYPE_DLL}"
            $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)
endif()

# Add a target to clean the Assimp installation if needed
add_custom_target(clean_assimp
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${ASSIMP_INSTALL_DIR}
    COMMENT "Cleaning Assimp installation directory"
)

add_custom_target(clean_freetype
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${FREETYPE_INSTALL_DIR}
    COMMENT "Cleaning FreeType installation directory"
)