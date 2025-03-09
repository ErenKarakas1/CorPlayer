#include "testutils.h"

#include "taglib/tagreader.h"
#include "taglib/tracktags.h"

#include <gtest/gtest.h>

class TagReaderTest : public ::testing::Test {
protected:
    TagReader m_tagReader;

    static void debugTags(const TrackTags& tags) {
        for (auto it = tags.fieldMapping().begin(); it != tags.fieldMapping().end(); ++it) {
            qDebug() << "Key: " << it.key() << ", Value: " << it.value().toString();
        }
    }
};

TEST_F(TagReaderTest, ReadMetadataOpus) {
    const QString fileName = QStringLiteral(":/audio/audio.opus");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(Metadata::Fields::Title).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(Metadata::Fields::Artist).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Album).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(Metadata::Fields::AlbumArtist).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Genre).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(Metadata::Fields::Year).toInt(), 2024);
    ASSERT_EQ(tags.value(Metadata::Fields::TrackNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::DiscNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::Duration).toInt(), 30);
    ASSERT_EQ(tags.value(Metadata::Fields::SampleRate).toInt(), 48000);
    ASSERT_EQ(tags.value(Metadata::Fields::BitRate).toInt(), 2000);
    ASSERT_EQ(tags.value(Metadata::Fields::Performer).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Composer).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Comment).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(Metadata::Fields::FileType).toString(), QStringLiteral("Opus"));
}

TEST_F(TagReaderTest, ReadMetadataFlac) {
    const QString fileName = QStringLiteral(":/audio/audio.flac");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(Metadata::Fields::Title).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(Metadata::Fields::Artist).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Album).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(Metadata::Fields::AlbumArtist).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Genre).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(Metadata::Fields::Year).toInt(), 2024);
    ASSERT_EQ(tags.value(Metadata::Fields::TrackNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::DiscNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::Duration).toInt(), 30);
    ASSERT_EQ(tags.value(Metadata::Fields::SampleRate).toInt(), 44100);
    ASSERT_EQ(tags.value(Metadata::Fields::BitRate).toInt(), 1000);
    ASSERT_EQ(tags.value(Metadata::Fields::Performer).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Composer).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Comment).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(Metadata::Fields::FileType).toString(), QStringLiteral("FLAC"));
}

TEST_F(TagReaderTest, ReadMetadataMp3) {
    const QString fileName = QStringLiteral(":/audio/audio.mp3");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(Metadata::Fields::Title).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(Metadata::Fields::Artist).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Album).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(Metadata::Fields::AlbumArtist).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Genre).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(Metadata::Fields::Year).toInt(), 2024);
    ASSERT_EQ(tags.value(Metadata::Fields::TrackNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::DiscNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::Duration).toInt(), 30);
    ASSERT_EQ(tags.value(Metadata::Fields::SampleRate).toInt(), 44100);
    ASSERT_EQ(tags.value(Metadata::Fields::BitRate).toInt(), 128000);
    ASSERT_EQ(tags.value(Metadata::Fields::Performer).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Composer).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Comment).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(Metadata::Fields::HasEmbeddedCover).toBool(), true);
    ASSERT_EQ(tags.value(Metadata::Fields::FileType).toString(), QStringLiteral("MPEG"));
}

TEST_F(TagReaderTest, ReadMetadataM4a) {
    const QString fileName = QStringLiteral(":/audio/audio.m4a");

    const TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(Metadata::Fields::Title).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(Metadata::Fields::Artist).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Album).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(Metadata::Fields::AlbumArtist).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Genre).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(Metadata::Fields::Year).toInt(), 2024);
    ASSERT_EQ(tags.value(Metadata::Fields::TrackNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::DiscNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::Duration).toInt(), 30);
    ASSERT_EQ(tags.value(Metadata::Fields::SampleRate).toInt(), 44100);
    ASSERT_EQ(tags.value(Metadata::Fields::Composer).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Comment).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(Metadata::Fields::FileType).toString(), QStringLiteral("MPEG-4"));
}

TEST_F(TagReaderTest, ReadMetadataOgg) {
    const QString fileName = QStringLiteral(":/audio/audio.ogg");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(Metadata::Fields::Title).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(Metadata::Fields::Artist).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Album).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(Metadata::Fields::AlbumArtist).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(Metadata::Fields::Genre).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(Metadata::Fields::Year).toInt(), 2024);
    ASSERT_EQ(tags.value(Metadata::Fields::TrackNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::DiscNumber).toInt(), 1);
    ASSERT_EQ(tags.value(Metadata::Fields::Duration).toInt(), 30);
    ASSERT_EQ(tags.value(Metadata::Fields::SampleRate).toInt(), 44100);
    ASSERT_EQ(tags.value(Metadata::Fields::BitRate).toInt(), 1000);
    ASSERT_EQ(tags.value(Metadata::Fields::Performer).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Composer).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(Metadata::Fields::Comment).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(Metadata::Fields::FileType).toString(), QStringLiteral("Vorbis"));
}

TEST_F(TagReaderTest, ReadMetadataBenchmark) {
    const QString fileName = QStringLiteral(":/audio/audio.opus");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());

    // Current time is 153 ms
    for (int i = 0; i < 1000; ++i) {
        m_tagReader.readMetadata(tempFile.fileName(), tags);
    }
}
