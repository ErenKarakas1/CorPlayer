cmake_minimum_required(VERSION 3.16)

project(CorPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Multimedia Widgets Gui)
find_package(ZLIB)
find_package(TagLib 2.0.2 REQUIRED)
#find_package(GTest)
#find_library(GMOCK_LIBRARY gmock)
#
#if (GTest_FOUND AND GMOCK_LIBRARY)
#    add_subdirectory(tests)
#endif()

qt_standard_project_setup(REQUIRES 6.5)

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
        -DQT_DISABLE_DEPRECATED_BEFORE=0x060200
)

include_directories(
        ${CMAKE_SOURCE_DIR}/playlist
)

qt_add_executable(appCorPlayer
        main.cpp
        corplayer.cpp
        corplayer.h
        activetrackmanager.cpp
        activetrackmanager.h
        mediaplayerwrapper.cpp
        mediaplayerwrapper.h
        metadatafields.cpp
        metadatafields.h
        playermanager.cpp
        playermanager.h
        playerutils.cpp
        playerutils.h
        playlist/trackplaylist.cpp
        playlist/trackplaylist.h
        playlist/trackplaylistproxymodel.cpp
        playlist/trackplaylistproxymodel.h
        playlist/playlistparser.cpp
        playlist/playlistparser.h
        trackprogresswatchdog.cpp
        trackprogresswatchdog.h
        trackswatchdog.cpp
        trackswatchdog.h
        trackmetadatamanager.cpp
        trackmetadatamanager.h
        filescanner.cpp
        filescanner.h
#        taglib/tagreader.cpp
#        taglib/tagreader.h
#        taglib/tracktags.h
#        taglib/tracktags.cpp
)

qt_add_qml_module(appCorPlayer
        URI CorPlayer
        VERSION 1.0
        QML_FILES
        qml/Main.qml
        qml/PlaybackControl.qml
        qml/PlaybackControlBar.qml
        qml/components/StepSlider.qml
        qml/components/DurationSlider.qml
        qml/components/MaterialButton.qml
        qml/components/VolumeControl.qml
        qml/components/TrackInfo.qml
        RESOURCES
        ../icons/icons.qrc
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appCorPlayer PROPERTIES
        #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appCorPlayer
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCorPlayer
        PRIVATE
        Qt6::Quick
        Qt6::Multimedia
        Qt6::Widgets
        Qt6::Gui
        TagLib::TagLib
)

include(GNUInstallDirs)
install(TARGETS appCorPlayer
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
