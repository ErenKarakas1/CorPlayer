#ifndef ASYNCCOVERPROVIDER_H
#define ASYNCCOVERPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <QThreadPool>

class AsyncCoverProvider : public QQuickAsyncImageProvider {
    Q_OBJECT

public:
    AsyncCoverProvider();
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override;

private:
    QThreadPool m_threadPool;
};

#endif // ASYNCCOVERPROVIDER_H
