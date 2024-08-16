cmake_minimum_required(VERSION 3.16)

project(mplayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Multimedia Widgets Gui)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(appmplayer
    main.cpp
        trackplaylist.cpp
        trackplaylist.h
        metadatafields.cpp
        metadatafields.h
        playerutils.cpp
        playerutils.h
        corplayer.cpp
        corplayer.h
)

qt_add_qml_module(appmplayer
    URI mplayer
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/PlaybackControl.qml
        qml/TrackControl.qml
    SOURCES
        mediaplayerwrapper.h mediaplayerwrapper.cpp
    RESOURCES
        fonts.qrc
        QML_FILES qml/components/IconRoundButton.qml
        SOURCES activetrackmanager.h activetrackmanager.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appmplayer PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appmplayer
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appmplayer
    PRIVATE
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Widgets
    Qt6::Gui
)

include(GNUInstallDirs)
install(TARGETS appmplayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
