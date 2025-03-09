set(PROJECT_SOURCES
    activetrackmanager.cpp
    activetrackmanager.h
    asynccoverprovider.cpp
    asynccoverprovider.h
    corplayer.cpp
    corplayer.h
    mediaplayerwrapper.cpp
    mediaplayerwrapper.h
    metadata.cpp
    metadata.hpp
    playermanager.cpp
    playermanager.h
    playerutils.cpp
    playerutils.hpp
    trackprogresswatchdog.cpp
    trackprogresswatchdog.h

    database/basedatabase.cpp
    database/basedatabase.h
    database/cordatabase.cpp
    database/cordatabase.h
    database/databasemanager.cpp
    database/databasemanager.h
    database/dbconnection.cpp
    database/dbconnection.h
    database/dbconnectionpool.cpp
    database/dbconnectionpool.h
    database/dbschema.cpp
    database/dbschema.h
    database/playlistdatabase.cpp
    database/playlistdatabase.h
    database/sqlquery.cpp
    database/sqlquery.h
    database/sqltransaction.cpp
    database/sqltransaction.h
    database/trackdatabase.cpp
    database/trackdatabase.h

    library/filescanner.cpp
    library/filescanner.h
    library/library.cpp
    library/library.hpp
    library/playlistparser.cpp
    library/playlistparser.h

    models/playlistcollectionmodel.cpp
    models/playlistcollectionmodel.hpp
    models/playlistmodel.cpp
    models/playlistmodel.hpp
    models/playlistproxymodel.cpp
    models/playlistproxymodel.hpp
    models/trackcollectionmodel.cpp
    models/trackcollectionmodel.hpp

    taglib/tagreader.cpp
    taglib/tagreader.h
    taglib/tracktags.cpp
    taglib/tracktags.h
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
    qml/TrackBrowser.qml

    qml/dialogs/DeletePlaylistDialog.qml
    qml/dialogs/NewPlaylistDialog.qml
    qml/dialogs/RenamePlaylistDialog.qml

    qml/components/DurationSlider.qml
    qml/components/IconButton.qml
    qml/components/IconButtonWithBackground.qml
    qml/components/ImageWithFallback.qml
    qml/components/StepSlider.qml
    qml/components/TrackInfo.qml
    qml/components/VolumeControl.qml
)

target_include_directories(CorPlayerCore
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/database
    ${CMAKE_CURRENT_SOURCE_DIR}/library
    ${CMAKE_CURRENT_SOURCE_DIR}/models
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
