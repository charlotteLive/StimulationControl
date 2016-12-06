#ifndef STIMULATOR_H
#define STIMULATOR_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QVector>
#include<QtSerialPort/QSerialPort>

class Stimulator : public QObject
{
    Q_OBJECT
public:
    explicit Stimulator(QObject *parent = 0);
    bool StimulaorConnect(QString comport);
    void StimulaorDisconnect();

    void InitAck();
    void WatchDog();
    void GetStimulationMode();
    void SinglePulse(int width, uchar current, uchar channel = 0);
    void InitChannelList(int interval = 40, char channel = 1);
    void StartChannel(int width, uchar current);
    void StartChannel(QVector<int> widths, QVector<uchar> currents);
    void StopChannel();

private:
    void writeData();
    uchar Checksum();
    void ByteStuffing();

signals:
    void recData(QString s);

public slots:
    void readData();
    void setWatchDog();

public:
    QTimer* my_watchdog;

private:
    QSerialPort* my_serialport;
    uchar packetNum;
    QByteArray array;
    QByteArray command_head;
    QByteArray command_data;
};

#endif // STIMULATOR_H
