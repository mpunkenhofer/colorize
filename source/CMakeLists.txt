include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include)

add_library(Utils
        utils.cpp        ${CMAKE_SOURCE_DIR}/include/utils.h)

add_executable(colors main.cpp)

target_link_libraries(colors
        Utils
        pthread
        stdc++fs
        ${OpenCV_LIBS}
        )