#pragma once
#include <QObject>
class GLobalSignal : public QObject
{
    Q_OBJECT

public:
    GLobalSignal() = default;
Q_SIGNALS:
    void signal_log(const QString& log);

};

extern GLobalSignal g_globalSignal;

#define LOG(text) emit g_globalSignal.signal_log(text)