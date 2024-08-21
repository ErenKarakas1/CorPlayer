#include "trackprogresswatcher.h"
#include <QTime>

TrackProgressWatcher::TrackProgressWatcher(QObject *parent) : QObject(parent) {}

TrackProgressWatcher::~TrackProgressWatcher() = default;

int TrackProgressWatcher::position() const {
    return m_position;
}

QString TrackProgressWatcher::progressDuration() const {
    return m_progressDuration;
}

void TrackProgressWatcher::setPosition(int position) {
    if (m_position == position) {
        return;
    }

    m_position = position;
    QTime currentProgress = QTime::fromMSecsSinceStartOfDay(m_position);

    if (currentProgress.hour() == 0) {
        m_progressDuration = currentProgress.toString(QStringLiteral("m:ss"));
    } else {
        m_progressDuration = currentProgress.toString(QStringLiteral("h:mm:ss"));
    }

    Q_EMIT positionChanged();
    Q_EMIT progressDurationChanged();
}
