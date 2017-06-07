IF (NOT MODULE_DVBS_Tuner)
    return()
ENDIF (NOT MODULE_DVBS_Tuner)

list (APPEND IPTV_CXX_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/DvbModule.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/DvbUtils.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/DvbEnum.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/DvbSection.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/DvbDemuxFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbChannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbEpg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Transponder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbJsCall.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbAssertions.cpp
)

include (${CMAKE_CURRENT_LIST_DIR}/Parser/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Scan/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Frontend/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Descriptors/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Store/CMakeLists.txt)

LIST (APPEND INCLUDES ${CMAKE_CURRENT_LIST_DIR})
