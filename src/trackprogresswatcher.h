#ifndef TRACKPROGRESSWATCHER_H
#define TRACKPROGRESSWATCHER_H

#include <QObject>
#include <QQmlEngine>

class TrackProgressWatcher : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QString progressDuration READ progressDuration NOTIFY progressDurationChanged)

public:
    explicit TrackProgressWatcher(QObject *parent = nullptr);
    ~TrackProgressWatcher() override;
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

#endif //TRACKPROGRESSWATCHER_H
