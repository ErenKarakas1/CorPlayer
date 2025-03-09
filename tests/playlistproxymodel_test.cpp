#include "testutils.h"

#include "library/library.hpp"
#include "models/playlistmodel.hpp"
#include "models/playlistproxymodel.hpp"

#include <QSignalSpy>

class PlaylistProxyModelTest : public GlobalTest {
protected:
    std::unique_ptr<Library> m_library;
    std::unique_ptr<PlaylistModel> m_playlistModel;
    std::unique_ptr<PlaylistProxyModel> m_proxyModel;

    void SetUp() override {
        GlobalTest::SetUp();

        m_library = std::make_unique<Library>();
        m_library->initialize(dbConnectionPool());

        m_playlistModel = std::make_unique<PlaylistModel>(m_library.get());
        m_proxyModel = std::make_unique<PlaylistProxyModel>(m_library.get());
        m_proxyModel->setPlaylistModel(m_playlistModel.get());
    }

    void TearDown() override {
        m_proxyModel.reset();
        m_playlistModel.reset();
        m_library.reset();
    }

    [[nodiscard]] QList<Metadata::TrackFields> createTestEntries(const int count) const {
        QList<Metadata::TrackFields> entries;

        for (int i = 0; i < count; ++i) {
            Metadata::TrackFields track;
            track.insert(Metadata::Fields::Title, QStringLiteral("Track %1").arg(i + 1));
            track.insert(Metadata::Fields::Artist, QStringLiteral("Artist %1").arg(i + 1));
            track.insert(Metadata::Fields::Album, QStringLiteral("Album %1").arg(i + 1));

            auto fileUrl = AudioFile::create(track);
            const quint64 trackId = m_library->addTrackFromUrl(fileUrl);

            track = m_library->getTrackById(trackId);

            entries.emplace_back(std::move(track));
        }

        return entries;
    }
};

TEST_F(PlaylistProxyModelTest, InitialState) {
    ASSERT_EQ(m_proxyModel->rowCount(), 0);
    ASSERT_EQ(m_proxyModel->repeatMode(), PlaylistProxyModel::RepeatMode::None);
    ASSERT_EQ(m_proxyModel->shuffleMode(), PlaylistProxyModel::ShuffleMode::NoShuffle);
}

TEST_F(PlaylistProxyModelTest, EnqueueReplacePlaylist) {
    auto entries = createTestEntries(3);

    m_proxyModel->enqueue(entries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    ASSERT_EQ(m_proxyModel->rowCount(), 3);

    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        auto index = m_proxyModel->index(i, 0);
        auto title = m_proxyModel->data(index, Metadata::Fields::Title).toString();
        ASSERT_EQ(title, QStringLiteral("Track %1").arg(i + 1));
    }

    auto newEntries = createTestEntries(2);
    for (auto& entry : newEntries) {
        const QString title = QStringLiteral("New ") + entry.get(Metadata::Fields::Title).toString();
        entry.insert(Metadata::Fields::Title, title);
    }

    m_proxyModel->enqueue(newEntries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    ASSERT_EQ(m_proxyModel->rowCount(), 2);

    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        auto index = m_proxyModel->index(i, 0);
        auto title = m_proxyModel->data(index, Metadata::Fields::Title).toString();
        ASSERT_EQ(title, QStringLiteral("New Track %1").arg(i + 1));
    }
}

TEST_F(PlaylistProxyModelTest, EnqueueAppendToPlaylist) {
    auto entries = createTestEntries(2);
    m_proxyModel->enqueue(entries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    ASSERT_EQ(m_proxyModel->rowCount(), 2);

    auto newEntries = createTestEntries(2);
    for (auto& entry : newEntries) {
        const QString title = QStringLiteral("New ") + entry.get(Metadata::Fields::Title).toString();
        entry.insert(Metadata::Fields::Title, title);
    }

    m_proxyModel->enqueue(newEntries, PlayerUtils::PlaylistEnqueueMode::AppendPlaylist, PlayerUtils::DoNotTriggerPlay);

    ASSERT_EQ(m_proxyModel->rowCount(), 4);

    QStringList expectedTitles = {QStringLiteral("Track 1"), QStringLiteral("Track 2"), QStringLiteral("New Track 1"),
                                  QStringLiteral("New Track 2")};
    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        auto index = m_proxyModel->index(i, 0);
        auto title = m_proxyModel->data(index, Metadata::Fields::Title).toString();
        ASSERT_EQ(title, expectedTitles[i]);
    }
}

TEST_F(PlaylistProxyModelTest, EnqueueAfterCurrentTrack) {
    const auto entries = createTestEntries(3);
    m_proxyModel->enqueue(entries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    ASSERT_EQ(m_proxyModel->rowCount(), 3);

    m_proxyModel->switchTo(1);

    auto newEntries = createTestEntries(2);
    for (auto& entry : newEntries) {
        const QString title = QStringLiteral("New ") + entry.get(Metadata::Fields::Title).toString();
        entry.insert(Metadata::Fields::Title,  title);
    }

    m_proxyModel->enqueue(newEntries, PlayerUtils::PlaylistEnqueueMode::AfterCurrentTrack,
                          PlayerUtils::DoNotTriggerPlay);

    ASSERT_EQ(m_proxyModel->rowCount(), 5);

    QStringList expectedTitles = {QStringLiteral("Track 1"), QStringLiteral("Track 2"), QStringLiteral("New Track 1"),
                                  QStringLiteral("New Track 2"), QStringLiteral("Track 3")};
    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        auto index = m_proxyModel->index(i, 0);
        auto title = m_proxyModel->data(index, Metadata::Fields::Title).toString();
        ASSERT_EQ(title, expectedTitles[i]);
    }
}

TEST_F(PlaylistProxyModelTest, SwitchTo) {
    auto entries = createTestEntries(3);
    m_proxyModel->enqueue(entries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    QSignalSpy currentTrackChangedSpy(m_proxyModel.get(), SIGNAL(currentTrackChanged(QPersistentModelIndex)));
    QSignalSpy previousTrackChangedSpy(m_proxyModel.get(), SIGNAL(previousTrackChanged(QPersistentModelIndex)));
    QSignalSpy nextTrackChangedSpy(m_proxyModel.get(), SIGNAL(nextTrackChanged(QPersistentModelIndex)));

    m_proxyModel->switchTo(1);

    ASSERT_EQ(currentTrackChangedSpy.count(), 1);
    ASSERT_EQ(previousTrackChangedSpy.count(), 1);
    ASSERT_EQ(nextTrackChangedSpy.count(), 1);

    ASSERT_TRUE(m_proxyModel->currentTrack().isValid());
    ASSERT_EQ(m_proxyModel->currentTrack(), m_proxyModel->index(1, 0));

    ASSERT_TRUE(m_proxyModel->previousTrack().isValid());
    ASSERT_EQ(m_proxyModel->previousTrack(), m_proxyModel->index(0, 0));

    ASSERT_TRUE(m_proxyModel->nextTrack().isValid());
    ASSERT_EQ(m_proxyModel->nextTrack(), m_proxyModel->index(2, 0));
}

TEST_F(PlaylistProxyModelTest, SkipNextTrack) {
    auto entries = createTestEntries(3);
    m_proxyModel->enqueue(entries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    m_proxyModel->switchTo(0);

    QSignalSpy currentTrackChangedSpy(m_proxyModel.get(), SIGNAL(currentTrackChanged(QPersistentModelIndex)));

    m_proxyModel->skipNextTrack();

    ASSERT_EQ(currentTrackChangedSpy.count(), 1);

    ASSERT_TRUE(m_proxyModel->currentTrack().isValid());
    ASSERT_EQ(m_proxyModel->currentTrack(), m_proxyModel->index(1, 0));

    currentTrackChangedSpy.clear();
    m_proxyModel->skipNextTrack();

    ASSERT_EQ(currentTrackChangedSpy.count(), 1);

    ASSERT_TRUE(m_proxyModel->currentTrack().isValid());
    ASSERT_EQ(m_proxyModel->currentTrack(), m_proxyModel->index(2, 0));

    currentTrackChangedSpy.clear();
    m_proxyModel->skipNextTrack();

    // No signal since we're at the end
    ASSERT_EQ(currentTrackChangedSpy.count(), 0);
}

TEST_F(PlaylistProxyModelTest, SkipPreviousTrack) {
    auto entries = createTestEntries(3);
    m_proxyModel->enqueue(entries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    m_proxyModel->switchTo(2);

    QSignalSpy currentTrackChangedSpy(m_proxyModel.get(), SIGNAL(currentTrackChanged(QPersistentModelIndex)));

    m_proxyModel->skipPreviousTrack(0);

    ASSERT_EQ(currentTrackChangedSpy.count(), 1);

    ASSERT_TRUE(m_proxyModel->currentTrack().isValid());
    ASSERT_EQ(m_proxyModel->currentTrack(), m_proxyModel->index(1, 0));

    currentTrackChangedSpy.clear();
    m_proxyModel->skipPreviousTrack(0);

    ASSERT_EQ(currentTrackChangedSpy.count(), 1);

    ASSERT_TRUE(m_proxyModel->currentTrack().isValid());
    ASSERT_EQ(m_proxyModel->currentTrack(), m_proxyModel->index(0, 0));

    currentTrackChangedSpy.clear();
    m_proxyModel->skipPreviousTrack(0);

    // No signal since we're at the beginning
    ASSERT_EQ(currentTrackChangedSpy.count(), 0);
}

TEST_F(PlaylistProxyModelTest, SkipPreviousWithPosition) {
    const auto entries = createTestEntries(3);
    m_proxyModel->enqueue(entries, PlayerUtils::PlaylistEnqueueMode::ReplacePlaylist, PlayerUtils::DoNotTriggerPlay);

    m_proxyModel->switchTo(2);

    QSignalSpy currentTrackChangedSpy(m_proxyModel.get(), SIGNAL(currentTrackChanged(QPersistentModelIndex)));

    // Try with position below threshold
    m_proxyModel->skipPreviousTrack(1000);

    ASSERT_EQ(currentTrackChangedSpy.count(), 1);

    ASSERT_TRUE(m_proxyModel->currentTrack().isValid());
    ASSERT_EQ(m_proxyModel->currentTrack(), m_proxyModel->index(1, 0));

    const auto currentTrackIndex = currentTrackChangedSpy.takeFirst().at(0).value<QPersistentModelIndex>();
    ASSERT_EQ(m_proxyModel->currentTrack(), currentTrackIndex);

    // Try with position past threshold
    m_proxyModel->skipPreviousTrack(4000);

    ASSERT_EQ(currentTrackChangedSpy.count(), 0);

    ASSERT_TRUE(m_proxyModel->currentTrack().isValid());
    ASSERT_EQ(m_proxyModel->currentTrack(), m_proxyModel->index(1, 0));
}
