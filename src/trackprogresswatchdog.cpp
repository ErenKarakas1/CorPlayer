#include "trackprogresswatchdog.h"
#include <QTime>

TrackProgressWatchdog::TrackProgressWatchdog(QObject *parent) : QObject(parent) {}

TrackProgressWatchdog::~TrackProgressWatchdog() = default;

int TrackProgressWatchdog::position() const {
    return m_position;
}

QString TrackProgressWatchdog::progressDuration() const {
    return m_progressDuration;
}

void TrackProgressWatchdog::setPosition(int position) {
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
