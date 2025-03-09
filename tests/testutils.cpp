#include "testutils.h"

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/id3v2frame.h>
#include <taglib/textidentificationframe.h>

#include "database/databasemanager.h"

#include <QDir>
#include <QTemporaryFile>
#include <QUuid>

void TestEnv::SetUp() {
    static int argc = 1;
    static char* argv[] = {const_cast<char*>("corplayer_test")};

    if (QCoreApplication::instance() == nullptr) {
        m_app = std::make_unique<QCoreApplication>(argc, argv);
    }
}

void TestEnv::TearDown() {
    m_app.reset();
}

void GlobalTest::SetUp() {
    ASSERT_TRUE(QCoreApplication::instance() != nullptr);
    ASSERT_TRUE(m_tempDir.isValid());
}

std::shared_ptr<DbConnectionPool> GlobalTest::dbConnectionPool() {
    if (m_dbManager == nullptr) {
        const QString dbPath = m_tempDir.filePath(QStringLiteral("test.db"));
        m_dbManager = &DatabaseManager::instance();
        m_dbManager->initialize(dbPath);
    }

    return m_dbManager->dbConnectionPool();
}

TemporaryFile::TemporaryFile(const QString& fileName, QObject* parent) : QTemporaryFile{parent} {
    setFileTemplate(QDir::tempPath() + QStringLiteral("/temp-XXXXXX"));
    if (!open()) {
        qWarning() << "Failed to open temporary file: " << fileName;
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file: " << fileName;
        return;
    }
    write(file.readAll());

    QTemporaryFile::reset();
}

QUrl AudioFile::create(const Metadata::TrackFields& metadata) {
    return createMp3(metadata);
}

QUrl AudioFile::create(const AudioFormat format, const Metadata::TrackFields& metadata) {
    switch (format) {
    case AudioFormat::Mp3:
        return createMp3(metadata);
    case AudioFormat::Ogg:
        return createOgg(metadata);
    case AudioFormat::Flac:
        return createFlac(metadata);
    default:
        break;
    }

    return {};
}

QUrl AudioFile::createMp3(const Metadata::TrackFields& metadata) {
    const QString sourceFile = QStringLiteral(":/audio/audio.mp3");
    const QString targetPath = QDir::tempPath() + QLatin1String("/") + QStringLiteral("test_") +
                               QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".mp3");

    QFile source(sourceFile);
    if (!source.exists()) {
        qWarning() << "Test file does not exist: " << sourceFile;
        return {};
    }

    if (!source.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open source file: " << sourceFile;
        return {};
    }

    QFile target(targetPath);
    if (!target.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not create target file: " << targetPath;
        return {};
    }

    target.write(source.readAll());
    target.close();

    if (!metadata.isEmpty()) {
        TagLib::MPEG::File file(targetPath.toUtf8().constData());
        if (!file.isValid()) {
            qWarning() << "Failed to open file: " << targetPath;
            return {};
        }

        auto properties = file.properties();

        // TODO: no need to go one by one like this
        if (metadata.contains(Metadata::Fields::Title)) {
            properties.replace("TITLE", QStringToTString(metadata.get(Metadata::Fields::Title).toString()));
        }
        if (metadata.contains(Metadata::Fields::Artist)) {
            properties.replace("ARTIST", QStringToTString(metadata.get(Metadata::Fields::Artist).toString()));
        }
        if (metadata.contains(Metadata::Fields::Album)) {
            properties.replace("ALBUM", QStringToTString(metadata.get(Metadata::Fields::Album).toString()));
        }
        if (metadata.contains(Metadata::Fields::AlbumArtist)) {
            properties.replace("ALBUMARTIST", QStringToTString(metadata.get(Metadata::Fields::AlbumArtist).toString()));
        }
        if (metadata.contains(Metadata::Fields::TrackNumber)) {
            const int trackNumber = metadata.get(Metadata::Fields::TrackNumber).toInt();
            if (trackNumber > 0) {
                properties.replace("TRACKNUMBER", QStringToTString(QString::number(trackNumber)));
            }
        }
        if (metadata.contains(Metadata::Fields::DiscNumber)) {
            const int discNumber = metadata.get(Metadata::Fields::DiscNumber).toInt();
            if (discNumber > 0) {
                properties.replace("DISCNUMBER", QStringToTString(QString::number(discNumber)));
            }
        }
        if (metadata.contains(Metadata::Fields::Performer)) {
            properties.replace("PERFORMER", QStringToTString(metadata.get(Metadata::Fields::Performer).toString()));
        }
        if (metadata.contains(Metadata::Fields::Composer)) {
            properties.replace("COMPOSER", QStringToTString(metadata.get(Metadata::Fields::Composer).toString()));
        }

        file.setProperties(properties);

        if (!file.save(TagLib::MPEG::File::ID3v2)) {
            qWarning() << "Failed to save metadata to file: " << targetPath;
            return {};
        }
    }

    return QUrl::fromLocalFile(targetPath);
}

QUrl AudioFile::createOgg(const Metadata::TrackFields& metadata) {
    const QString sourceFile = QStringLiteral(":/audio/audio.ogg");
    const QString targetPath = QDir::tempPath() + QLatin1String("/") + QStringLiteral("test_") +
                               QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".ogg");

    QFile source(sourceFile);
    if (!source.exists()) {
        qWarning() << "Test file does not exist: " << sourceFile;
        return {};
    }

    if (!source.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open source file: " << sourceFile;
        return {};
    }

    QFile target(targetPath);
    if (!target.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not create target file: " << targetPath;
        return {};
    }

    target.write(source.readAll());
    target.close();

    if (!metadata.isEmpty()) {
    }

    return QUrl::fromLocalFile(targetPath);
}

QUrl AudioFile::createFlac(const Metadata::TrackFields& metadata) {
    const QString sourceFile = QStringLiteral(":/audio/audio.flac");
    const QString targetPath = QDir::tempPath() + QLatin1String("/") + QStringLiteral("test_") +
                               QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".flac");

    QFile source(sourceFile);
    if (!source.exists()) {
        qWarning() << "Test file does not exist: " << sourceFile;
        return {};
    }

    if (!source.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open source file: " << sourceFile;
        return {};
    }

    QFile target(targetPath);
    if (!target.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not create target file: " << targetPath;
        return {};
    }

    target.write(source.readAll());
    target.close();

    if (!metadata.isEmpty()) {
    }

    return QUrl::fromLocalFile(targetPath);
}

