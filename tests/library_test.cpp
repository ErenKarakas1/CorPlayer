#include "testutils.h"

#include "library/library.hpp"

#include <QSignalSpy>

class LibraryTest : public GlobalTest {
protected:
    std::unique_ptr<Library> m_library;

    void SetUp() override {
        GlobalTest::SetUp();
        m_library = std::make_unique<Library>();
        m_library->initialize(dbConnectionPool());
    }

    void TearDown() override {
        m_library.reset();
    }
};

TEST_F(LibraryTest, AddTrackFromUrl) {
    const QUrl fileUrl = AudioFile::create();

    QSignalSpy trackAddedSpy(m_library.get(), SIGNAL(trackAdded(quint64, const Metadata::TrackFields&)));

    const quint64 trackId = m_library->addTrackFromUrl(fileUrl);

    ASSERT_NE(trackId, 0U);
    ASSERT_EQ(trackAddedSpy.count(), 1);

    const Metadata::TrackFields track = m_library->getTrackById(trackId);
    ASSERT_TRUE(track.isValid());
    ASSERT_EQ(track.get(Metadata::Fields::ResourceUrl).toUrl(), fileUrl);

    // Adding the same track again should return the same ID without emitting a signal
    trackAddedSpy.clear();
    const quint64 duplicateTrackId = m_library->addTrackFromUrl(fileUrl);
    ASSERT_EQ(duplicateTrackId, trackId);
    ASSERT_EQ(trackAddedSpy.count(), 0);
}

TEST_F(LibraryTest, AddTracksFromUrls) {
    const QUrl fileUrl1 = AudioFile::create();
    const QUrl fileUrl2 = AudioFile::create();

    const QList<QUrl> urls = {fileUrl1, fileUrl2};
    QList<quint64> trackIds = m_library->addTracksFromUrls(urls);

    ASSERT_EQ(trackIds.size(), 2);
    ASSERT_NE(trackIds[0], 0U);
    ASSERT_NE(trackIds[1], 0U);

    const Metadata::TrackFields track1 = m_library->getTrackById(trackIds[0]);
    const Metadata::TrackFields track2 = m_library->getTrackById(trackIds[1]);

    ASSERT_TRUE(track1.isValid());
    ASSERT_TRUE(track2.isValid());

    ASSERT_EQ(track1.get(Metadata::Fields::ResourceUrl).toUrl(), fileUrl1);
    ASSERT_EQ(track2.get(Metadata::Fields::ResourceUrl).toUrl(), fileUrl2);
}

TEST_F(LibraryTest, UpdateTrack) {
    const QUrl fileUrl = AudioFile::create();
    const quint64 trackId = m_library->addTrackFromUrl(fileUrl);
    ASSERT_NE(trackId, 0U);

    Metadata::TrackFields track = m_library->getTrackById(trackId);
    ASSERT_TRUE(track.isValid());

    QSignalSpy trackModifiedSpy(m_library.get(), SIGNAL(trackModified(quint64, const Metadata::TrackFields&)));

    const QString newTitle = QStringLiteral("Updated Title");
    track.insert(Metadata::Fields::Title, newTitle);
    m_library->updateTrack(track);

    ASSERT_EQ(trackModifiedSpy.count(), 1);

    const Metadata::TrackFields updatedTrack = m_library->getTrackById(trackId);
    ASSERT_EQ(updatedTrack.get(Metadata::Fields::Title).toString(), newTitle);
}

TEST_F(LibraryTest, RemoveTrack) {
    const QUrl fileUrl = AudioFile::create();
    const quint64 trackId = m_library->addTrackFromUrl(fileUrl);
    ASSERT_NE(trackId, 0U);

    QSignalSpy trackRemovedSpy(m_library.get(), SIGNAL(trackRemoved(quint64)));

    m_library->removeTrack(trackId);

    ASSERT_EQ(trackRemovedSpy.count(), 1);

    const Metadata::TrackFields removedTrack = m_library->getTrackById(trackId);
    ASSERT_FALSE(removedTrack.isValid());
}

TEST_F(LibraryTest, CreatePlaylistFromUrls) {
    const QUrl fileUrl1 = AudioFile::create();
    const QUrl fileUrl2 = AudioFile::create();
    const QList<QUrl> urls = {fileUrl1, fileUrl2};

    QSignalSpy playlistModifiedSpy(m_library.get(), SIGNAL(playlistModified(quint64)));

    const QString playlistName = QStringLiteral("Test Playlist");
    const quint64 playlistId = m_library->createPlaylistFromUrls(playlistName, urls);
    ASSERT_NE(playlistId, 0U);
    ASSERT_EQ(playlistModifiedSpy.count(), 1);

    const QList<quint64> trackIds = m_library->playlistDatabase().getPlaylistTracks(playlistId);
    ASSERT_EQ(trackIds.size(), 2);

    const auto playlist = m_library->playlistDatabase().getPlaylist(playlistId);
    ASSERT_EQ(playlist.id, playlistId);
    ASSERT_EQ(playlist.name, playlistName);
    ASSERT_EQ(playlist.trackIds.size(), 2);
}

TEST_F(LibraryTest, RenamePlaylist) {
    const QUrl fileUrl = AudioFile::create();
    const QString originalName = QStringLiteral("Original Name");
    const quint64 playlistId = m_library->createPlaylistFromUrls(originalName, {fileUrl});
    ASSERT_NE(playlistId, 0U);

    QSignalSpy playlistModifiedSpy(m_library.get(), SIGNAL(playlistModified(quint64)));

    // Rename the playlist
    const QString newName = QStringLiteral("New Name");
    m_library->renamePlaylist(playlistId, newName);

    ASSERT_EQ(playlistModifiedSpy.count(), 1);

    const auto playlist = m_library->playlistDatabase().getPlaylist(playlistId);
    ASSERT_EQ(playlist.id, playlistId);
    ASSERT_EQ(playlist.name, newName);
}

TEST_F(LibraryTest, RemovePlaylist) {
    const QUrl fileUrl = AudioFile::create();
    const quint64 playlistId = m_library->createPlaylistFromUrls(QStringLiteral("Test Playlist"), {fileUrl});
    ASSERT_NE(playlistId, 0U);

    QSignalSpy playlistModifiedSpy(m_library.get(), SIGNAL(playlistModified(quint64)));

    m_library->removePlaylist(playlistId);

    ASSERT_EQ(playlistModifiedSpy.count(), 1);

    const auto playlist = m_library->playlistDatabase().getPlaylist(playlistId);
    ASSERT_EQ(playlist.id, 0U);
    ASSERT_EQ(playlist.name, QString());
    ASSERT_TRUE(playlist.trackIds.isEmpty());
}
