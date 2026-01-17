cmake_minimum_required(VERSION 3.10)

project(WebCrawler VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SLOGGER_SRC
    ${PROJECT_SOURCE_DIR}/thirdparty/slogger/src/*.c
)

add_library(slogger STATIC ${SLOGGER_SRC})
target_include_directories(slogger PUBLIC
    ${PROJECT_SOURCE_DIR}/thirdparty/slogger/src
)

add_subdirectory(thirdparty/json)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

add_executable(WebCrawler
    main.cpp
    src/HttpsClient.cpp
    src/HtmlParser.cpp
    src/Utils.cpp
    src/CSV.cpp
    src/ProductXPathConfig.cpp
)

target_include_directories(WebCrawler PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/thirdparty/fast-cpp-csv-parser
    ${PROJECT_SOURCE_DIR}/thirdparty/slogger/src
    ${CURL_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
)

target_link_libraries(WebCrawler PRIVATE
    slogger
    ${CURL_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    nlohmann_json::nlohmann_json
)

target_compile_options(WebCrawler PRIVATE
    $<$<CONFIG:Debug>:-g;-Wall;-Wextra>
)

set_target_properties(WebCrawler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_compile_definitions(WebCrawler PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
)
