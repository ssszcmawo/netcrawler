cmake_minimum_required(VERSION 3.10)

project(WebCrawler VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
    ${PROJECT_SOURCE_DIR}/include 
)

file(GLOB SLOGGER_SRC
      ${PROJECT_SOURCE_DIR}/thirdparty/slogger/src/*.c
    )

add_library(slogger STATIC ${SLOGGER_SRC})
target_include_directories(slogger PUBLIC ${PROJECT_SOURCE_DIR}/thirdparty/slogger/src)
add_executable(WebCrawler
    main.cpp
    src/HttpsClient.cpp
    src/HtmlParser.cpp 
    src/Utils.cpp
)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

target_include_directories(WebCrawler PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty/fast-cpp-csv-parser
    ${PROJECT_SOURCE_DIR}/thirdparty/slogger/src
    ${CURL_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
)

target_link_libraries(WebCrawler PRIVATE
    slogger
    ${CURL_LIBRARIES}
    ${LIBXML2_LIBRARIES}
)

set_target_properties(WebCrawler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_compile_definitions(WebCrawler PRIVATE 
    $<$<CONFIG:DEBUG>:DEBUG_BUILD>
    $<$<CONFIG:RELEASE>:RELEASE_BUILD>
) 
