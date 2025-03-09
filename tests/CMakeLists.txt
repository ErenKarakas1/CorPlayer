enable_testing()

include(GoogleTest)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/database)
include_directories(${CMAKE_SOURCE_DIR}/src/library)
include_directories(${CMAKE_SOURCE_DIR}/src/models)
include_directories(${CMAKE_SOURCE_DIR}/src/taglib)

qt_add_resources(TEST_RESOURCES testdata/audio.qrc)

add_executable(corplayer_tests
    main_test.cpp

    asynccoverprovider_test.cpp
    library_test.cpp
    mediaplayerwrapper_test.cpp
    playlistproxymodel_test.cpp
    tagreader_test.cpp

    testutils.cpp
    ${TEST_RESOURCES}
)

target_compile_features(corplayer_tests PRIVATE cxx_std_20)
target_compile_options(corplayer_tests PRIVATE -O2 -Wall -Wextra -Wpedantic -Wstrict-overflow=2 -Wdisabled-optimization -Wsign-conversion
    -Wsign-promo -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wdouble-promotion
    -Wuseless-cast -Wformat=2 -Wimplicit-fallthrough -Wpessimizing-move -Wredundant-move -Wnoexcept -Wctor-dtor-privacy -Wswitch-default
    -Wconversion)

target_link_libraries(corplayer_tests
    PRIVATE
    CorPlayerCore
    Qt6::Test
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(corplayer_tests)

option(BUILD_INDIVIDUAL_TESTS ON)

if (BUILD_INDIVIDUAL_TESTS)
    macro(add_individual_test filename)
        add_executable(${filename}
            ${filename}.cpp
            testutils.cpp
            ${TEST_RESOURCES}
        )

        target_compile_features(${filename} PRIVATE cxx_std_20)
        target_compile_options(${filename} PRIVATE -O2 -Wall -Wextra -Wpedantic -Wstrict-overflow=2 -Wdisabled-optimization -Wsign-conversion
            -Wsign-promo -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wdouble-promotion
            -Wuseless-cast -Wformat=2 -Wimplicit-fallthrough -Wpessimizing-move -Wredundant-move -Wnoexcept -Wctor-dtor-privacy -Wswitch-default
            -Wconversion)

        target_link_libraries(${filename}
            PRIVATE
            CorPlayerCore
            Qt6::Test
            GTest::gtest_main
        )

        gtest_discover_tests(${filename})
    endmacro()

    add_individual_test(asynccoverprovider_test)
    add_individual_test(library_test)
    add_individual_test(mediaplayerwrapper_test)
    add_individual_test(playlistproxymodel_test)
    add_individual_test(tagreader_test)
endif ()
