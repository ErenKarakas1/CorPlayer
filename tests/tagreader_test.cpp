#include "testutils.h"

#include "taglib/tagreader.h"
#include "taglib/tracktags.h"

#include <gtest/gtest.h>

class TagReaderTest : public ::testing::Test {
protected:
    TagReader m_tagReader;

    void debugTags(const TrackTags &tags) {
        for (auto it = tags.roleMapping().begin(); it != tags.roleMapping().end(); ++it) {
            qDebug() << "Key: " << it.key() << ", Value: " << it.value();
        }
    }
};

TEST_F(TagReaderTest, ReadMetadataOpus) {
    const QString fileName = QStringLiteral(":/audio/audio.opus");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(MetadataFields::TitleRole).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(MetadataFields::ArtistRole).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumRole).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumArtistRole).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(MetadataFields::GenreRole).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(MetadataFields::YearRole).toInt(), 2024);
    ASSERT_EQ(tags.value(MetadataFields::TrackNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DiscNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DurationRole).toInt(), 30);
    ASSERT_EQ(tags.value(MetadataFields::SampleRateRole).toInt(), 48000);
    ASSERT_EQ(tags.value(MetadataFields::BitRateRole).toInt(), 2000);
    ASSERT_EQ(tags.value(MetadataFields::PerformerRole).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(MetadataFields::ComposerRole).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(MetadataFields::CommentRole).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(MetadataFields::FileTypeRole).toString(), QStringLiteral("Opus"));
}

TEST_F(TagReaderTest, ReadMetadataFlac) {
    const QString fileName = QStringLiteral(":/audio/audio.flac");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(MetadataFields::TitleRole).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(MetadataFields::ArtistRole).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumRole).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumArtistRole).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(MetadataFields::GenreRole).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(MetadataFields::YearRole).toInt(), 2024);
    ASSERT_EQ(tags.value(MetadataFields::TrackNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DiscNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DurationRole).toInt(), 30);
    ASSERT_EQ(tags.value(MetadataFields::SampleRateRole).toInt(), 44100);
    ASSERT_EQ(tags.value(MetadataFields::BitRateRole).toInt(), 1000);
    ASSERT_EQ(tags.value(MetadataFields::PerformerRole).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(MetadataFields::ComposerRole).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(MetadataFields::CommentRole).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(MetadataFields::FileTypeRole).toString(), QStringLiteral("FLAC"));
}

TEST_F(TagReaderTest, ReadMetadataMp3) {
    const QString fileName = QStringLiteral(":/audio/audio.mp3");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(MetadataFields::TitleRole).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(MetadataFields::ArtistRole).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumRole).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumArtistRole).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(MetadataFields::GenreRole).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(MetadataFields::YearRole).toInt(), 2024);
    ASSERT_EQ(tags.value(MetadataFields::TrackNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DiscNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DurationRole).toInt(), 30);
    ASSERT_EQ(tags.value(MetadataFields::SampleRateRole).toInt(), 44100);
    ASSERT_EQ(tags.value(MetadataFields::BitRateRole).toInt(), 128000);
    ASSERT_EQ(tags.value(MetadataFields::PerformerRole).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(MetadataFields::ComposerRole).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(MetadataFields::CommentRole).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(MetadataFields::HasEmbeddedCover).toBool(), false);
    ASSERT_EQ(tags.value(MetadataFields::FileTypeRole).toString(), QStringLiteral("MPEG"));
}

TEST_F(TagReaderTest, ReadMetadataM4a) {
    const QString fileName = QStringLiteral(":/audio/audio.m4a");

    const TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(MetadataFields::TitleRole).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(MetadataFields::ArtistRole).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumRole).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumArtistRole).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(MetadataFields::GenreRole).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(MetadataFields::YearRole).toInt(), 2024);
    ASSERT_EQ(tags.value(MetadataFields::TrackNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DiscNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DurationRole).toInt(), 30);
    ASSERT_EQ(tags.value(MetadataFields::SampleRateRole).toInt(), 44100);
    ASSERT_EQ(tags.value(MetadataFields::ComposerRole).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(MetadataFields::CommentRole).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(MetadataFields::FileTypeRole).toString(), QStringLiteral("MPEG-4"));
}

TEST_F(TagReaderTest, ReadMetadataOgg) {
    const QString fileName = QStringLiteral(":/audio/audio.ogg");

    TemporaryFile tempFile(fileName);

    ASSERT_FALSE(tempFile.fileName().isEmpty());

    TrackTags tags(tempFile.fileName());
    m_tagReader.readMetadata(tempFile.fileName(), tags);

    ASSERT_EQ(tags.value(MetadataFields::TitleRole).toString(), QStringLiteral("Test Title"));
    ASSERT_EQ(tags.value(MetadataFields::ArtistRole).toString(), QStringLiteral("Test Artist"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumRole).toString(), QStringLiteral("Test Album"));
    ASSERT_EQ(tags.value(MetadataFields::AlbumArtistRole).toString(), QStringLiteral("Test Album Artist"));
    ASSERT_EQ(tags.value(MetadataFields::GenreRole).toString(), QStringLiteral("Rock"));
    ASSERT_EQ(tags.value(MetadataFields::YearRole).toInt(), 2024);
    ASSERT_EQ(tags.value(MetadataFields::TrackNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DiscNumberRole).toInt(), 1);
    ASSERT_EQ(tags.value(MetadataFields::DurationRole).toInt(), 30);
    ASSERT_EQ(tags.value(MetadataFields::SampleRateRole).toInt(), 44100);
    ASSERT_EQ(tags.value(MetadataFields::BitRateRole).toInt(), 1000);
    ASSERT_EQ(tags.value(MetadataFields::PerformerRole).toString(), QStringLiteral("Test Performer"));
    ASSERT_EQ(tags.value(MetadataFields::ComposerRole).toString(), QStringLiteral("Test Composer"));
    ASSERT_EQ(tags.value(MetadataFields::CommentRole).toString(), QStringLiteral("This is a test file"));
    ASSERT_EQ(tags.value(MetadataFields::FileTypeRole).toString(), QStringLiteral("Vorbis"));
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
