
# FindCreait.cmake - Find the Creait static library
# Looks for libcreait.a and associated headers
# Provides: 
#   CREAIT_FOUND
#   CREAIT_INCLUDE_DIRS
#   CREAIT_LIBRARIES

# Allow user to specify a custom installation prefix
set(_CREAIT_HINT_DIRS
    $ENV{HOME}/.local
    $ENV{HOME}/.local/RevEngAI/Radare2
    $ENV{HOME}/.local/RevEngAI/Rizin
    /usr/local
    /usr
)

# Locate the header
find_path(CREAIT_INCLUDE_DIR
    NAMES "Rea/Sys.h" "Reai/Config.h" "Reai/Api.h" "Reai/File.h" "Reai/Log.h" "Reai/Types.h"
    HINTS ${_CREAIT_HINT_DIRS}
    PATH_SUFFIXES include
)

# Locate the static library
find_library(CREAIT_LIBRARY
    NAMES reai
    HINTS ${_CREAIT_HINT_DIRS}
    PATH_SUFFIXES lib
)

# Set output variables
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Creait
    REQUIRED_VARS CREAIT_LIBRARY CREAIT_INCLUDE_DIR
    VERSION_VAR CREAIT_VERSION
)

if(CREAIT_FOUND)
    set(CREAIT_LIBRARIES ${CREAIT_LIBRARY})
    set(CREAIT_INCLUDE_DIRS ${CREAIT_INCLUDE_DIR})
endif()

mark_as_advanced(CREAIT_INCLUDE_DIR CREAIT_LIBRARY)
