#ifndef STUDENTMANAGER_H
#define STUDENTMANAGER_H

#include "Student.h"

#include <QList>
#include <QString>

class StudentManager
{
public:
    struct Statistics
    {
        int count = 0;
        int passCount = 0;
        double average = 0.0;
        double maxScore = 0.0;
        double minScore = 0.0;
        double passRate = 0.0;
    };

public:
    explicit StudentManager(const QString &filePath = QString());

    bool load(QString *errorMessage = nullptr);
    bool save(QString *errorMessage = nullptr) const;

    const QList<Student> &students() const;

    bool addStudent(const Student &student, QString *errorMessage = nullptr);
    bool updateStudent(int index, const Student &student, QString *errorMessage = nullptr);
    bool removeStudent(int index, QString *errorMessage = nullptr);

    QList<int> search(const QString &id,
                      const QString &name,
                      const QString &className,
                      const QString &course) const;

    void sortByScoreAscending();
    void sortByScoreDescending();

    bool calculateStatistics(const QString &className,
                             const QString &course,
                             Statistics &statistics) const;

    QString dataFilePath() const;

private:
    bool validateStudent(const Student &student,
                         QString *errorMessage = nullptr,
                         int ignoreIndex = -1) const;

private:
    QList<Student> m_students;
    QString m_filePath;
};

#endif