ADD_DIR(${SDK8_ROOT}/Driver/${DRIVER})
ADD_DIR(${SDK8_ROOT}/Driver/Common)
ADD_DIR(${SDK8_ROOT}/Driver/Common/Firefly)
ADD_DIR(${SDK8_ROOT}/Driver/Common/Dragonfly)
if(ENABLE_INTERNAL_MONITORING)
ADD_DIR(${SDK8_ROOT}/Driver/Common/Debug)
endif()
ADD_DIR(${SDK8_ROOT}/Bridge/Common)
ADD_DIR(${SDK8_ROOT}/Bridge/${BRIDGE})
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
set(SRCS gitref.c ${SRCS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# compilation in ANSI C
set(CMAKE_C_FLAGS "-ansi")


if(DRIVER STREQUAL "Driver2Bridge")
add_definitions(-DDRIVER_AND_BRIDGE_MERGED)
endif()

option(ENABLE_SHARED "Enable Shared Library" ON)

if(ENABLE_SHARED)
    option(ENABLE_DTA_BATCH_WRITER "Enable API-to-dta-batch writer (LD_PRELOAD)" OFF)
    if(ENABLE_DTA_BATCH_WRITER)
        add_library(DibExpApiToBatchWriter SHARED ${SDK8_ROOT}/Sample/LinuxDriverLibrary/DibExtApiToBatchWriter.c)
        target_link_libraries(DibExpApiToBatchWriter dl)
    endif()
endif()

# generate git ref
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GetGitRevisionDescription)
git_describe_current(GIT_REFSPEC --tags --dirty --long)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gitref.h.in" "${CMAKE_BINARY_DIR}/gitref.h" @ONLY)
include_directories(${CMAKE_BINARY_DIR})

if (ENABLE_SHARED)
    add_library(DibDriverLibrary SHARED ${SRCS})
    set_target_properties(DibDriverLibrary PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${VERSION_MAJOR})
else(ENABLE_SHARED)
    add_library(DibDriverLibrary STATIC ${SRCS})
endif(ENABLE_SHARED)

install(TARGETS DibDriverLibrary LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
#install(DIRECTORY "${SDK8_ROOT}/User/Common" DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "CVS" EXCLUDE)

