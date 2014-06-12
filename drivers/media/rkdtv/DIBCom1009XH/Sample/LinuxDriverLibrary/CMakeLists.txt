cmake_minimum_required(VERSION 2.6)

macro(ADD_DIR)
    foreach(d ${ARGN})
        include_directories(${d})
        file(GLOB this ${d}/*.c)
        set(SRCS ${this} ${SRCS})
    endforeach()
endmacro()

if("${SDK8_ROOT}" STREQUAL "")
    set(SDK8_ROOT "../../")
endif()

set(SDK8_ROOT "${CMAKE_SOURCE_DIR}/${SDK8_ROOT}")

ADD_DIR(${SDK8_ROOT}/Driver/Target)
ADD_DIR(${SDK8_ROOT}/Driver/Common)
ADD_DIR(${SDK8_ROOT}/Driver/Common/Firefly)
ADD_DIR(${SDK8_ROOT}/Driver/Common/Dragonfly)
ADD_DIR(${SDK8_ROOT}/Driver/Common/Debug)
ADD_DIR(${SDK8_ROOT}/Bridge/Target)
ADD_DIR(${SDK8_ROOT}/Bridge/Common)
ADD_DIR(${SDK8_ROOT}/Bridge/Common/Firefly)
ADD_DIR(${SDK8_ROOT}/Bridge/Common/Dragonfly)
ADD_DIR(${SDK8_ROOT}/Bridge/Common/Messages)
ADD_DIR(${SDK8_ROOT}/User/Common)
ADD_DIR(${SDK8_ROOT}/Board)
ADD_DIR(${SDK8_ROOT}/Driver/Common/Messages)

# avoid duplicated files in library
list(REMOVE_ITEM SRCS
    ${SDK8_ROOT}/Bridge/Common/Messages/DibMsgFields.c
    ${SDK8_ROOT}/Bridge/Common/Messages/DibMsgGeneric.c
    ${SDK8_ROOT}/Driver/Common/Messages/DibMsgData.c
)


set(BRIDGE LinuxUserDib07x0)
set(DRIVER Driver2Bridge)
add_definitions(-DDRIVER_AND_BRIDGE_MERGED)

option(ENABLE_SHARED "Enable Shared Library")

add_custom_command(OUTPUT bootstrapped
    COMMAND ${CMAKE_SOURCE_DIR}/bootstrap.sh ARGS LinuxUserDib07x0 Driver2Bridge ${CMAKE_SOURCE_DIR}
    COMMENT "Bootstrapping for '${BRIDGE}' and '${DRIVER}'")

# Get SDK version
file(STRINGS "${SDK8_ROOT}/Driver/Common/DibDriverConstants.h" version REGEX "DIB_SET_VER\\([0-9]+")
string(REGEX MATCH "[0-9 ]+,[0-9 ]+,[0-9 ]+" VERSION_STRING ${version})
string(REGEX MATCH "[0-9]+" VERSION_MAJOR ${version})
string(REGEX REPLACE " " "" VERSION_STRING ${VERSION_STRING})
string(REGEX REPLACE , . VERSION_STRING ${VERSION_STRING})

if (ENABLE_SHARED)
    add_library(DibDriverLibrary SHARED ${SRCS} bootstrapped)
    set_target_properties(DibDriverLibrary PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})
else(ENABLE_SHARED)
    add_library(DibDriverLibrary STATIC ${SRCS} bootstrapped)
endif(ENABLE_SHARED)

install(TARGETS DibDriverLibrary LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
#install(DIRECTORY "${SDK8_ROOT}/User/Common" DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "CVS" EXCLUDE)

