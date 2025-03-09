#ifndef TESTUTILS_H
#define TESTUTILS_H

#include "metadata.hpp"
#include "database/databasemanager.h"

#include "gtest/gtest.h"

#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <QTemporaryFile>

// ==========================================================================================================
// Global test environment
// ==========================================================================================================

class TestEnv : public ::testing::Environment {
protected:
    std::unique_ptr<QCoreApplication> m_app;

    void SetUp() override;
    void TearDown() override;
};

inline const ::testing::Environment* const env = ::testing::AddGlobalTestEnvironment(new TestEnv);

class GlobalTest : public ::testing::Test {
protected:
    DatabaseManager* m_dbManager = nullptr;
    QTemporaryDir m_tempDir;

    void SetUp() override;
    std::shared_ptr<DbConnectionPool> dbConnectionPool();
};

class TemporaryFile : public QTemporaryFile {
    Q_OBJECT

public:
    explicit TemporaryFile(const QString &fileName, QObject *parent = nullptr);
};

// ==========================================================================================================
// End of global test environment
// ==========================================================================================================

enum class AudioFormat : std::uint8_t { Mp3, Ogg, Flac };

class AudioFile {
public:
    static QUrl create(const Metadata::TrackFields& metadata = {});
    static QUrl create(AudioFormat format, const Metadata::TrackFields& metadata = {});

private:
    static QUrl createMp3(const Metadata::TrackFields& metadata);
    static QUrl createOgg(const Metadata::TrackFields& metadata);
    static QUrl createFlac(const Metadata::TrackFields& metadata);
};

#endif // TESTUTILS_H
