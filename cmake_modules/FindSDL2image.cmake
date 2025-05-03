# This module defines:
# SDL2_IMAGE_LIBRARY, the name of the SDL2_image library to link against
# SDL2_IMAGE_FOUND, if false, do not try to link to SDL2_image
# SDL2_IMAGE_INCLUDE_DIR, where to find SDL_image.h
#
# Additional Note: If you see an empty SDL2_IMAGE_LIBRARY_TEMP in your configuration
# and no SDL2_IMAGE_LIBRARY, it means CMake did not find your SDL2_image library
# (SDL2_image.dll, libSDL2_image.so, SDL2_image.framework, etc).
# Set SDL2_IMAGE_LIBRARY_TEMP to point to your SDL2_image library, and configure again.

SET(SDL2_IMAGE_SEARCH_PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
        ${SDL2_IMAGE_PATH}
)

# Locate SDL_image.h
FIND_PATH(SDL2_IMAGE_INCLUDE_DIR SDL_image.h
        HINTS
        $ENV{SDL2DIR}
        PATH_SUFFIXES include/SDL2 include
        PATHS ${SDL2_IMAGE_SEARCH_PATHS}
)

# Determine the appropriate library suffix for 32-bit or 64-bit systems
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PATH_SUFFIXES lib64 lib/x64 lib)
else()
    set(PATH_SUFFIXES lib/x86 lib)
endif()

# Locate the SDL2_image library
FIND_LIBRARY(SDL2_IMAGE_LIBRARY_TEMP
        NAMES SDL2_image
        HINTS
        $ENV{SDL2DIR}
        PATH_SUFFIXES ${PATH_SUFFIXES}
        PATHS ${SDL2_IMAGE_SEARCH_PATHS}
)

# Handle OS-specific dependencies and options
IF(APPLE)
    SET(SDL2_IMAGE_LIBRARY_TEMP ${SDL2_IMAGE_LIBRARY_TEMP} "-framework Cocoa")
ENDIF(APPLE)

# For MinGW, add -mwindows and mingw32
IF(MINGW)
    SET(MINGW32_LIBRARY mingw32 "-mwindows" CACHE STRING "mwindows for MinGW")
    SET(SDL2_IMAGE_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL2_IMAGE_LIBRARY_TEMP})
ENDIF(MINGW)

# Define the final SDL2_IMAGE_LIBRARY variable
IF(SDL2_IMAGE_LIBRARY_TEMP)
    SET(SDL2_IMAGE_LIBRARY ${SDL2_IMAGE_LIBRARY_TEMP} CACHE STRING "Where the SDL2_image library can be found")
    SET(SDL2_IMAGE_LIBRARY_TEMP "${SDL2_IMAGE_LIBRARY_TEMP}" CACHE INTERNAL "")
ENDIF(SDL2_IMAGE_LIBRARY_TEMP)

# Standard handling for CMake find modules
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_image REQUIRED_VARS SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)
