#include "modbusclient.h"
#include <QDebug>

ModbusClient::ModbusClient(QObject *parent)
    : QObject(parent)
    , m_modbusDevice(nullptr)
    , m_currentPollIndex(0)
{
    m_pollingTimer = new QTimer(this);
    connect(m_pollingTimer, &QTimer::timeout, this, &ModbusClient::onPollingTimeout);
}

ModbusClient::~ModbusClient()
{
    disconnectDevice();
}

bool ModbusClient::connectDevice(const QString &port, int baudRate,
                                 QSerialPort::DataBits dataBits,
                                 QSerialPort::Parity parity,
                                 QSerialPort::StopBits stopBits)
{
    if (m_modbusDevice) {
        disconnectDevice();
    }

    m_modbusDevice = new QModbusRtuSerialMaster(this);

    connect(m_modbusDevice, &QModbusClient::stateChanged,
            this, &ModbusClient::onStateChanged);
    connect(m_modbusDevice, &QModbusClient::errorOccurred,
            this, &ModbusClient::onErrorOccurred);

    m_modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, port);
    m_modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudRate);
    m_modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
    m_modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
    m_modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);

    // 设置超时和重试
    m_modbusDevice->setTimeout(1000);
    m_modbusDevice->setNumberOfRetries(3);

    if (!m_modbusDevice->connectDevice()) {
        emit errorOccurred(tr("连接失败: ") + m_modbusDevice->errorString());
        delete m_modbusDevice;
        m_modbusDevice = nullptr;
        return false;
    }

    return true;
}

void ModbusClient::disconnectDevice()
{
    stopAutoPolling();

    if (m_modbusDevice) {
        if (m_modbusDevice->state() != QModbusDevice::UnconnectedState) {
            m_modbusDevice->disconnectDevice();
        }
        delete m_modbusDevice;
        m_modbusDevice = nullptr;
    }

    emit connectionStatusChanged(false);
}

bool ModbusClient::isConnected() const
{
    return m_modbusDevice && m_modbusDevice->state() == QModbusDevice::ConnectedState;
}

bool ModbusClient::readCoils(int slaveAddress, quint16 startAddress, quint16 quantity)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    if (quantity < 1 || quantity > 2000) {
        emit errorOccurred(tr("线圈数量必须在1-2000之间"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, startAddress, quantity);

    if (auto *reply = m_modbusDevice->sendReadRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                processReply(reply, ReadCoils);
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送读取线圈请求失败"));
    return false;
}

bool ModbusClient::readDiscreteInputs(int slaveAddress, quint16 startAddress, quint16 quantity)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    if (quantity < 1 || quantity > 2000) {
        emit errorOccurred(tr("离散输入数量必须在1-2000之间"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::DiscreteInputs, startAddress, quantity);

    if (auto *reply = m_modbusDevice->sendReadRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                processReply(reply, ReadDiscreteInputs);
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送读取离散输入请求失败"));
    return false;
}

bool ModbusClient::readHoldingRegisters(int slaveAddress, quint16 startAddress, quint16 quantity)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    if (quantity < 1 || quantity > 125) {
        emit errorOccurred(tr("保持寄存器数量必须在1-125之间"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, quantity);

    if (auto *reply = m_modbusDevice->sendReadRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                processReply(reply, ReadHoldingRegisters);
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送读取保持寄存器请求失败"));
    return false;
}

bool ModbusClient::readInputRegisters(int slaveAddress, quint16 startAddress, quint16 quantity)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    if (quantity < 1 || quantity > 125) {
        emit errorOccurred(tr("输入寄存器数量必须在1-125之间"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::InputRegisters, startAddress, quantity);

    if (auto *reply = m_modbusDevice->sendReadRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                processReply(reply, ReadInputRegisters);
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送读取输入寄存器请求失败"));
    return false;
}

bool ModbusClient::writeSingleCoil(int slaveAddress, quint16 address, bool value)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, address, 1);
    request.setValue(0, value ? 0xFF00 : 0x0000);

    if (auto *reply = m_modbusDevice->sendWriteRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                if (reply->error() == QModbusDevice::NoError) {
                    QVector<quint16> data;
                    data.append(value ? 1 : 0);
                    emit dataReceived(slaveAddress, WriteSingleCoil, data);
                } else {
                    emit errorOccurred(reply->errorString());
                }
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送写入线圈请求失败"));
    return false;
}

bool ModbusClient::writeSingleRegister(int slaveAddress, quint16 address, quint16 value)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, address, 1);
    request.setValue(0, value);

    if (auto *reply = m_modbusDevice->sendWriteRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                if (reply->error() == QModbusDevice::NoError) {
                    QVector<quint16> data;
                    data.append(value);
                    emit dataReceived(slaveAddress, WriteSingleRegister, data);
                } else {
                    emit errorOccurred(reply->errorString());
                }
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送写入寄存器请求失败"));
    return false;
}

bool ModbusClient::writeMultipleCoils(int slaveAddress, quint16 startAddress, const QVector<bool> &values)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    if (values.isEmpty() || values.size() > 1968) {
        emit errorOccurred(tr("线圈数量必须在1-1968之间"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, startAddress, static_cast<quint16>(values.size()));
    for (int i = 0; i < values.size(); ++i) {
        request.setValue(static_cast<quint16>(i), values[i] ? 0xFF00 : 0x0000);
    }

    if (auto *reply = m_modbusDevice->sendWriteRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                if (reply->error() == QModbusDevice::NoError) {
                    QVector<quint16> data;
                    for (bool value : values) {
                        data.append(value ? 1 : 0);
                    }
                    emit dataReceived(slaveAddress, WriteMultipleCoils, data);
                } else {
                    emit errorOccurred(reply->errorString());
                }
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送写入多个线圈请求失败"));
    return false;
}

bool ModbusClient::writeMultipleRegisters(int slaveAddress, quint16 startAddress, const QVector<quint16> &values)
{
    if (!isConnected()) {
        emit errorOccurred(tr("设备未连接"));
        return false;
    }

    if (values.isEmpty() || values.size() > 123) {
        emit errorOccurred(tr("寄存器数量必须在1-123之间"));
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, startAddress, static_cast<quint16>(values.size()));
    for (int i = 0; i < values.size(); ++i) {
        request.setValue(static_cast<quint16>(i), values[i]);
    }

    if (auto *reply = m_modbusDevice->sendWriteRequest(request, slaveAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [=]() {
                if (reply->error() == QModbusDevice::NoError) {
                    emit dataReceived(slaveAddress, WriteMultipleRegisters, values);
                } else {
                    emit errorOccurred(reply->errorString());
                }
                reply->deleteLater();
            });
            return true;
        } else {
            delete reply;
            return false;
        }
    }

    emit errorOccurred(tr("发送写入多个寄存器请求失败"));
    return false;
}

void ModbusClient::startAutoPolling(int intervalMs)
{
    if (!m_pollingRequests.isEmpty()) {
        m_pollingTimer->start(intervalMs);
    }
}

void ModbusClient::stopAutoPolling()
{
    m_pollingTimer->stop();
    m_currentPollIndex = 0;
}

void ModbusClient::addPollingRequest(const ModbusRequest &request)
{
    m_pollingRequests.append(request);
}

void ModbusClient::clearPollingRequests()
{
    m_pollingRequests.clear();
    stopAutoPolling();
}

void ModbusClient::processReply(QModbusReply *reply, ModbusFunction expectedFunction)
{
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        int slaveAddress = static_cast<int>(reply->serverAddress());

        QVector<quint16> data;
        for (int i = 0; i < static_cast<int>(unit.valueCount()); ++i) {
            data.append(unit.value(static_cast<quint16>(i)));
        }

        emit dataReceived(slaveAddress, expectedFunction, data);

        // 根据功能码发送特定信号
        switch (expectedFunction) {
        case ReadCoils:
            emit coilsReceived(slaveAddress, bytesToBits(reply->rawResult().data(),
                                                         static_cast<int>(unit.valueCount())));
            break;
        case ReadDiscreteInputs:
            emit discreteInputsReceived(slaveAddress, bytesToBits(reply->rawResult().data(),
                                                                  static_cast<int>(unit.valueCount())));
            break;
        case ReadHoldingRegisters:
            emit holdingRegistersReceived(slaveAddress, data);
            break;
        case ReadInputRegisters:
            emit inputRegistersReceived(slaveAddress, data);
            break;
        default:
            break;
        }
    } else {
        emit errorOccurred(reply->errorString());
    }
}

QVector<bool> ModbusClient::bytesToBits(const QByteArray &bytes, int bitCount)
{
    QVector<bool> bits;
    bits.reserve(bitCount);

    for (int i = 0; i < bytes.size() && bits.size() < bitCount; ++i) {
        quint8 byte = static_cast<quint8>(bytes.at(i));
        for (int bit = 0; bit < 8 && bits.size() < bitCount; ++bit) {
            bits.append((byte & (1 << bit)) != 0);
        }
    }

    return bits;
}

void ModbusClient::onPollingTimeout()
{
    if (m_pollingRequests.isEmpty() || !isConnected()) {
        return;
    }

    if (m_currentPollIndex >= m_pollingRequests.size()) {
        m_currentPollIndex = 0;
    }

    const ModbusRequest &request = m_pollingRequests[m_currentPollIndex];

    switch (request.function) {
    case ReadCoils:
        readCoils(request.slaveAddress, request.startAddress, request.quantity);
        break;
    case ReadDiscreteInputs:
        readDiscreteInputs(request.slaveAddress, request.startAddress, request.quantity);
        break;
    case ReadHoldingRegisters:
        readHoldingRegisters(request.slaveAddress, request.startAddress, request.quantity);
        break;
    case ReadInputRegisters:
        readInputRegisters(request.slaveAddress, request.startAddress, request.quantity);
        break;
    default:
        break;
    }

    m_currentPollIndex = (m_currentPollIndex + 1) % m_pollingRequests.size();
}

void ModbusClient::onStateChanged(QModbusDevice::State state)
{
    qDebug() << "Modbus device state changed:" << state;

    if (state == QModbusDevice::ConnectedState) {
        emit connectionStatusChanged(true);
    } else if (state == QModbusDevice::UnconnectedState) {
        emit connectionStatusChanged(false);
        emit errorOccurred(tr("设备已断开连接"));
    }
}

void ModbusClient::onErrorOccurred(QModbusDevice::Error error)
{
    if (error != QModbusDevice::NoError) {
        QString errorStr = m_modbusDevice->errorString();
        qDebug() << "Modbus error:" << errorStr;
        emit errorOccurred(errorStr);
    }
}
