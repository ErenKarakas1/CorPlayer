#include "asynccoverprovider.h"

#include <utility>

#include "taglib/tagreader.h"
#include "taglib/tracktags.h"

class AsyncImageResponse : public QQuickImageResponse, public QRunnable {
    Q_OBJECT

public:
    AsyncImageResponse(QString id, const QSize& requestedSize) : m_id(std::move(id)), m_requestedSize(requestedSize) {
        setAutoDelete(false);

        if (m_requestedSize.isEmpty()) {
            m_requestedSize = QSize(63, 63);
        }
    }

    [[nodiscard]] QQuickTextureFactory* textureFactory() const override {
        return QQuickTextureFactory::textureFactoryForImage(m_coverImage);
    }

    void run() override {
        TrackTags tags(m_id);
        m_tagReader.readMetadata(m_id, tags);

        const auto coverImage = tags.coverImage();
        if (coverImage.isEmpty()) {
            m_errorString = QStringLiteral("Failed to load cover image for %1").arg(m_id);
            Q_EMIT finished();
            return;
        }

        m_coverImage = QImage::fromData(coverImage);
        if (m_coverImage.isNull()) {
            m_errorString = QStringLiteral("Failed to load cover image for %1").arg(m_id);
            Q_EMIT finished();
            return;
        }

        QImage scaledImage = m_coverImage.scaled(m_requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!scaledImage.isNull()) {
            m_coverImage = std::move(scaledImage);
        }

        Q_EMIT finished();
    }

    [[nodiscard]] QString errorString() const override {
        return m_errorString;
    }

private:
    QString m_id;
    QSize m_requestedSize;
    QImage m_coverImage;
    QString m_errorString;
    TagReader m_tagReader;
};

AsyncCoverProvider::AsyncCoverProvider() = default;

QQuickImageResponse* AsyncCoverProvider::requestImageResponse(const QString& id, const QSize& requestedSize) {
    auto response = std::make_unique<AsyncImageResponse>(id, requestedSize);
    m_threadPool.start(response.get());
    return response.release();
}

#include "asynccoverprovider.moc"
