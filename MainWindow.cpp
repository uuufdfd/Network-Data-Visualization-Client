#include "MainWindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtGlobal>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupChart();
    setupStyle();

    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);
#else
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &MainWindow::onSocketError);
#endif

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshDisplay);
    m_refreshTimer->start();

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(3000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &MainWindow::tryReconnect);
    m_reconnectTimer->start();

    setUiConnected(false);
    updateConnectionStatus("未连接", "#94a3b8");
    updateDataStatus("normal");
}

MainWindow::~MainWindow()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
    }
}

void MainWindow::setupUi()
{
    setWindowTitle("网络数据可视化客户端");
    resize(1180, 760);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    QLabel *title = new QLabel("网络数据可视化客户端", this);
    title->setObjectName("TitleLabel");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QGroupBox *networkGroup = new QGroupBox("网络连接", this);
    QGridLayout *networkLayout = new QGridLayout(networkGroup);

    m_ipEdit = new QLineEdit("127.0.0.1", this);
    m_ipEdit->setPlaceholderText("服务器地址，例如 127.0.0.1");

    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(8080);

    m_connectButton = new QPushButton("连接", this);
    m_disconnectButton = new QPushButton("断开", this);
    m_autoReconnectCheck = new QCheckBox("断线自动重连", this);
    m_autoReconnectCheck->setChecked(true);

    m_connectionLabel = new QLabel("未连接", this);
    m_connectionLabel->setObjectName("ConnectionLabel");

    networkLayout->addWidget(new QLabel("服务器地址：", this), 0, 0);
    networkLayout->addWidget(m_ipEdit, 0, 1);
    networkLayout->addWidget(new QLabel("端口：", this), 0, 2);
    networkLayout->addWidget(m_portSpin, 0, 3);
    networkLayout->addWidget(m_connectButton, 0, 4);
    networkLayout->addWidget(m_disconnectButton, 0, 5);
    networkLayout->addWidget(m_autoReconnectCheck, 0, 6);
    networkLayout->addWidget(new QLabel("连接状态：", this), 1, 0);
    networkLayout->addWidget(m_connectionLabel, 1, 1, 1, 6);

    mainLayout->addWidget(networkGroup);

    QGroupBox *controlGroup = new QGroupBox("控制面板", this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    m_startButton = new QPushButton("开始接收", this);
    m_pauseButton = new QPushButton("暂停接收", this);
    m_clearButton = new QPushButton("清空显示", this);
    m_exportButton = new QPushButton("手动导出 CSV", this);

    m_refreshCombo = new QComboBox(this);
    m_refreshCombo->addItem("0.5 秒", 500);
    m_refreshCombo->addItem("1 秒", 1000);
    m_refreshCombo->addItem("2 秒", 2000);
    m_refreshCombo->addItem("3 秒", 3000);
    m_refreshCombo->addItem("5 秒", 5000);
    m_refreshCombo->setCurrentIndex(1);

    m_dataStatusLight = new QLabel(this);
    m_dataStatusLight->setFixedSize(18, 18);
    m_dataStatusText = new QLabel("数据状态：正常", this);

    m_countLabel = new QLabel("已接收：0 条", this);

    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_pauseButton);
    controlLayout->addWidget(m_clearButton);
    controlLayout->addWidget(m_exportButton);
    controlLayout->addSpacing(18);
    controlLayout->addWidget(new QLabel("刷新频率：", this));
    controlLayout->addWidget(m_refreshCombo);
    controlLayout->addStretch();
    controlLayout->addWidget(m_dataStatusLight);
    controlLayout->addWidget(m_dataStatusText);
    controlLayout->addSpacing(18);
    controlLayout->addWidget(m_countLabel);

    mainLayout->addWidget(controlGroup);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(12);

    m_chartView = new QChartView(this);
    m_chartView->setMinimumHeight(420);
    contentLayout->addWidget(m_chartView, 3);

    QGroupBox *tableGroup = new QGroupBox("最新 20 条数据", this);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels(QStringList() << "时间" << "温度 value1" << "湿度 value2" << "状态");
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);

    tableLayout->addWidget(m_table);

    contentLayout->addWidget(tableGroup, 2);
    mainLayout->addLayout(contentLayout);

    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::connectToServer);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::disconnectFromServer);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::startReceiving);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::pauseReceiving);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearDisplayData);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::exportCsvManually);
    connect(m_refreshCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::changeRefreshInterval);
}

void MainWindow::setupChart()
{
    m_value1Series = new QLineSeries(this);
    m_value2Series = new QLineSeries(this);

    m_value1Series->setName("value1 温度");
    m_value2Series->setName("value2 湿度");

    m_chart = new QChart();
    m_chart->addSeries(m_value1Series);
    m_chart->addSeries(m_value2Series);
    m_chart->setTitle("实时数据折线图");
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_axisX = new QValueAxis(this);
    m_axisX->setTitleText("最近数据点");
    m_axisX->setRange(0, 50);
    m_axisX->setLabelFormat("%d");

    m_axisY = new QValueAxis(this);
    m_axisY->setTitleText("数值");
    m_axisY->setRange(0, 100);
    m_axisY->setLabelFormat("%.1f");

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_value1Series->attachAxis(m_axisX);
    m_value1Series->attachAxis(m_axisY);
    m_value2Series->attachAxis(m_axisX);
    m_value2Series->attachAxis(m_axisY);

    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::setupStyle()
{
    setStyleSheet(
        "QMainWindow { background: #f4f7fb; }"
        "#TitleLabel { font-size: 30px; font-weight: 700; color: #0f172a; padding: 8px; }"
        "QGroupBox { background: white; border: 1px solid #dbe3ef; border-radius: 12px; margin-top: 10px; padding: 14px; font-weight: 600; color: #1e293b; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; }"
        "QLineEdit, QSpinBox, QComboBox { height: 32px; border: 1px solid #cbd5e1; border-radius: 8px; padding-left: 8px; background: white; }"
        "QPushButton { height: 34px; min-width: 92px; border: none; border-radius: 8px; background: #2563eb; color: white; font-weight: 600; }"
        "QPushButton:hover { background: #1d4ed8; }"
        "QPushButton:disabled { background: #94a3b8; }"
        "QTableWidget { background: white; border: 1px solid #dbe3ef; border-radius: 10px; gridline-color: #e2e8f0; }"
        "QHeaderView::section { background: #eaf2ff; border: none; border-right: 1px solid #dbe3ef; padding: 6px; font-weight: 700; color: #0f172a; }"
        "#ConnectionLabel { font-weight: 700; }"
    );
}

void MainWindow::connectToServer()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState ||
        m_socket->state() == QAbstractSocket::ConnectingState) {
        return;
    }

    m_manualDisconnect = false;
    m_buffer.clear();

    const QString ip = m_ipEdit->text().trimmed();
    const quint16 port = static_cast<quint16>(m_portSpin->value());

    updateConnectionStatus("正在连接 " + ip + ":" + QString::number(port), "#f59e0b");
    m_socket->connectToHost(ip, port);
}

void MainWindow::disconnectFromServer()
{
    m_manualDisconnect = true;
    m_socket->disconnectFromHost();

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }

    updateConnectionStatus("已断开", "#94a3b8");
    setUiConnected(false);
}

void MainWindow::onConnected()
{
    updateConnectionStatus("已连接", "#16a34a");
    setUiConnected(true);
}

void MainWindow::onDisconnected()
{
    setUiConnected(false);

    if (m_manualDisconnect) {
        updateConnectionStatus("已断开", "#94a3b8");
    } else {
        updateConnectionStatus("连接断开，等待自动重连", "#f97316");
    }
}

void MainWindow::onSocketError(QAbstractSocket::SocketError)
{
    if (!m_manualDisconnect) {
        updateConnectionStatus("连接错误：" + m_socket->errorString(), "#dc2626");
    }
}

void MainWindow::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    int newlineIndex = -1;
    while ((newlineIndex = m_buffer.indexOf('\n')) >= 0) {
        QByteArray line = m_buffer.left(newlineIndex).trimmed();
        m_buffer.remove(0, newlineIndex + 1);

        if (!line.isEmpty()) {
            processLine(line);
        }
    }
}

void MainWindow::processLine(const QByteArray &line)
{
    if (!m_receiving) {
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        updateDataStatus("error");
        return;
    }

    DataRecord record = DataRecord::fromJson(doc.object());
    appendRecord(record);
}

void MainWindow::appendRecord(const DataRecord &record)
{
    m_records.append(record);

    if (m_records.size() > 5000) {
        m_records.remove(0, m_records.size() - 5000);
    }

    updateDataStatus(record.status);
    saveRecordToDailyCsv(record);
}

void MainWindow::refreshDisplay()
{
    refreshChart();
    refreshTable();
    m_countLabel->setText(QString("已接收：%1 条").arg(m_records.size()));
}

void MainWindow::refreshChart()
{
    m_value1Series->clear();
    m_value2Series->clear();

    const int maxPoints = 50;
    const int start = qMax(0, m_records.size() - maxPoints);
    const int count = m_records.size() - start;

    double minY = 0.0;
    double maxY = 100.0;
    bool first = true;

    for (int i = 0; i < count; ++i) {
        const DataRecord &r = m_records[start + i];

        m_value1Series->append(i, r.value1);
        m_value2Series->append(i, r.value2);

        if (first) {
            minY = qMin(r.value1, r.value2);
            maxY = qMax(r.value1, r.value2);
            first = false;
        } else {
            minY = qMin(minY, qMin(r.value1, r.value2));
            maxY = qMax(maxY, qMax(r.value1, r.value2));
        }
    }

    m_axisX->setRange(0, qMax(10, maxPoints));

    if (!first) {
        const double margin = qMax(5.0, (maxY - minY) * 0.15);
        m_axisY->setRange(qMax(0.0, minY - margin), maxY + margin);
    }
}

void MainWindow::refreshTable()
{
    const int maxRows = 20;
    const int start = qMax(0, m_records.size() - maxRows);
    const int count = m_records.size() - start;

    m_table->setRowCount(count);

    for (int row = 0; row < count; ++row) {
        const DataRecord &r = m_records[m_records.size() - 1 - row];
        QStringList values = r.toStringList();

        for (int col = 0; col < values.size(); ++col) {
            QString text = values[col];
            if (col == 3) {
                text = statusChinese(text);
            }

            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, col, item);
        }
    }
}

void MainWindow::startReceiving()
{
    m_receiving = true;
    m_startButton->setEnabled(false);
    m_pauseButton->setEnabled(true);
}

void MainWindow::pauseReceiving()
{
    m_receiving = false;
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
}

void MainWindow::clearDisplayData()
{
    m_records.clear();
    refreshDisplay();
    updateDataStatus("normal");
}

void MainWindow::exportCsvManually()
{
    if (m_records.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有可导出的数据。");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this,
        "导出 CSV 文件",
        QDir::homePath() + "/network_data_export.csv",
        "CSV 文件 (*.csv)"
    );

    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, "错误", "无法导出文件：" + file.errorString());
        return;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out << "timestamp,value1,value2,status\n";

    for (const DataRecord &record : qAsConst(m_records)) {
        out << record.toCsvLine();
    }

    file.close();
    QMessageBox::information(this, "完成", "CSV 导出成功。");
}

void MainWindow::changeRefreshInterval(int index)
{
    int interval = m_refreshCombo->itemData(index).toInt();
    if (interval <= 0) {
        interval = 1000;
    }

    m_refreshTimer->setInterval(interval);
}

void MainWindow::tryReconnect()
{
    if (m_manualDisconnect || !m_autoReconnectCheck->isChecked()) {
        return;
    }

    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        connectToServer();
    }
}

void MainWindow::updateConnectionStatus(const QString &text, const QString &color)
{
    m_connectionLabel->setText(text);
    m_connectionLabel->setStyleSheet("color: " + color + "; font-weight: 700;");
}

void MainWindow::updateDataStatus(const QString &status)
{
    QString color = "#16a34a";
    QString text = "正常";

    if (status == "warning" || status == "warn") {
        color = "#f59e0b";
        text = "警告";
    } else if (status == "error") {
        color = "#dc2626";
        text = "错误";
    }

    m_dataStatusLight->setStyleSheet(
        QString("background:%1; border-radius:9px; border:1px solid rgba(0,0,0,0.15);").arg(color)
    );
    m_dataStatusText->setText("数据状态：" + text);
}

void MainWindow::saveRecordToDailyCsv(const DataRecord &record)
{
    QString path = dailyCsvPath(record.timestamp.date());
    ensureCsvHeader(path);

    QFile file(path);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out << record.toCsvLine();
    file.close();
}

QString MainWindow::dailyCsvPath(const QDate &date) const
{
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("data")) {
        dir.mkdir("data");
    }

    return dir.filePath("data/data_" + date.toString("yyyyMMdd") + ".csv");
}

void MainWindow::ensureCsvHeader(const QString &path)
{
    QFile file(path);

    if (file.exists() && file.size() > 0) {
        return;
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        out.setCodec("UTF-8");
#endif
        out << "timestamp,value1,value2,status\n";
        file.close();
    }
}

void MainWindow::setUiConnected(bool connected)
{
    m_connectButton->setEnabled(!connected);
    m_disconnectButton->setEnabled(connected);
    m_ipEdit->setEnabled(!connected);
    m_portSpin->setEnabled(!connected);

    m_startButton->setEnabled(!m_receiving);
    m_pauseButton->setEnabled(m_receiving);
}

QString MainWindow::statusChinese(const QString &status) const
{
    QString s = status.trimmed().toLower();

    if (s == "warning" || s == "warn") {
        return "警告";
    }

    if (s == "error") {
        return "错误";
    }

    return "正常";
}
