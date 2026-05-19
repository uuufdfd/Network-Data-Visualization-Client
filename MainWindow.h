#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "DataRecord.h"

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QComboBox;
class QTableWidget;
class QCheckBox;
QT_END_NAMESPACE

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void connectToServer();
    void disconnectFromServer();
    void onConnected();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onReadyRead();

    void startReceiving();
    void pauseReceiving();
    void clearDisplayData();
    void exportCsvManually();
    void changeRefreshInterval(int index);
    void tryReconnect();

private:
    void setupUi();
    void setupChart();
    void setupStyle();

    void processLine(const QByteArray &line);
    void appendRecord(const DataRecord &record);
    void refreshDisplay();
    void refreshChart();
    void refreshTable();
    void updateConnectionStatus(const QString &text, const QString &color);
    void updateDataStatus(const QString &status);
    void saveRecordToDailyCsv(const DataRecord &record);
    QString dailyCsvPath(const QDate &date) const;
    void ensureCsvHeader(const QString &path);
    void setUiConnected(bool connected);
    QString statusChinese(const QString &status) const;

private:
    QTcpSocket *m_socket = nullptr;
    QByteArray m_buffer;

    QVector<DataRecord> m_records;

    bool m_receiving = true;
    bool m_manualDisconnect = false;

    QTimer *m_refreshTimer = nullptr;
    QTimer *m_reconnectTimer = nullptr;

    QLineEdit *m_ipEdit = nullptr;
    QSpinBox *m_portSpin = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;
    QPushButton *m_startButton = nullptr;
    QPushButton *m_pauseButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_exportButton = nullptr;
    QComboBox *m_refreshCombo = nullptr;
    QCheckBox *m_autoReconnectCheck = nullptr;

    QLabel *m_connectionLabel = nullptr;
    QLabel *m_dataStatusLight = nullptr;
    QLabel *m_dataStatusText = nullptr;
    QLabel *m_countLabel = nullptr;

    QTableWidget *m_table = nullptr;

    QChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QLineSeries *m_value1Series = nullptr;
    QLineSeries *m_value2Series = nullptr;
    QValueAxis *m_axisX = nullptr;
    QValueAxis *m_axisY = nullptr;
};

#endif
