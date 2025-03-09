#include "testutils.h"

#include "asynccoverprovider.h"

#include <QQuickImageResponse>
#include <QSignalSpy>

class AsyncCoverProviderTest : public ::testing::Test {
protected:
    AsyncCoverProvider m_coverProvider;
};

TEST_F(AsyncCoverProviderTest, RequestImageResponseBasic) {
    TemporaryFile tempFile(QStringLiteral(":/audio/audio.mp3"));
    ASSERT_FALSE(tempFile.fileName().isEmpty());

    QQuickImageResponse* response = m_coverProvider.requestImageResponse(tempFile.fileName(), QSize(100, 100));
    ASSERT_TRUE(response != nullptr);

    QSignalSpy spy(response, SIGNAL(finished()));
    ASSERT_TRUE(spy.wait(200)); // Wait up to 2 seconds

    ASSERT_TRUE(response->errorString().isEmpty());

    delete response;
}

TEST_F(AsyncCoverProviderTest, InvalidFile) {
    // Request an image for a non-existent file
    QQuickImageResponse* response = m_coverProvider.requestImageResponse(QStringLiteral("/non/existent/file.mp3"), QSize(100, 100));
    ASSERT_TRUE(response != nullptr);

    QSignalSpy spy(response, SIGNAL(finished()));
    ASSERT_TRUE(spy.wait(200));

    ASSERT_FALSE(response->errorString().isEmpty());

    delete response;
}

TEST_F(AsyncCoverProviderTest, ScaledImage) {
    TemporaryFile tempFile(QStringLiteral(":/audio/audio.mp3"));
    ASSERT_FALSE(tempFile.fileName().isEmpty());

    QSize requestSize(50, 50);
    QQuickImageResponse* response = m_coverProvider.requestImageResponse(tempFile.fileName(), requestSize);
    ASSERT_TRUE(response != nullptr);

    QSignalSpy spy(response, SIGNAL(finished()));
    ASSERT_TRUE(spy.wait(200));

    ASSERT_TRUE(response->errorString().isEmpty());

    delete response;
}

TEST_F(AsyncCoverProviderTest, EmptySize) {
    TemporaryFile tempFile(QStringLiteral(":/audio/audio.mp3"));
    ASSERT_FALSE(tempFile.fileName().isEmpty());

    QQuickImageResponse* response = m_coverProvider.requestImageResponse(tempFile.fileName(), QSize());

    QSignalSpy spy(response, SIGNAL(finished()));
    ASSERT_TRUE(spy.wait(200));

    ASSERT_TRUE(response->errorString().isEmpty());

    delete response;
}
