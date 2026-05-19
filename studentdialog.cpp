#include <QPushButton>
#include "StudentDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

StudentDialog::StudentDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("学生成绩信息");
    resize(360, 260);

    m_idEdit = new QLineEdit(this);
    m_nameEdit = new QLineEdit(this);
    m_classEdit = new QLineEdit(this);
    m_courseEdit = new QLineEdit(this);

    m_scoreSpinBox = new QDoubleSpinBox(this);
    m_scoreSpinBox->setRange(0.0, 100.0);
    m_scoreSpinBox->setDecimals(1);
    m_scoreSpinBox->setSingleStep(1.0);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("学号：", m_idEdit);
    formLayout->addRow("姓名：", m_nameEdit);
    formLayout->addRow("班级：", m_classEdit);
    formLayout->addRow("课程：", m_courseEdit);
    formLayout->addRow("成绩：", m_scoreSpinBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
        );

    buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");

    connect(buttonBox, &QDialogButtonBox::accepted, this, &StudentDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &StudentDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

void StudentDialog::setStudent(const Student &student)
{
    m_idEdit->setText(student.id);
    m_nameEdit->setText(student.name);
    m_classEdit->setText(student.className);
    m_courseEdit->setText(student.course);
    m_scoreSpinBox->setValue(student.score);
}

Student StudentDialog::student() const
{
    Student s;
    s.id = m_idEdit->text().trimmed();
    s.name = m_nameEdit->text().trimmed();
    s.className = m_classEdit->text().trimmed();
    s.course = m_courseEdit->text().trimmed();
    s.score = m_scoreSpinBox->value();
    return s;
}