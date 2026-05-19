#include "StudentManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <algorithm>

StudentManager::StudentManager(const QString &filePath)
{
    if (filePath.isEmpty()) {
        m_filePath = QCoreApplication::applicationDirPath() + QDir::separator() + "students.json";
    } else {
        m_filePath = filePath;
    }
}

bool StudentManager::load(QString *errorMessage)
{
    m_students.clear();

    QFile file(m_filePath);

    if (!file.exists()) {
        return save(errorMessage);
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = "无法打开数据文件：" + file.errorString();
        }
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.trimmed().isEmpty()) {
        return true;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage) {
            *errorMessage = "JSON 文件解析失败：" + parseError.errorString();
        }
        return false;
    }

    if (!doc.isArray()) {
        if (errorMessage) {
            *errorMessage = "数据文件格式错误：根节点不是数组。";
        }
        return false;
    }

    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        if (value.isObject()) {
            m_students.append(Student::fromJson(value.toObject()));
        }
    }

    return true;
}

bool StudentManager::save(QString *errorMessage) const
{
    QJsonArray array;

    for (const Student &student : m_students) {
        array.append(student.toJson());
    }

    QJsonDocument doc(array);

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "保存数据失败：" + file.errorString();
        }
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

const QList<Student> &StudentManager::students() const
{
    return m_students;
}

bool StudentManager::addStudent(const Student &student, QString *errorMessage)
{
    if (!validateStudent(student, errorMessage)) {
        return false;
    }

    m_students.append(student);
    return save(errorMessage);
}

bool StudentManager::updateStudent(int index, const Student &student, QString *errorMessage)
{
    if (index < 0 || index >= m_students.size()) {
        if (errorMessage) {
            *errorMessage = "请选择要修改的记录。";
        }
        return false;
    }

    if (!validateStudent(student, errorMessage, index)) {
        return false;
    }

    m_students[index] = student;
    return save(errorMessage);
}

bool StudentManager::removeStudent(int index, QString *errorMessage)
{
    if (index < 0 || index >= m_students.size()) {
        if (errorMessage) {
            *errorMessage = "请选择要删除的记录。";
        }
        return false;
    }

    m_students.removeAt(index);
    return save(errorMessage);
}

QList<int> StudentManager::search(const QString &id,
                                  const QString &name,
                                  const QString &className,
                                  const QString &course) const
{
    QList<int> result;

    QString idKey = id.trimmed();
    QString nameKey = name.trimmed();
    QString classKey = className.trimmed();
    QString courseKey = course.trimmed();

    for (int i = 0; i < m_students.size(); ++i) {
        const Student &s = m_students[i];

        bool matched = true;

        if (!idKey.isEmpty() && s.id != idKey) {
            matched = false;
        }

        if (!nameKey.isEmpty() && s.name != nameKey) {
            matched = false;
        }

        if (!classKey.isEmpty() && !s.className.contains(classKey, Qt::CaseInsensitive)) {
            matched = false;
        }

        if (!courseKey.isEmpty() && !s.course.contains(courseKey, Qt::CaseInsensitive)) {
            matched = false;
        }

        if (matched) {
            result.append(i);
        }
    }

    return result;
}

void StudentManager::sortByScoreAscending()
{
    std::sort(m_students.begin(), m_students.end(), [](const Student &a, const Student &b) {
        return a.score < b.score;
    });

    save();
}

void StudentManager::sortByScoreDescending()
{
    std::sort(m_students.begin(), m_students.end(), [](const Student &a, const Student &b) {
        return a.score > b.score;
    });

    save();
}

bool StudentManager::calculateStatistics(const QString &className,
                                         const QString &course,
                                         Statistics &statistics) const
{
    statistics = Statistics();

    QString classKey = className.trimmed();
    QString courseKey = course.trimmed();

    if (classKey.isEmpty() || courseKey.isEmpty()) {
        return false;
    }

    double sum = 0.0;
    bool first = true;

    for (const Student &s : m_students) {
        if (s.className == classKey && s.course == courseKey) {
            statistics.count++;
            sum += s.score;

            if (s.score >= 60.0) {
                statistics.passCount++;
            }

            if (first) {
                statistics.maxScore = s.score;
                statistics.minScore = s.score;
                first = false;
            } else {
                if (s.score > statistics.maxScore) {
                    statistics.maxScore = s.score;
                }

                if (s.score < statistics.minScore) {
                    statistics.minScore = s.score;
                }
            }
        }
    }

    if (statistics.count == 0) {
        return false;
    }

    statistics.average = sum / statistics.count;
    statistics.passRate = statistics.passCount * 100.0 / statistics.count;

    return true;
}

QString StudentManager::dataFilePath() const
{
    return m_filePath;
}

bool StudentManager::validateStudent(const Student &student,
                                     QString *errorMessage,
                                     int ignoreIndex) const
{
    if (student.id.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "学号不能为空。";
        }
        return false;
    }

    if (student.name.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "姓名不能为空。";
        }
        return false;
    }

    if (student.className.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "班级不能为空。";
        }
        return false;
    }

    if (student.course.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "课程不能为空。";
        }
        return false;
    }

    if (student.score < 0.0 || student.score > 100.0) {
        if (errorMessage) {
            *errorMessage = "成绩必须在 0 到 100 之间。";
        }
        return false;
    }

    for (int i = 0; i < m_students.size(); ++i) {
        if (i == ignoreIndex) {
            continue;
        }

        const Student &s = m_students[i];

        if (s.id == student.id && s.course == student.course) {
            if (errorMessage) {
                *errorMessage = "同一个学生的同一门课程已经存在，不能重复添加。";
            }
            return false;
        }
    }

    return true;
}