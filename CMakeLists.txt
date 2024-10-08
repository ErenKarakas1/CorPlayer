cmake_minimum_required(VERSION 3.30)
project(CorPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_LINK_DEPENDS_NO_SHARED ON)

set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

find_package(Qt6 6.7 REQUIRED COMPONENTS Quick Multimedia Widgets Gui Sql)
find_package(ZLIB REQUIRED)
find_package(TagLib 2.0.2 REQUIRED)

qt_standard_project_setup(REQUIRES 6.7)

option(USE_PCH "Use precompiled headers" OFF)
option(BUILD_TESTS "Build tests" OFF)

add_subdirectory(src)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wunused
    -Wshadow
    -Wundef
    -Wuninitialized
    -Wredundant-decls
    -Wcast-align
    -Winit-self
    -Wmissing-include-dirs
    -Wstrict-overflow=2
    -Wunused-parameter
    -Wdisabled-optimization
    -Woverloaded-virtual
    -Wold-style-cast
    -Winvalid-pch
    -Wnon-virtual-dtor
    -Wconversion
    -Wsign-conversion
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wnull-dereference
    -Wdouble-promotion
    -Wuseless-cast
    -Wformat=2
    -Wimplicit-fallthrough
    -fpch-preprocess
)

add_definitions(
    -DQT_STRICT_ITERATORS
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_ASCII_CAST_WARNINGS
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DQT_NO_FOREACH
    -DQT_NO_KEYWORDS
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_DISABLE_DEPRECATED_UP_TO=0x060700
    -DQT_ENABLE_STRICT_MODE_UP_TO=0x060700
)

set(PROJECT_SOURCES ${PROJECT_SOURCES} src/main.cpp)

qt_add_resources(PROJECT_SOURCES
    resources/icons.qrc
)

qt_add_executable(CorPlayer
    ${PROJECT_SOURCES}
)

target_link_libraries(CorPlayer
    PRIVATE
    CorPlayerCore
    Qt6::Quick
    Qt6::Widgets
)

if (USE_PCH)
    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/empty_pch.cpp
        CONTENT "/* empty file for precompiled headers */")
    target_sources(CorPlayer
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/empty_pch.cpp)
    target_precompile_headers(CorPlayer
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)
    set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
endif()

if (BUILD_TESTS)
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(CorPlayer PROPERTIES
    #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CorPlayer
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CorPlayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
