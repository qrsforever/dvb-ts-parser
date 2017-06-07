
list (APPEND IPTV_CXX_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/DvbDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbTimeOffsetDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbServiceDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbParentalRatingDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbShortEventDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbExtendedEventDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbSatelliteDescriptor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DvbLinkageDescriptor.cpp
)

LIST (APPEND INCLUDES ${CMAKE_CURRENT_LIST_DIR})
