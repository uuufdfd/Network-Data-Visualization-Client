#include "MainWindow.h"
#include "StudentDialog.h"

#include <QAction>
#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupStyle();

    QString error;
    if (!m_manager.load(&error)) {
        showError(error);
    }

    refreshTable();
}

void MainWindow::setupUi()
{
    setWindowTitle("学生成绩管理系统");
    resize(980, 640);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QLabel *titleLabel = new QLabel("学生成绩管理系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("titleLabel");

    mainLayout->addWidget(titleLabel);

    QGroupBox *searchGroupBox = new QGroupBox("查询条件", this);
    QGridLayout *searchLayout = new QGridLayout(searchGroupBox);

    m_idSearchEdit = new QLineEdit(this);
    m_nameSearchEdit = new QLineEdit(this);
    m_classSearchEdit = new QLineEdit(this);
    m_courseSearchEdit = new QLineEdit(this);

    m_idSearchEdit->setPlaceholderText("精确查询学号");
    m_nameSearchEdit->setPlaceholderText("精确查询姓名");
    m_classSearchEdit->setPlaceholderText("模糊查询班级");
    m_courseSearchEdit->setPlaceholderText("模糊查询课程");

    QPushButton *searchButton = new QPushButton("查询", this);
    QPushButton *resetButton = new QPushButton("重置", this);

    searchLayout->addWidget(new QLabel("学号：", this), 0, 0);
    searchLayout->addWidget(m_idSearchEdit, 0, 1);
    searchLayout->addWidget(new QLabel("姓名：", this), 0, 2);
    searchLayout->addWidget(m_nameSearchEdit, 0, 3);
    searchLayout->addWidget(new QLabel("班级：", this), 1, 0);
    searchLayout->addWidget(m_classSearchEdit, 1, 1);
    searchLayout->addWidget(new QLabel("课程：", this), 1, 2);
    searchLayout->addWidget(m_courseSearchEdit, 1, 3);
    searchLayout->addWidget(searchButton, 0, 4);
    searchLayout->addWidget(resetButton, 1, 4);

    mainLayout->addWidget(searchGroupBox);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << "学号" << "姓名" << "班级" << "课程" << "成绩");
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setAlternatingRowColors(true);

    mainLayout->addWidget(m_tableWidget);

    QHBoxLayout *buttonLayout = new QHBoxLayout;

    QPushButton *addButton = new QPushButton("添加", this);
    QPushButton *editButton = new QPushButton("修改", this);
    QPushButton *deleteButton = new QPushButton("删除", this);
    QPushButton *sortAscButton = new QPushButton("成绩升序", this);
    QPushButton *sortDescButton = new QPushButton("成绩降序", this);
    QPushButton *statisticsButton = new QPushButton("统计成绩", this);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(sortAscButton);
    buttonLayout->addWidget(sortDescButton);
    buttonLayout->addWidget(statisticsButton);

    mainLayout->addLayout(buttonLayout);

    m_statusLabel = new QLabel(this);
    mainLayout->addWidget(m_statusLabel);

    QMenu *fileMenu = menuBar()->addMenu("文件");
    QAction *exitAction = fileMenu->addAction("退出");

    QMenu *helpMenu = menuBar()->addMenu("帮助");
    QAction *aboutAction = helpMenu->addAction("关于");

    connect(searchButton, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onResetSearch);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddStudent);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditStudent);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteStudent);
    connect(sortAscButton, &QPushButton::clicked, this, &MainWindow::onSortAscending);
    connect(sortDescButton, &QPushButton::clicked, this, &MainWindow::onSortDescending);
    connect(statisticsButton, &QPushButton::clicked, this, &MainWindow::onStatistics);

    connect(m_tableWidget, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onTableDoubleClicked);

    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this,
                           "关于",
                           "学生成绩管理系统\n\n"
                           "功能：增删改查、排序、统计、JSON 持久化存储。\n"
                           "技术：Qt Widgets、QFile、QJsonDocument、QList、信号与槽。");
    });
}

void MainWindow::setupStyle()
{
    setStyleSheet(
        "QMainWindow { background-color: #f4f7fb; }"
        "#titleLabel { font-size: 28px; font-weight: bold; color: #1f2937; padding: 12px; }"
        "QGroupBox { font-size: 15px; font-weight: bold; border: 1px solid #cbd5e1; border-radius: 8px; margin-top: 10px; padding: 12px; background: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
        "QLineEdit { height: 30px; border: 1px solid #cbd5e1; border-radius: 6px; padding-left: 8px; background: white; }"
        "QPushButton { min-width: 86px; height: 34px; border-radius: 7px; border: 1px solid #2563eb; background-color: #2563eb; color: white; font-weight: bold; }"
        "QPushButton:hover { background-color: #1d4ed8; }"
        "QPushButton:pressed { background-color: #1e40af; }"
        "QTableWidget { background: white; border: 1px solid #cbd5e1; border-radius: 8px; gridline-color: #e5e7eb; }"
        "QHeaderView::section { background-color: #eaf2ff; color: #1f2937; font-weight: bold; height: 34px; border: none; border-right: 1px solid #cbd5e1; }"
        "QLabel { color: #1f2937; }"
        );
}

void MainWindow::refreshTable()
{
    QList<int> indexes;
    for (int i = 0; i < m_manager.students().size(); ++i) {
        indexes.append(i);
    }

    refreshTableByIndexes(indexes);
}

void MainWindow::refreshTableByIndexes(const QList<int> &indexes)
{
    m_visibleIndexes = indexes.toVector();

    m_tableWidget->setRowCount(m_visibleIndexes.size());

    const QList<Student> &students = m_manager.students();

    for (int row = 0; row < m_visibleIndexes.size(); ++row) {
        int sourceIndex = m_visibleIndexes[row];

        if (sourceIndex < 0 || sourceIndex >= students.size()) {
            continue;
        }

        const Student &s = students[sourceIndex];

        QTableWidgetItem *idItem = new QTableWidgetItem(s.id);
        QTableWidgetItem *nameItem = new QTableWidgetItem(s.name);
        QTableWidgetItem *classItem = new QTableWidgetItem(s.className);
        QTableWidgetItem *courseItem = new QTableWidgetItem(s.course);
        QTableWidgetItem *scoreItem = new QTableWidgetItem(QString::number(s.score, 'f', 1));

        idItem->setTextAlignment(Qt::AlignCenter);
        nameItem->setTextAlignment(Qt::AlignCenter);
        classItem->setTextAlignment(Qt::AlignCenter);
        courseItem->setTextAlignment(Qt::AlignCenter);
        scoreItem->setTextAlignment(Qt::AlignCenter);

        m_tableWidget->setItem(row, 0, idItem);
        m_tableWidget->setItem(row, 1, nameItem);
        m_tableWidget->setItem(row, 2, classItem);
        m_tableWidget->setItem(row, 3, courseItem);
        m_tableWidget->setItem(row, 4, scoreItem);
    }

    updateStatus();
}

int MainWindow::selectedStudentIndex() const
{
    int row = m_tableWidget->currentRow();

    if (row < 0 || row >= m_visibleIndexes.size()) {
        return -1;
    }

    return m_visibleIndexes[row];
}

void MainWindow::onAddStudent()
{
    StudentDialog dialog(this);
    dialog.setWindowTitle("添加学生成绩");

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString error;
    if (!m_manager.addStudent(dialog.student(), &error)) {
        showError(error);
        return;
    }

    refreshTable();
}

void MainWindow::onEditStudent()
{
    int index = selectedStudentIndex();

    if (index < 0) {
        showError("请先选择要修改的记录。");
        return;
    }

    StudentDialog dialog(this);
    dialog.setWindowTitle("修改学生成绩");
    dialog.setStudent(m_manager.students().at(index));

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString error;
    if (!m_manager.updateStudent(index, dialog.student(), &error)) {
        showError(error);
        return;
    }

    refreshTable();
}

void MainWindow::onDeleteStudent()
{
    int index = selectedStudentIndex();

    if (index < 0) {
        showError("请先选择要删除的记录。");
        return;
    }

    const Student &s = m_manager.students().at(index);

    int ret = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除这条记录吗？\n\n学号：%1\n姓名：%2\n课程：%3")
            .arg(s.id, s.name, s.course),
        QMessageBox::Yes | QMessageBox::No
        );

    if (ret != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!m_manager.removeStudent(index, &error)) {
        showError(error);
        return;
    }

    refreshTable();
}

void MainWindow::onSearch()
{
    QList<int> indexes = m_manager.search(
        m_idSearchEdit->text(),
        m_nameSearchEdit->text(),
        m_classSearchEdit->text(),
        m_courseSearchEdit->text()
        );

    refreshTableByIndexes(indexes);
}

void MainWindow::onResetSearch()
{
    m_idSearchEdit->clear();
    m_nameSearchEdit->clear();
    m_classSearchEdit->clear();
    m_courseSearchEdit->clear();

    refreshTable();
}

void MainWindow::onSortAscending()
{
    m_manager.sortByScoreAscending();
    onSearch();
}

void MainWindow::onSortDescending()
{
    m_manager.sortByScoreDescending();
    onSearch();
}

void MainWindow::onStatistics()
{
    bool ok = false;

    QString className = QInputDialog::getText(
        this,
        "统计成绩",
        "请输入班级：",
        QLineEdit::Normal,
        "",
        &ok
        );

    if (!ok) {
        return;
    }

    className = className.trimmed();

    QString course = QInputDialog::getText(
        this,
        "统计成绩",
        "请输入课程名称：",
        QLineEdit::Normal,
        "",
        &ok
        );

    if (!ok) {
        return;
    }

    course = course.trimmed();

    StudentManager::Statistics statistics;

    if (!m_manager.calculateStatistics(className, course, statistics)) {
        showError("没有找到该班级该课程的成绩记录，或输入内容为空。");
        return;
    }

    QString message = QString(
                          "班级：%1\n"
                          "课程：%2\n\n"
                          "人数：%3\n"
                          "平均分：%4\n"
                          "最高分：%5\n"
                          "最低分：%6\n"
                          "及格人数：%7\n"
                          "及格率：%8%"
                          )
                          .arg(className)
                          .arg(course)
                          .arg(statistics.count)
                          .arg(QString::number(statistics.average, 'f', 2))
                          .arg(QString::number(statistics.maxScore, 'f', 1))
                          .arg(QString::number(statistics.minScore, 'f', 1))
                          .arg(statistics.passCount)
                          .arg(QString::number(statistics.passRate, 'f', 2));

    QMessageBox::information(this, "统计结果", message);
}

void MainWindow::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)

    onEditStudent();
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::warning(this, "提示", message);
}

void MainWindow::updateStatus()
{
    m_statusLabel->setText(
        QString("总记录数：%1    当前显示：%2    数据文件：%3")
            .arg(m_manager.students().size())
            .arg(m_visibleIndexes.size())
            .arg(m_manager.dataFilePath())
        );
}