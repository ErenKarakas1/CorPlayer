#include "testutils.h"

#include "mediaplayerwrapper.h"

#include <QSignalSpy>
#include <QMediaPlayer>
#include <QTest>

class MediaPlayerWrapperTest : public ::testing::Test {
protected:
    std::unique_ptr<MediaPlayerWrapper> m_player;

    void SetUp() override {
        m_player = std::make_unique<MediaPlayerWrapper>();
    }

    void TearDown() override {
        m_player.reset();
    }
};

TEST_F(MediaPlayerWrapperTest, InitialState) {
    ASSERT_EQ(m_player->playbackState(), QMediaPlayer::StoppedState);
    ASSERT_EQ(m_player->status(), QMediaPlayer::NoMedia);
    ASSERT_EQ(m_player->error(), QMediaPlayer::NoError);
    ASSERT_FALSE(m_player->isSeekable());
    ASSERT_EQ(m_player->position(), 0);
    ASSERT_EQ(m_player->duration(), 0);
}

TEST_F(MediaPlayerWrapperTest, SetSource) {
    QUrl fileUrl = AudioFile::create();
    ASSERT_FALSE(fileUrl.toString().isEmpty());

    QSignalSpy sourceChangedSpy(m_player.get(), SIGNAL(sourceChanged()));
    QSignalSpy statusChangedSpy(m_player.get(), SIGNAL(statusChanged(QMediaPlayer::MediaStatus)));

    m_player->setSource(fileUrl);

    ASSERT_TRUE(sourceChangedSpy.wait(100));
    ASSERT_TRUE(statusChangedSpy.wait(100));

    ASSERT_EQ(m_player->source(), fileUrl);
}

TEST_F(MediaPlayerWrapperTest, PlayPauseStop) {
    QUrl fileUrl = AudioFile::create();
    ASSERT_FALSE(fileUrl.toString().isEmpty());

    m_player->setSource(fileUrl);

    QSignalSpy playbackStateChangedSpy(m_player.get(), SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)));

    m_player->play();

    ASSERT_TRUE(playbackStateChangedSpy.wait(100));
    ASSERT_GE(playbackStateChangedSpy.count(), 1);

    playbackStateChangedSpy.clear();
    m_player->pause();

    ASSERT_TRUE(playbackStateChangedSpy.wait(100));
    ASSERT_GE(playbackStateChangedSpy.count(), 1);
    ASSERT_EQ(m_player->playbackState(), QMediaPlayer::PausedState);

    playbackStateChangedSpy.clear();
    m_player->stop();

    ASSERT_TRUE(playbackStateChangedSpy.wait(100));
    ASSERT_GE(playbackStateChangedSpy.count(), 1);
    ASSERT_EQ(m_player->playbackState(), QMediaPlayer::StoppedState);
}

TEST_F(MediaPlayerWrapperTest, SeekPosition) {
    QUrl fileUrl = AudioFile::create();
    ASSERT_FALSE(fileUrl.toString().isEmpty());

    m_player->setSource(fileUrl);
    m_player->play();

    QSignalSpy loadedSpy(m_player.get(), SIGNAL(statusChanged(QMediaPlayer::MediaStatus)));
    ASSERT_TRUE(loadedSpy.wait(200));

    QTest::qWait(100); // Brief wait for duration to be known

    // Only test seeking if the media is seekable
    if (m_player->isSeekable() && m_player->duration() > 0) {
        QSignalSpy positionChangedSpy(m_player.get(), SIGNAL(positionChanged(qint64)));

        // Seek to 2 seconds
        qint64 seekPos = 2000;
        m_player->setPosition(seekPos);

        ASSERT_TRUE(positionChangedSpy.wait(100));

        const qint64 actualPos = m_player->position();
        ASSERT_NEAR(actualPos, seekPos, 500); // 500ms tolerance
    }
}

TEST_F(MediaPlayerWrapperTest, VolumeAndMute) {
    // TODO: should be 100
    ASSERT_EQ(m_player->volume(), 99);
    ASSERT_FALSE(m_player->isMuted());

    QSignalSpy volumeChangedSpy(m_player.get(), SIGNAL(volumeChanged()));
    m_player->setVolume(50);

    ASSERT_TRUE(volumeChangedSpy.wait(100));
    ASSERT_FLOAT_EQ(m_player->volume(), 50);

    QSignalSpy mutedChangedSpy(m_player.get(), SIGNAL(mutedChanged(bool)));
    m_player->setMuted(true);

    ASSERT_TRUE(mutedChangedSpy.wait(100));
    ASSERT_TRUE(mutedChangedSpy.at(0).at(0).toBool());
    ASSERT_TRUE(m_player->isMuted());

    m_player->setMuted(false);
    ASSERT_FLOAT_EQ(m_player->volume(), 50);
}

//TEST_F(MediaPlayerWrapperTest, SaveAndRestorePosition) {
//    QUrl fileUrl = AudioFile::create();
//    ASSERT_FALSE(fileUrl.toString().isEmpty());
//
//    m_player->setSource(fileUrl);
//    m_player->play();
//
//    QSignalSpy loadedSpy(m_player.get(), SIGNAL(statusChanged(QMediaPlayer::MediaStatus)));
//    ASSERT_TRUE(loadedSpy.wait(2000));
//
//    QTest::qWait(500);
//
//    if (m_player->isSeekable() && m_player->duration() > 0) {
//        qint64 testPos = 2000;
//        m_player->setPosition(testPos);
//
//        QTest::qWait(100);
//
//        // Save position
//        m_player->savePosition();
//
//        m_player->setPosition(4000);
//
//        QTest::qWait(100);
//
//        m_player->restorePosition();
//
//        QTest::qWait(100);
//
//        ASSERT_NEAR(m_player->position(), testPos, 500); // 500ms tolerance
//    }
//}
