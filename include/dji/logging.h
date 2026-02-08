#ifndef DJI_LOGGING_H
#define DJI_LOGGING_H

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(djiBleLog)

#define DJI_BLE_LOG(msg) qCDebug(djiBleLog).noquote() << msg

#endif // DJI_LOGGING_H
