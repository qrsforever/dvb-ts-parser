
list (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/DvbScan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbAntennaSetup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbBlindScan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbScanTransponder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbScanChannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbScanEpg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbScanOta.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbScanDateTime.cpp
)

LIST (APPEND INCLUDES ${CMAKE_CURRENT_LIST_DIR})
