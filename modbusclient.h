#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>
#include <QModbusReply>
#include <QSerialPort>
#include <QTimer>
#include <QVector>
#include <QMap>

class ModbusClient : public QObject
{
    Q_OBJECT

public:
    enum ModbusFunction {
        ReadCoils = 0x01,
        ReadDiscreteInputs = 0x02,
        ReadHoldingRegisters = 0x03,
        ReadInputRegisters = 0x04,
        WriteSingleCoil = 0x05,
        WriteSingleRegister = 0x06,
        WriteMultipleCoils = 0x0F,
        WriteMultipleRegisters = 0x10
    };

    struct ModbusRequest {
        int slaveAddress;
        ModbusFunction function;
        quint16 startAddress;
        quint16 quantity;
        QVector<quint16> writeData;
    };

    explicit ModbusClient(QObject *parent = nullptr);
    ~ModbusClient();

    // 连接/断开
    bool connectDevice(const QString &port, int baudRate = QSerialPort::Baud9600,
                       QSerialPort::DataBits dataBits = QSerialPort::Data8,
                       QSerialPort::Parity parity = QSerialPort::NoParity,
                       QSerialPort::StopBits stopBits = QSerialPort::OneStop);
    void disconnectDevice();
    bool isConnected() const;

    // 读取操作
    bool readCoils(int slaveAddress, quint16 startAddress, quint16 quantity);
    bool readDiscreteInputs(int slaveAddress, quint16 startAddress, quint16 quantity);
    bool readHoldingRegisters(int slaveAddress, quint16 startAddress, quint16 quantity);
    bool readInputRegisters(int slaveAddress, quint16 startAddress, quint16 quantity);

    // 写入操作
    bool writeSingleCoil(int slaveAddress, quint16 address, bool value);
    bool writeSingleRegister(int slaveAddress, quint16 address, quint16 value);
    bool writeMultipleCoils(int slaveAddress, quint16 startAddress, const QVector<bool> &values);
    bool writeMultipleRegisters(int slaveAddress, quint16 startAddress, const QVector<quint16> &values);

    // 自动轮询
    void startAutoPolling(int intervalMs = 1000);
    void stopAutoPolling();
    void addPollingRequest(const ModbusRequest &request);
    void clearPollingRequests();

signals:
    void connectionStatusChanged(bool connected);
    void dataReceived(int slaveAddress, ModbusFunction function, const QVector<quint16> &data);
    void coilsReceived(int slaveAddress, const QVector<bool> &coils);
    void discreteInputsReceived(int slaveAddress, const QVector<bool> &inputs);
    void holdingRegistersReceived(int slaveAddress, const QVector<quint16> &registers);
    void inputRegistersReceived(int slaveAddress, const QVector<quint16> &registers);
    void errorOccurred(const QString &error);

private slots:
    // 移除未实现的槽函数声明
    // void onReadReady();  // 移除这行

    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);
    void onPollingTimeout();

private:
    QModbusRtuSerialMaster *m_modbusDevice;
    QTimer *m_pollingTimer;
    QList<ModbusRequest> m_pollingRequests;
    int m_currentPollIndex;

    void processReply(QModbusReply *reply, ModbusFunction expectedFunction);
    static QVector<bool> bytesToBits(const QByteArray &bytes, int bitCount);
};

#endif // MODBUSCLIENT_H
