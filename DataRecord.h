#ifndef DATARECORD_H
#define DATARECORD_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>

struct DataRecord
{
    QDateTime timestamp;
    double value1 = 0.0;
    double value2 = 0.0;
    QString status = "normal";

    static DataRecord fromJson(const QJsonObject &obj)
    {
        DataRecord r;

        QJsonValue ts = obj.value("timestamp");
        if (ts.isDouble()) {
            // Python time.time() 是秒级时间戳
            qint64 msecs = static_cast<qint64>(ts.toDouble() * 1000.0);
            r.timestamp = QDateTime::fromMSecsSinceEpoch(msecs);
        } else if (ts.isString()) {
            r.timestamp = QDateTime::fromString(ts.toString(), Qt::ISODate);
            if (!r.timestamp.isValid()) {
                r.timestamp = QDateTime::currentDateTime();
            }
        } else {
            r.timestamp = QDateTime::currentDateTime();
        }

        r.value1 = obj.value("value1").toDouble();
        r.value2 = obj.value("value2").toDouble();
        r.status = obj.value("status").toString("normal").trimmed().toLower();

        if (r.status.isEmpty()) {
            r.status = "normal";
        }

        return r;
    }

    QStringList toStringList() const
    {
        return {
            timestamp.toString("yyyy-MM-dd HH:mm:ss"),
            QString::number(value1, 'f', 2),
            QString::number(value2, 'f', 2),
            status
        };
    }

    QString toCsvLine() const
    {
        QStringList list = toStringList();
        for (QString &item : list) {
            item.replace("\"", "\"\"");
            item = "\"" + item + "\"";
        }
        return list.join(",") + "\n";
    }
};

#endif
