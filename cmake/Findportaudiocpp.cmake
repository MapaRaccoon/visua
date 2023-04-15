# - Try to find Portaudiocpp
# Once done this will define
#
#  PORTAUDIO_CPP_FOUND - system has Portaudiocpp
#  PORTAUDIO_CPP_INCLUDE_DIRS - the Portaudiocpp include directory
#  PORTAUDIO_CPP_LIBRARIES - Link these to use Portaudiocpp
#  PORTAUDIO_CPP_DEFINITIONS - Compiler switches required for using Portaudiocpp
#  PORTAUDIO_CPP_VERSION - Portaudiocpp version
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (PORTAUDIO_CPP_LIBRARIES AND PORTAUDIO_CPP_INCLUDE_DIRS)
  # in cache already
  set(PORTAUDIO_CPP_FOUND TRUE)
else (PORTAUDIO_CPP_LIBRARIES AND PORTAUDIO_CPP_INCLUDE_DIRS)
  if (NOT WIN32)
   include(FindPkgConfig)
   pkg_check_modules(PORTAUDIO_CPP2 portaudiocpp-2.0)
  endif (NOT WIN32)

  if (PORTAUDIO_CPP2_FOUND)
    set(PORTAUDIO_CPP_INCLUDE_DIRS
      ${PORTAUDIO_CPP2_INCLUDE_DIRS}
    )
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(PORTAUDIO_CPP_LIBRARIES "${PORTAUDIO_CPP2_LIBRARY_DIRS}/lib${PORTAUDIO_CPP2_LIBRARIES}.dylib")
    else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(PORTAUDIO_CPP_LIBRARIES
        ${PORTAUDIO_CPP2_LIBRARIES}
      )
    endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(PORTAUDIO_CPP_VERSION
      19
    )
    set(PORTAUDIO_CPP_FOUND TRUE)
  else (PORTAUDIO_CPP2_FOUND)
    find_path(PORTAUDIO_CPP_INCLUDE_DIR
      NAMES
      #PortAudioCpp.hxx
        portaudiocpp
      PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
    )

    find_library(PORTAUDIO_CPP_LIBRARY
      NAMES
        portaudiocpp
      PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )

    find_path(PORTAUDIO_CPP_LIBRARY_DIR
      NAMES
        portaudiocpp
      PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )

    set(PORTAUDIO_CPP_INCLUDE_DIRS
      ${PORTAUDIO_CPP_INCLUDE_DIR}
    )
    set(PORTAUDIO_CPP_LIBRARIES
      ${PORTAUDIO_CPP_LIBRARY}
    )

    set(PORTAUDIO_CPP_LIBRARY_DIRS
      ${PORTAUDIO_CPP_LIBRARY_DIR}
    )

    set(PORTAUDIO_CPP_VERSION
      18
    )

    if (PORTAUDIO_CPP_INCLUDE_DIRS AND PORTAUDIO_CPP_LIBRARIES)
       set(PORTAUDIO_CPP_FOUND TRUE)
    endif (PORTAUDIO_CPP_INCLUDE_DIRS AND PORTAUDIO_CPP_LIBRARIES)

    if (PORTAUDIO_CPP_FOUND)
      if (NOT Portaudiocpp_FIND_QUIETLY)
        message(STATUS "Found Portaudiocpp: ${PORTAUDIO_CPP_LIBRARIES}")
      endif (NOT Portaudiocpp_FIND_QUIETLY)
    else (PORTAUDIO_CPP_FOUND)
      if (Portaudiocpp_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find Portaudiocpp")
      endif (Portaudiocpp_FIND_REQUIRED)
    endif (PORTAUDIO_CPP_FOUND)
  endif (PORTAUDIO_CPP2_FOUND)


  # show the PORTAUDIO_CPP_INCLUDE_DIRS and PORTAUDIO_CPP_LIBRARIES variables only in the advanced view
  mark_as_advanced(PORTAUDIO_CPP_INCLUDE_DIRS PORTAUDIO_CPP_LIBRARIES)

endif (PORTAUDIO_CPP_LIBRARIES AND PORTAUDIO_CPP_INCLUDE_DIRS)
