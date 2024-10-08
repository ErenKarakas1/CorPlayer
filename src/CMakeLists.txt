set(PROJECT_SOURCES
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
    trackprogresswatchdog.cpp
    trackprogresswatchdog.h
    trackswatchdog.cpp
    trackswatchdog.h
    trackmetadatamanager.cpp
    trackmetadatamanager.h
    filescanner.cpp
    filescanner.h
    cormanager.cpp
    cormanager.h

    playlist/trackplaylist.cpp
    playlist/trackplaylist.h
    playlist/trackplaylistproxymodel.cpp
    playlist/trackplaylistproxymodel.h
    playlist/playlistparser.cpp
    playlist/playlistparser.h

    taglib/tagreader.cpp
    taglib/tagreader.h
    taglib/tracktags.h
    taglib/tracktags.cpp

    database/basedatabase.cpp
    database/basedatabase.h
    database/cordatabase.cpp
    database/cordatabase.h
    database/databasemanager.cpp
    database/databasemanager.h
    database/dbconnectionpool.cpp
    database/dbconnectionpool.h
    database/trackdatabase.cpp
    database/trackdatabase.h
    database/dbconnection.cpp
    database/dbconnection.h
    database/dbschema.cpp
    database/dbschema.h
    database/sqlquery.cpp
    database/sqlquery.h
    database/sqltransaction.cpp
    database/sqltransaction.h
)

add_library(CorPlayerCore
    SHARED
    ${PROJECT_SOURCES}
)

qt_add_qml_module(CorPlayerCore
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
)

target_include_directories(CorPlayerCore
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/database
    ${CMAKE_CURRENT_SOURCE_DIR}/playlist
    ${CMAKE_CURRENT_SOURCE_DIR}/taglib
)

target_link_libraries(CorPlayerCore
    PUBLIC
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Widgets
    Qt6::Gui
    Qt6::Sql
    TagLib::TagLib
)
