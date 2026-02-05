#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QLabel>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QScrollBar>
#include "modbusclient.h"

// 注释掉Qt Charts命名空间
// QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // UI组件
    QWidget *centralWidget;
    QSplitter *splitter;
    QTabWidget *tabWidget;

    // 左侧控件
    QWidget *leftWidget;
    QVBoxLayout *leftLayout;

    // 串口连接组
    QGroupBox *connectionGroupBox;
    QGridLayout *connectionLayout;
    QLabel *portLabel;
    QComboBox *portComboBox;
    QPushButton *scanPortsButton;
    QLabel *baudRateLabel;
    QComboBox *baudRateComboBox;
    QLabel *dataBitsLabel;
    QComboBox *dataBitsComboBox;
    QLabel *stopBitsLabel;
    QComboBox *stopBitsComboBox;
    QLabel *parityLabel;
    QComboBox *parityComboBox;
    QPushButton *connectButton;
    QPushButton *disconnectButton;

    // 读取操作组
    QGroupBox *readGroupBox;
    QGridLayout *readLayout;
    QLabel *slaveAddressLabel;
    QSpinBox *slaveAddressSpin;
    QLabel *startAddressLabel;
    QSpinBox *startAddressSpin;
    QLabel *quantityLabel;
    QSpinBox *quantitySpin;
    QPushButton *readCoilsButton;
    QPushButton *readDiscreteInputsButton;
    QPushButton *readHoldingRegistersButton;
    QPushButton *readInputRegistersButton;

    // 写入操作组
    QGroupBox *writeGroupBox;
    QGridLayout *writeLayout;
    QLabel *writeAddressLabel;
    QSpinBox *writeAddressSpin;
    QLabel *coilValueLabel;
    QComboBox *coilValueComboBox;
    QLabel *registerValueLabel;
    QSpinBox *registerValueSpin;
    QPushButton *writeSingleCoilButton;
    QPushButton *writeSingleRegisterButton;

    // 自动轮询组
    QGroupBox *pollingGroupBox;
    QGridLayout *pollingLayout;
    QLabel *pollIntervalLabel;
    QSpinBox *pollIntervalSpin;
    QLabel *pollSlaveAddressLabel;
    QSpinBox *pollSlaveAddressSpin;
    QLabel *pollFunctionLabel;
    QComboBox *pollFunctionComboBox;
    QLabel *pollStartAddressLabel;
    QSpinBox *pollStartAddressSpin;
    QLabel *pollQuantityLabel;
    QSpinBox *pollQuantitySpin;
    QPushButton *addPollingTaskButton;
    QPushButton *startPollingButton;
    QPushButton *stopPollingButton;

    // 右侧表格
    QWidget *coilsTab;
    QVBoxLayout *coilsLayout;
    QTableWidget *coilsTable;

    QWidget *discreteInputsTab;
    QVBoxLayout *discreteInputsLayout;
    QTableWidget *discreteInputsTable;

    QWidget *holdingRegistersTab;
    QVBoxLayout *holdingRegistersLayout;
    QTableWidget *holdingRegistersTable;

    QWidget *inputRegistersTab;
    QVBoxLayout *inputRegistersLayout;
    QTableWidget *inputRegistersTable;

    // 图表Tab（暂时用QLabel代替）
    QWidget *chartTab;
    QVBoxLayout *chartLayout;
    QLabel *chartLabel;  // 改为QLabel

    // 日志
    QWidget *logTab;
    QVBoxLayout *logLayout;
    QTextEdit *logTextEdit;

    // 状态栏
    QStatusBar *statusBar;
    QLabel *connectionStatusLabel;

    // Modbus客户端
    ModbusClient *m_modbusClient;

    void setupUI();
    // void setupChart();  // 暂时注释
    void populateSerialPortSettings();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onScanPortsClicked();
    void onReadCoilsClicked();
    void onReadDiscreteInputsClicked();
    void onReadHoldingRegistersClicked();
    void onReadInputRegistersClicked();
    void onWriteSingleCoilClicked();
    void onWriteSingleRegisterClicked();
    void onStartPollingClicked();
    void onStopPollingClicked();
    void onAddPollingTaskClicked();
    void onConnectionStatusChanged(bool connected);
    void onDataReceived(int slaveAddress, ModbusClient::ModbusFunction function,
                        const QVector<quint16> &data);
    void onCoilsReceived(int slaveAddress, const QVector<bool> &coils);
    void onDiscreteInputsReceived(int slaveAddress, const QVector<bool> &inputs);
    void onHoldingRegistersReceived(int slaveAddress, const QVector<quint16> &registers);
    void onInputRegistersReceived(int slaveAddress, const QVector<quint16> &registers);
    void onErrorOccurred(const QString &error);
    void updateCoilsTable(const QVector<bool> &coils, int startAddress);
    void updateDiscreteInputsTable(const QVector<bool> &inputs, int startAddress);
    void updateHoldingRegistersTable(const QVector<quint16> &registers, int startAddress);
    void updateInputRegistersTable(const QVector<quint16> &registers, int startAddress);
    void logMessage(const QString &message, bool isError = false);
};
#endif // MAINWINDOW_H
