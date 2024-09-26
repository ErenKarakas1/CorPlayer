enable_testing()

include(GoogleTest)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/playlist)
include_directories(${CMAKE_SOURCE_DIR}/src/database)
include_directories(${CMAKE_SOURCE_DIR}/src/taglib)

qt_add_resources(TEST_RESOURCES testdata/audio.qrc)

add_executable(tagreader_test
    tagreader_test.cpp
    testutils.cpp
    ${TEST_RESOURCES}
)

target_link_libraries(tagreader_test
    PRIVATE
    CorPlayerCore
    GTest::gtest_main
)

gtest_discover_tests(tagreader_test)
