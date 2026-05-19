#ifndef STUDENTDIALOG_H
#define STUDENTDIALOG_H

#include "Student.h"

#include <QDialog>

class QLineEdit;
class QDoubleSpinBox;

class StudentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StudentDialog(QWidget *parent = nullptr);

    void setStudent(const Student &student);
    Student student() const;

private:
    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QLineEdit *m_classEdit;
    QLineEdit *m_courseEdit;
    QDoubleSpinBox *m_scoreSpinBox;
};

#endif