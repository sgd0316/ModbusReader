#include "mainwindow.h"
#include "modbusclient.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QHeaderView>
#include <QtCharts/QValueAxis>
#include <QSpacerItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_modbusClient(new ModbusClient(this))
{
    setWindowTitle("Modbus RTU 上位机");
    resize(1200, 800);

    setupUI();
    //setupChart();
    populateSerialPortSettings();

    // 连接Modbus客户端信号
    connect(m_modbusClient, &ModbusClient::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    connect(m_modbusClient, &ModbusClient::dataReceived,
            this, &MainWindow::onDataReceived);
    connect(m_modbusClient, &ModbusClient::coilsReceived,
            this, &MainWindow::onCoilsReceived);
    connect(m_modbusClient, &ModbusClient::discreteInputsReceived,
            this, &MainWindow::onDiscreteInputsReceived);
    connect(m_modbusClient, &ModbusClient::holdingRegistersReceived,
            this, &MainWindow::onHoldingRegistersReceived);
    connect(m_modbusClient, &ModbusClient::inputRegistersReceived,
            this, &MainWindow::onInputRegistersReceived);
    connect(m_modbusClient, &ModbusClient::errorOccurred,
            this, &MainWindow::onErrorOccurred);

    logMessage("Modbus RTU 上位机已启动");
}

MainWindow::~MainWindow()
{
    // 所有Qt对象都会自动删除，因为设置了父对象
}

void MainWindow::setupUI()
{
    // 创建中心部件
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // 创建分割器
    splitter = new QSplitter(Qt::Horizontal, centralWidget);
    mainLayout->addWidget(splitter);

    // ============ 左侧控件 ============
    leftWidget = new QWidget(splitter);
    leftLayout = new QVBoxLayout(leftWidget);

    // ----- 串口连接组 -----
    connectionGroupBox = new QGroupBox("串口连接", leftWidget);
    connectionLayout = new QGridLayout(connectionGroupBox);

    portLabel = new QLabel("串口:", connectionGroupBox);
    portComboBox = new QComboBox(connectionGroupBox);
    scanPortsButton = new QPushButton("扫描", connectionGroupBox);

    baudRateLabel = new QLabel("波特率:", connectionGroupBox);
    baudRateComboBox = new QComboBox(connectionGroupBox);

    dataBitsLabel = new QLabel("数据位:", connectionGroupBox);
    dataBitsComboBox = new QComboBox(connectionGroupBox);

    stopBitsLabel = new QLabel("停止位:", connectionGroupBox);
    stopBitsComboBox = new QComboBox(connectionGroupBox);

    parityLabel = new QLabel("校验位:", connectionGroupBox);
    parityComboBox = new QComboBox(connectionGroupBox);

    connectButton = new QPushButton("连接", connectionGroupBox);
    disconnectButton = new QPushButton("断开", connectionGroupBox);
    disconnectButton->setEnabled(false);

    // 添加到布局
    connectionLayout->addWidget(portLabel, 0, 0);
    connectionLayout->addWidget(portComboBox, 0, 1);
    connectionLayout->addWidget(scanPortsButton, 0, 2);
    connectionLayout->addWidget(baudRateLabel, 1, 0);
    connectionLayout->addWidget(baudRateComboBox, 1, 1);
    connectionLayout->addWidget(dataBitsLabel, 2, 0);
    connectionLayout->addWidget(dataBitsComboBox, 2, 1);
    connectionLayout->addWidget(stopBitsLabel, 3, 0);
    connectionLayout->addWidget(stopBitsComboBox, 3, 1);
    connectionLayout->addWidget(parityLabel, 4, 0);
    connectionLayout->addWidget(parityComboBox, 4, 1);
    connectionLayout->addWidget(connectButton, 5, 0);
    connectionLayout->addWidget(disconnectButton, 5, 1);

    // ----- 读取操作组 -----
    readGroupBox = new QGroupBox("读取操作", leftWidget);
    readLayout = new QGridLayout(readGroupBox);

    slaveAddressLabel = new QLabel("从站地址:", readGroupBox);
    slaveAddressSpin = new QSpinBox(readGroupBox);
    slaveAddressSpin->setRange(1, 247);
    slaveAddressSpin->setValue(1);

    startAddressLabel = new QLabel("起始地址:", readGroupBox);
    startAddressSpin = new QSpinBox(readGroupBox);
    startAddressSpin->setRange(0, 65535);

    quantityLabel = new QLabel("数量:", readGroupBox);
    quantitySpin = new QSpinBox(readGroupBox);
    quantitySpin->setRange(1, 125);
    quantitySpin->setValue(10);

    readCoilsButton = new QPushButton("读线圈", readGroupBox);
    readDiscreteInputsButton = new QPushButton("读离散输入", readGroupBox);
    readHoldingRegistersButton = new QPushButton("读保持寄存器", readGroupBox);
    readInputRegistersButton = new QPushButton("读输入寄存器", readGroupBox);

    // 添加到布局
    readLayout->addWidget(slaveAddressLabel, 0, 0);
    readLayout->addWidget(slaveAddressSpin, 0, 1);
    readLayout->addWidget(startAddressLabel, 1, 0);
    readLayout->addWidget(startAddressSpin, 1, 1);
    readLayout->addWidget(quantityLabel, 2, 0);
    readLayout->addWidget(quantitySpin, 2, 1);

    QHBoxLayout *readButtons1 = new QHBoxLayout();
    readButtons1->addWidget(readCoilsButton);
    readButtons1->addWidget(readDiscreteInputsButton);
    readLayout->addLayout(readButtons1, 3, 0, 1, 2);

    QHBoxLayout *readButtons2 = new QHBoxLayout();
    readButtons2->addWidget(readHoldingRegistersButton);
    readButtons2->addWidget(readInputRegistersButton);
    readLayout->addLayout(readButtons2, 4, 0, 1, 2);

    // ----- 写入操作组 -----
    writeGroupBox = new QGroupBox("写入操作", leftWidget);
    writeLayout = new QGridLayout(writeGroupBox);

    writeAddressLabel = new QLabel("写入地址:", writeGroupBox);
    writeAddressSpin = new QSpinBox(writeGroupBox);
    writeAddressSpin->setRange(0, 65535);

    coilValueLabel = new QLabel("线圈值:", writeGroupBox);
    coilValueComboBox = new QComboBox(writeGroupBox);
    coilValueComboBox->addItem("OFF");
    coilValueComboBox->addItem("ON");

    registerValueLabel = new QLabel("寄存器值:", writeGroupBox);
    registerValueSpin = new QSpinBox(writeGroupBox);
    registerValueSpin->setRange(0, 65535);

    writeSingleCoilButton = new QPushButton("写单线圈", writeGroupBox);
    writeSingleRegisterButton = new QPushButton("写单寄存器", writeGroupBox);

    // 添加到布局
    writeLayout->addWidget(writeAddressLabel, 0, 0);
    writeLayout->addWidget(writeAddressSpin, 0, 1);
    writeLayout->addWidget(coilValueLabel, 1, 0);
    writeLayout->addWidget(coilValueComboBox, 1, 1);
    writeLayout->addWidget(registerValueLabel, 2, 0);
    writeLayout->addWidget(registerValueSpin, 2, 1);

    QHBoxLayout *writeButtons = new QHBoxLayout();
    writeButtons->addWidget(writeSingleCoilButton);
    writeButtons->addWidget(writeSingleRegisterButton);
    writeLayout->addLayout(writeButtons, 3, 0, 1, 2);

    // ----- 自动轮询组 -----
    pollingGroupBox = new QGroupBox("自动轮询", leftWidget);
    pollingLayout = new QGridLayout(pollingGroupBox);

    pollIntervalLabel = new QLabel("轮询间隔(ms):", pollingGroupBox);
    pollIntervalSpin = new QSpinBox(pollingGroupBox);
    pollIntervalSpin->setRange(100, 10000);
    pollIntervalSpin->setValue(1000);

    pollSlaveAddressLabel = new QLabel("从站地址:", pollingGroupBox);
    pollSlaveAddressSpin = new QSpinBox(pollingGroupBox);
    pollSlaveAddressSpin->setRange(1, 247);
    pollSlaveAddressSpin->setValue(1);

    pollFunctionLabel = new QLabel("功能码:", pollingGroupBox);
    pollFunctionComboBox = new QComboBox(pollingGroupBox);
    pollFunctionComboBox->addItem("读线圈");
    pollFunctionComboBox->addItem("读离散输入");
    pollFunctionComboBox->addItem("读保持寄存器");
    pollFunctionComboBox->addItem("读输入寄存器");

    pollStartAddressLabel = new QLabel("起始地址:", pollingGroupBox);
    pollStartAddressSpin = new QSpinBox(pollingGroupBox);
    pollStartAddressSpin->setRange(0, 65535);

    pollQuantityLabel = new QLabel("数量:", pollingGroupBox);
    pollQuantitySpin = new QSpinBox(pollingGroupBox);
    pollQuantitySpin->setRange(1, 125);
    pollQuantitySpin->setValue(10);

    addPollingTaskButton = new QPushButton("添加任务", pollingGroupBox);
    startPollingButton = new QPushButton("开始轮询", pollingGroupBox);
    stopPollingButton = new QPushButton("停止轮询", pollingGroupBox);

    // 添加到布局
    pollingLayout->addWidget(pollIntervalLabel, 0, 0);
    pollingLayout->addWidget(pollIntervalSpin, 0, 1);
    pollingLayout->addWidget(pollSlaveAddressLabel, 1, 0);
    pollingLayout->addWidget(pollSlaveAddressSpin, 1, 1);
    pollingLayout->addWidget(pollFunctionLabel, 2, 0);
    pollingLayout->addWidget(pollFunctionComboBox, 2, 1);
    pollingLayout->addWidget(pollStartAddressLabel, 3, 0);
    pollingLayout->addWidget(pollStartAddressSpin, 3, 1);
    pollingLayout->addWidget(pollQuantityLabel, 4, 0);
    pollingLayout->addWidget(pollQuantitySpin, 4, 1);

    QHBoxLayout *pollingButtons = new QHBoxLayout();
    pollingButtons->addWidget(addPollingTaskButton);
    pollingButtons->addWidget(startPollingButton);
    pollingButtons->addWidget(stopPollingButton);
    pollingLayout->addLayout(pollingButtons, 5, 0, 1, 2);

    // 将左侧所有组添加到左侧布局
    leftLayout->addWidget(connectionGroupBox);
    leftLayout->addWidget(readGroupBox);
    leftLayout->addWidget(writeGroupBox);
    leftLayout->addWidget(pollingGroupBox);
    leftLayout->addStretch();

    // ============ 右侧Tab控件 ============
    tabWidget = new QTabWidget(splitter);

    // ----- 线圈状态Tab -----
    coilsTab = new QWidget();
    coilsLayout = new QVBoxLayout(coilsTab);
    coilsTable = new QTableWidget(coilsTab);
    coilsTable->setColumnCount(3);
    coilsTable->setHorizontalHeaderLabels(QStringList() << "地址" << "值" << "状态");
    coilsTable->setAlternatingRowColors(true);
    coilsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    coilsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    coilsLayout->addWidget(coilsTable);
    tabWidget->addTab(coilsTab, "线圈状态");

    // ----- 离散输入Tab -----
    discreteInputsTab = new QWidget();
    discreteInputsLayout = new QVBoxLayout(discreteInputsTab);
    discreteInputsTable = new QTableWidget(discreteInputsTab);
    discreteInputsTable->setColumnCount(3);
    discreteInputsTable->setHorizontalHeaderLabels(QStringList() << "地址" << "值" << "状态");
    discreteInputsTable->setAlternatingRowColors(true);
    discreteInputsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    discreteInputsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    discreteInputsLayout->addWidget(discreteInputsTable);
    tabWidget->addTab(discreteInputsTab, "离散输入");

    // ----- 保持寄存器Tab -----
    holdingRegistersTab = new QWidget();
    holdingRegistersLayout = new QVBoxLayout(holdingRegistersTab);
    holdingRegistersTable = new QTableWidget(holdingRegistersTab);
    holdingRegistersTable->setColumnCount(3);
    holdingRegistersTable->setHorizontalHeaderLabels(QStringList() << "地址" << "HEX" << "DEC");
    holdingRegistersTable->setAlternatingRowColors(true);
    holdingRegistersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    holdingRegistersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    holdingRegistersLayout->addWidget(holdingRegistersTable);
    tabWidget->addTab(holdingRegistersTab, "保持寄存器");

    // ----- 输入寄存器Tab -----
    inputRegistersTab = new QWidget();
    inputRegistersLayout = new QVBoxLayout(inputRegistersTab);
    inputRegistersTable = new QTableWidget(inputRegistersTab);
    inputRegistersTable->setColumnCount(3);
    inputRegistersTable->setHorizontalHeaderLabels(QStringList() << "地址" << "HEX" << "DEC");
    inputRegistersTable->setAlternatingRowColors(true);
    inputRegistersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputRegistersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    inputRegistersLayout->addWidget(inputRegistersTable);
    tabWidget->addTab(inputRegistersTab, "输入寄存器");

    // ----- 数据趋势Tab -----
    // ----- 数据趋势Tab -----
    chartTab = new QWidget();
    chartLayout = new QVBoxLayout(chartTab);

    // 暂时用QLabel代替图表
    QLabel *chartLabel = new QLabel("图表功能需要Qt Charts模块支持\n请确保已安装并启用Qt Charts模块", chartTab);
    chartLabel->setAlignment(Qt::AlignCenter);
    chartLayout->addWidget(chartLabel);

    tabWidget->addTab(chartTab, "数据趋势");

    // ----- 运行日志Tab -----
    logTab = new QWidget();
    logLayout = new QVBoxLayout(logTab);
    logTextEdit = new QTextEdit(logTab);
    logTextEdit->setReadOnly(true);
    logLayout->addWidget(logTextEdit);
    tabWidget->addTab(logTab, "运行日志");

    // 设置分割器大小
    splitter->addWidget(leftWidget);
    splitter->addWidget(tabWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    // ============ 状态栏 ============
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    connectionStatusLabel = new QLabel("未连接", statusBar);
    connectionStatusLabel->setStyleSheet("color: red;");
    statusBar->addWidget(connectionStatusLabel);

    // 连接按钮信号
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    connect(scanPortsButton, &QPushButton::clicked, this, &MainWindow::onScanPortsClicked);

    connect(readCoilsButton, &QPushButton::clicked, this, &MainWindow::onReadCoilsClicked);
    connect(readDiscreteInputsButton, &QPushButton::clicked, this, &MainWindow::onReadDiscreteInputsClicked);
    connect(readHoldingRegistersButton, &QPushButton::clicked, this, &MainWindow::onReadHoldingRegistersClicked);
    connect(readInputRegistersButton, &QPushButton::clicked, this, &MainWindow::onReadInputRegistersClicked);

    connect(writeSingleCoilButton, &QPushButton::clicked, this, &MainWindow::onWriteSingleCoilClicked);
    connect(writeSingleRegisterButton, &QPushButton::clicked, this, &MainWindow::onWriteSingleRegisterClicked);

    connect(startPollingButton, &QPushButton::clicked, this, &MainWindow::onStartPollingClicked);
    connect(stopPollingButton, &QPushButton::clicked, this, &MainWindow::onStopPollingClicked);
    connect(addPollingTaskButton, &QPushButton::clicked, this, &MainWindow::onAddPollingTaskClicked);
}

// void MainWindow::setupChart()
// {
//     chart = new QChart();
//     series = new QLineSeries();

//     chart->addSeries(series);
//     chart->setTitle("寄存器数据趋势图");
//     chart->setAnimationOptions(QChart::SeriesAnimations);

//     QValueAxis *axisX = new QValueAxis;
//     axisX->setTitleText("时间");
//     axisX->setLabelFormat("%d");
//     chart->addAxis(axisX, Qt::AlignBottom);
//     series->attachAxis(axisX);

//     QValueAxis *axisY = new QValueAxis;
//     axisY->setTitleText("值");
//     chart->addAxis(axisY, Qt::AlignLeft);
//     series->attachAxis(axisY);

//     chartView->setChart(chart);
//     chartView->setRenderHint(QPainter::Antialiasing);
// }

void MainWindow::populateSerialPortSettings()
{
    // 获取可用串口
    portComboBox->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        portComboBox->addItem(info.portName());
    }

    // 波特率
    baudRateComboBox->clear();
    QList<qint32> baudRates = {9600, 19200, 38400, 57600, 115200};
    for (qint32 baud : baudRates) {
        baudRateComboBox->addItem(QString::number(baud), baud);
    }
    baudRateComboBox->setCurrentIndex(0);

    // 数据位
    dataBitsComboBox->clear();
    dataBitsComboBox->addItem("5", QSerialPort::Data5);
    dataBitsComboBox->addItem("6", QSerialPort::Data6);
    dataBitsComboBox->addItem("7", QSerialPort::Data7);
    dataBitsComboBox->addItem("8", QSerialPort::Data8);
    dataBitsComboBox->setCurrentIndex(3);

    // 停止位
    stopBitsComboBox->clear();
    stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
    stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
    stopBitsComboBox->setCurrentIndex(0);

    // 校验位
    parityComboBox->clear();
    parityComboBox->addItem("无", QSerialPort::NoParity);
    parityComboBox->addItem("奇校验", QSerialPort::OddParity);
    parityComboBox->addItem("偶校验", QSerialPort::EvenParity);
    parityComboBox->setCurrentIndex(0);
}

void MainWindow::onConnectClicked()
{
    if (portComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择串口");
        return;
    }

    QString port = portComboBox->currentText();
    int baudRate = baudRateComboBox->currentData().toInt();
    QSerialPort::DataBits dataBits = static_cast<QSerialPort::DataBits>(
        dataBitsComboBox->currentData().toInt());
    QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(
        parityComboBox->currentData().toInt());
    QSerialPort::StopBits stopBits = static_cast<QSerialPort::StopBits>(
        stopBitsComboBox->currentData().toInt());

    logMessage(QString("正在连接: %1, %2 baud").arg(port).arg(baudRate));

    if (m_modbusClient->connectDevice(port, baudRate, dataBits, parity, stopBits)) {
        connectionStatusLabel->setText("已连接");
        connectionStatusLabel->setStyleSheet("color: green;");
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
    }
}

void MainWindow::onDisconnectClicked()
{
    m_modbusClient->disconnectDevice();
    connectionStatusLabel->setText("未连接");
    connectionStatusLabel->setStyleSheet("color: red;");
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    logMessage("已断开连接");
}

void MainWindow::onScanPortsClicked()
{
    populateSerialPortSettings();
    logMessage("已刷新串口列表");
}

void MainWindow::onReadCoilsClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int slaveAddress = slaveAddressSpin->value();
    int startAddress = startAddressSpin->value();
    int quantity = quantitySpin->value();

    logMessage(QString("读取线圈: 从站=%1, 起始地址=%2, 数量=%3")
                   .arg(slaveAddress).arg(startAddress).arg(quantity));

    m_modbusClient->readCoils(slaveAddress, startAddress, quantity);
}

void MainWindow::onReadDiscreteInputsClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int slaveAddress = slaveAddressSpin->value();
    int startAddress = startAddressSpin->value();
    int quantity = quantitySpin->value();

    logMessage(QString("读取离散输入: 从站=%1, 起始地址=%2, 数量=%3")
                   .arg(slaveAddress).arg(startAddress).arg(quantity));

    m_modbusClient->readDiscreteInputs(slaveAddress, startAddress, quantity);
}

void MainWindow::onReadHoldingRegistersClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int slaveAddress = slaveAddressSpin->value();
    int startAddress = startAddressSpin->value();
    int quantity = quantitySpin->value();

    logMessage(QString("读取保持寄存器: 从站=%1, 起始地址=%2, 数量=%3")
                   .arg(slaveAddress).arg(startAddress).arg(quantity));

    m_modbusClient->readHoldingRegisters(slaveAddress, startAddress, quantity);
}

void MainWindow::onReadInputRegistersClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int slaveAddress = slaveAddressSpin->value();
    int startAddress = startAddressSpin->value();
    int quantity = quantitySpin->value();

    logMessage(QString("读取输入寄存器: 从站=%1, 起始地址=%2, 数量=%3")
                   .arg(slaveAddress).arg(startAddress).arg(quantity));

    m_modbusClient->readInputRegisters(slaveAddress, startAddress, quantity);
}

void MainWindow::onWriteSingleCoilClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int slaveAddress = slaveAddressSpin->value();
    int address = writeAddressSpin->value();
    bool value = coilValueComboBox->currentText() == "ON";

    logMessage(QString("写入单线圈: 从站=%1, 地址=%2, 值=%3")
                   .arg(slaveAddress).arg(address).arg(value ? "ON" : "OFF"));

    m_modbusClient->writeSingleCoil(slaveAddress, address, value);
}

void MainWindow::onWriteSingleRegisterClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int slaveAddress = slaveAddressSpin->value();
    int address = writeAddressSpin->value();
    quint16 value = static_cast<quint16>(registerValueSpin->value());

    logMessage(QString("写入单寄存器: 从站=%1, 地址=%2, 值=%3")
                   .arg(slaveAddress).arg(address).arg(value));

    m_modbusClient->writeSingleRegister(slaveAddress, address, value);
}

void MainWindow::onStartPollingClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int interval = pollIntervalSpin->value();
    m_modbusClient->startAutoPolling(interval);
    logMessage(QString("开始自动轮询，间隔: %1 ms").arg(interval));
}

void MainWindow::onStopPollingClicked()
{
    m_modbusClient->stopAutoPolling();
    logMessage("停止自动轮询");
}

void MainWindow::onAddPollingTaskClicked()
{
    if (!m_modbusClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接设备");
        return;
    }

    int slaveAddress = pollSlaveAddressSpin->value();
    int functionCode = pollFunctionComboBox->currentIndex();
    int startAddress = pollStartAddressSpin->value();
    int quantity = pollQuantitySpin->value();

    ModbusClient::ModbusRequest request;
    request.slaveAddress = slaveAddress;
    request.startAddress = startAddress;
    request.quantity = quantity;

    switch (functionCode) {
    case 0:
        request.function = ModbusClient::ReadCoils;
        break;
    case 1:
        request.function = ModbusClient::ReadDiscreteInputs;
        break;
    case 2:
        request.function = ModbusClient::ReadHoldingRegisters;
        break;
    case 3:
        request.function = ModbusClient::ReadInputRegisters;
        break;
    }

    m_modbusClient->addPollingRequest(request);

    QString functionStr;
    switch (functionCode) {
    case 0: functionStr = "线圈"; break;
    case 1: functionStr = "离散输入"; break;
    case 2: functionStr = "保持寄存器"; break;
    case 3: functionStr = "输入寄存器"; break;
    }

    logMessage(QString("添加轮询任务: 从站=%1, 功能=%2, 起始地址=%3, 数量=%4")
                   .arg(slaveAddress).arg(functionStr).arg(startAddress).arg(quantity));
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    if (connected) {
        connectionStatusLabel->setText("已连接");
        connectionStatusLabel->setStyleSheet("color: green;");
        logMessage("设备连接成功");
    } else {
        connectionStatusLabel->setText("未连接");
        connectionStatusLabel->setStyleSheet("color: red;");
        logMessage("设备断开连接");
    }

    connectButton->setEnabled(!connected);
    disconnectButton->setEnabled(connected);
}

void MainWindow::onDataReceived(int slaveAddress, ModbusClient::ModbusFunction function,
                                const QVector<quint16> &data)
{
    QString functionStr;
    switch (function) {
    case ModbusClient::ReadCoils:
        functionStr = "读取线圈";
        break;
    case ModbusClient::ReadDiscreteInputs:
        functionStr = "读取离散输入";
        break;
    case ModbusClient::ReadHoldingRegisters:
        functionStr = "读取保持寄存器";
        break;
    case ModbusClient::ReadInputRegisters:
        functionStr = "读取输入寄存器";
        break;
    case ModbusClient::WriteSingleCoil:
        functionStr = "写入单线圈";
        break;
    case ModbusClient::WriteSingleRegister:
        functionStr = "写入单寄存器";
        break;
    default:
        functionStr = "未知功能";
        break;
    }

    QString dataStr;
    for (int i = 0; i < data.size(); ++i) {
        dataStr += QString::number(data[i]) + " ";
        if (i >= 9) { // 只显示前10个数据
            dataStr += "...";
            break;
        }
    }

    logMessage(QString("收到数据: 从站=%1, 功能=%2, 数据=[%3]")
                   .arg(slaveAddress).arg(functionStr).arg(dataStr.trimmed()));
}

void MainWindow::onCoilsReceived(int slaveAddress, const QVector<bool> &coils)
{
    int startAddress = startAddressSpin->value();
    updateCoilsTable(coils, startAddress);

    // 暂时注释图表更新
    /*
    // 更新图表（取第一个线圈的值作为示例）
    if (!coils.isEmpty()) {
        static int timePoint = 0;
        series->append(timePoint++, coils.first() ? 1 : 0);
        if (series->count() > 50) {
            series->remove(0);
        }
        chart->axisX()->setRange(0, timePoint);
    }
    */
}

void MainWindow::onDiscreteInputsReceived(int slaveAddress, const QVector<bool> &inputs)
{
    int startAddress = startAddressSpin->value();
    updateDiscreteInputsTable(inputs, startAddress);
}

// 修改onHoldingRegistersReceived函数，移除图表更新
void MainWindow::onHoldingRegistersReceived(int slaveAddress, const QVector<quint16> &registers)
{
    int startAddress = startAddressSpin->value();
    updateHoldingRegistersTable(registers, startAddress);

    // 暂时注释图表更新
    /*
    // 更新图表（取第一个寄存器的值）
    if (!registers.isEmpty()) {
        static int timePoint = 0;
        series->append(timePoint++, registers.first());
        if (series->count() > 50) {
            series->remove(0);
        }
        chart->axisX()->setRange(0, timePoint);

        // 动态调整Y轴范围
        auto minMax = std::minmax_element(registers.begin(), registers.end());
        int minVal = *minMax.first;
        int maxVal = *minMax.second;

        // 添加一些边距
        int margin = (maxVal - minVal) * 0.1;
        if (margin < 1) margin = 1;

        chart->axisY()->setRange(minVal - margin, maxVal + margin);
    }
    */
}

void MainWindow::onInputRegistersReceived(int slaveAddress, const QVector<quint16> &registers)
{
    int startAddress = startAddressSpin->value();
    updateInputRegistersTable(registers, startAddress);
}

void MainWindow::onErrorOccurred(const QString &error)
{
    logMessage("错误: " + error, true);
    QMessageBox::warning(this, "错误", error);
}

void MainWindow::updateCoilsTable(const QVector<bool> &coils, int startAddress)
{
    coilsTable->setRowCount(coils.size());

    for (int i = 0; i < coils.size(); ++i) {
        int address = startAddress + i;
        bool value = coils[i];

        QTableWidgetItem *addrItem = new QTableWidgetItem(QString::number(address));
        QTableWidgetItem *valueItem = new QTableWidgetItem(value ? "1" : "0");
        QTableWidgetItem *statusItem = new QTableWidgetItem(value ? "ON" : "OFF");

        coilsTable->setItem(i, 0, addrItem);
        coilsTable->setItem(i, 1, valueItem);
        coilsTable->setItem(i, 2, statusItem);

        // 设置颜色
        statusItem->setForeground(value ? Qt::darkGreen : Qt::darkRed);
    }
}

void MainWindow::updateDiscreteInputsTable(const QVector<bool> &inputs, int startAddress)
{
    discreteInputsTable->setRowCount(inputs.size());

    for (int i = 0; i < inputs.size(); ++i) {
        int address = startAddress + i;
        bool value = inputs[i];

        QTableWidgetItem *addrItem = new QTableWidgetItem(QString::number(address));
        QTableWidgetItem *valueItem = new QTableWidgetItem(value ? "1" : "0");
        QTableWidgetItem *statusItem = new QTableWidgetItem(value ? "ON" : "OFF");

        discreteInputsTable->setItem(i, 0, addrItem);
        discreteInputsTable->setItem(i, 1, valueItem);
        discreteInputsTable->setItem(i, 2, statusItem);

        // 设置颜色
        statusItem->setForeground(value ? Qt::darkGreen : Qt::darkRed);
    }
}

void MainWindow::updateHoldingRegistersTable(const QVector<quint16> &registers, int startAddress)
{
    holdingRegistersTable->setRowCount(registers.size());

    for (int i = 0; i < registers.size(); ++i) {
        int address = startAddress + i;
        quint16 value = registers[i];

        QTableWidgetItem *addrItem = new QTableWidgetItem(QString::number(address));
        QTableWidgetItem *hexItem = new QTableWidgetItem(QString("0x%1").arg(value, 4, 16, QChar('0')).toUpper());
        QTableWidgetItem *decItem = new QTableWidgetItem(QString::number(value));

        holdingRegistersTable->setItem(i, 0, addrItem);
        holdingRegistersTable->setItem(i, 1, hexItem);
        holdingRegistersTable->setItem(i, 2, decItem);
    }
}

void MainWindow::updateInputRegistersTable(const QVector<quint16> &registers, int startAddress)
{
    inputRegistersTable->setRowCount(registers.size());

    for (int i = 0; i < registers.size(); ++i) {
        int address = startAddress + i;
        quint16 value = registers[i];

        QTableWidgetItem *addrItem = new QTableWidgetItem(QString::number(address));
        QTableWidgetItem *hexItem = new QTableWidgetItem(QString("0x%1").arg(value, 4, 16, QChar('0')).toUpper());
        QTableWidgetItem *decItem = new QTableWidgetItem(QString::number(value));

        inputRegistersTable->setItem(i, 0, addrItem);
        inputRegistersTable->setItem(i, 1, hexItem);
        inputRegistersTable->setItem(i, 2, decItem);
    }
}

void MainWindow::logMessage(const QString &message, bool isError)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp).arg(message);

    QTextCharFormat format;
    if (isError) {
        format.setForeground(QBrush(Qt::red));
    } else {
        format.setForeground(QBrush(Qt::black));
    }

    QTextCursor cursor(logTextEdit->textCursor());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(logEntry + "\n", format);

    // 自动滚动到底部
    logTextEdit->verticalScrollBar()->setValue(
        logTextEdit->verticalScrollBar()->maximum()
        );
}
