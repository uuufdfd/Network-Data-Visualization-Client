#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "StudentManager.h"

#include <QMainWindow>
#include <QVector>

class QLineEdit;
class QPushButton;
class QTableWidget;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onAddStudent();
    void onEditStudent();
    void onDeleteStudent();
    void onSearch();
    void onResetSearch();
    void onSortAscending();
    void onSortDescending();
    void onStatistics();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUi();
    void setupStyle();
    void refreshTable();
    void refreshTableByIndexes(const QList<int> &indexes);
    int selectedStudentIndex() const;
    void showError(const QString &message);
    void updateStatus();

private:
    StudentManager m_manager;
    QVector<int> m_visibleIndexes;

    QLineEdit *m_idSearchEdit;
    QLineEdit *m_nameSearchEdit;
    QLineEdit *m_classSearchEdit;
    QLineEdit *m_courseSearchEdit;

    QTableWidget *m_tableWidget;
    QLabel *m_statusLabel;
};

#endif