#ifndef TRACKPROGRESSWATCHER_H
#define TRACKPROGRESSWATCHER_H

#include <QQmlEngine>

class TrackProgressWatchdog : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QString progressDuration READ progressDuration NOTIFY progressDurationChanged)

public:
    explicit TrackProgressWatchdog(QObject* parent = nullptr);
    ~TrackProgressWatchdog() override;
    [[nodiscard]] int position() const;
    [[nodiscard]] QString progressDuration() const;

Q_SIGNALS:
    void positionChanged();
    void progressDurationChanged();

public Q_SLOTS:
    void setPosition(int position);

private:
    int m_position = 0;
    QString m_progressDuration;
};

#endif // TRACKPROGRESSWATCHER_H
