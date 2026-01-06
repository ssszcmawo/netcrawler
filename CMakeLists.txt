cmake_minimum_required(VERSION 3.10)


project(WebCrawler VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(WebCrawler
    main.cpp
    src/HttpsClient.cpp
)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

target_include_directories(WebCrawler PRIVATE ${CURL_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS})
target_link_libraries(WebCrawler PRIVATE ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES})

set_target_properties(WebCrawler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

