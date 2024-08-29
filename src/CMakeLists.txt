cmake_minimum_required(VERSION 3.16)

project(CorPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Multimedia Widgets Gui)

qt_standard_project_setup(REQUIRES 6.5)

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
        trackplaylist.cpp
        trackplaylist.h
        trackplaylistproxymodel.cpp
        trackplaylistproxymodel.h
        trackprogresswatchdog.cpp
        trackprogresswatchdog.h
        trackswatchdog.cpp
        trackswatchdog.h
        trackmetadatamanager.cpp
        trackmetadatamanager.h
        filescanner.cpp
        filescanner.h
        qml/components/fonts.qrc
)

qt_add_qml_module(appCorPlayer
        URI CorPlayer
        VERSION 1.0
        QML_FILES
        qml/Main.qml
        qml/PlaybackControl.qml
        qml/PlaybackControlBar.qml
        qml/components/IconRoundButton.qml
        qml/components/StepSlider.qml
        qml/components/DurationSlider.qml
        qml/components/fonts/Fonts.qml
        qml/components/fonts/qmldir
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
)

## Create a local directory for fonts in the build/install directory
#set(FONT_INSTALL_DIR "${CMAKE_BINARY_DIR}/fonts")
#file(MAKE_DIRECTORY "${FONT_INSTALL_DIR}")
#file(COPY ${CMAKE_SOURCE_DIR}/qml/fonts/ DESTINATION ${FONT_INSTALL_DIR})
#install(DIRECTORY ${CMAKE_BINARY_DIR}/fonts/
#        DESTINATION ${CMAKE_INSTALL_PREFIX}/fonts
#        FILES_MATCHING PATTERN "*.otf")

include(GNUInstallDirs)
install(TARGETS appCorPlayer
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
