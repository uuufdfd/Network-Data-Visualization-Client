#ifndef STUDENT_H
#define STUDENT_H

#include <QString>
#include <QJsonObject>

class Student
{
public:
    QString id;          // 学号
    QString name;        // 姓名
    QString className;   // 班级
    QString course;      // 课程
    double score = 0.0;  // 成绩

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["className"] = className;
        obj["course"] = course;
        obj["score"] = score;
        return obj;
    }

    static Student fromJson(const QJsonObject &obj)
    {
        Student s;
        s.id = obj["id"].toString();
        s.name = obj["name"].toString();
        s.className = obj["className"].toString();
        s.course = obj["course"].toString();
        s.score = obj["score"].toDouble();
        return s;
    }
};

#endif