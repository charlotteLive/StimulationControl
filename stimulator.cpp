#include "stimulator.h"

Stimulator::Stimulator(QObject *parent)
    : QObject(parent),
      packetNum(0)
{
    my_serialport = new QSerialPort(this);
    connect(my_serialport, SIGNAL(readyRead()), this, SLOT(readData()));

    my_watchdog = new QTimer(this);
    connect(my_watchdog,SIGNAL(timeout()),this,SLOT(setWatchDog()));
    command_data.reserve(20);
    command_head.append(char(0xF0)).append(char(0x81)).append(char(0x00))
            .append(char(0x81)).append(char(0x00));

}

uchar Stimulator::Checksum()
{
    uchar i;
    uchar crc=0;
    int len = command_data.size();
    QByteArray::iterator iter = command_data.begin();
    while(len--!=0)
    {
        for(i=uchar(0x80); i!=0; i/=2)
        {
            if((crc&0x80)!=0) {crc*=2; crc^=uchar(0x07);} /* 余式CRC 乘以2 再求CRC */
            else crc*=2;
            if((*iter&i)!=0) crc^=uchar(0x07); /* 再加上本位的CRC */
        }
        iter++;
    }
    return crc^uchar(0x55);
}

void Stimulator::ByteStuffing()
{
    QByteArray::iterator iter = command_data.begin();
    while(iter != command_data.end())
    {
        if (char(0xF0) == *iter || 0x0F == *iter)
        {
            *iter = (*iter)^0x55;
            command_data.insert((iter++)-command_data.begin(),char(0x81));
        }
        iter++;
    }
    command_head[2] = Checksum();
    command_head[4] = uchar(command_data.size())^uchar(0x55);
}

//读取串口数据
void Stimulator::readData()
{
     QByteArray data = my_serialport->readAll();
//     emit recData(QString(data.toHex()));  //处理并显示收到的命令，暂时不做处理
}

//写串口数据
void Stimulator::writeData()
{
    ByteStuffing();
    array.clear();
    array.append(command_head).append(command_data).append(char(0x0F));
    my_serialport->write(array);
    packetNum++;
}

void Stimulator::InitAck()
{
    command_data.clear();
    command_data.append(packetNum).append(char(2)).append(char(0));
    writeData();
}

void Stimulator::WatchDog()
{
    command_data.clear();
    command_data.append(packetNum).append(char(4));
    writeData();
}

void Stimulator::GetStimulationMode()
{
    command_data.clear();
    command_data.append(packetNum).append(char(10));
    writeData();
}

void Stimulator::SinglePulse(int width, uchar current, uchar channel)
{
    if(width<20)   width = 20;
    else if(width>500) width = 500;

    if(current>130) current = 130;

    command_data.clear();
    command_data.append(packetNum).append(char(36)).append(channel).append(char(width/256))
            .append(char(width%256)).append(current);
    writeData();
}

void Stimulator::InitChannelList(int interval,char channel)
{
    command_data.clear();
    command_data.append(packetNum).append(char(30)).append(char(0))
            .append(char(1<<channel)).append(char(0)).append(char(interval*2-3)).append(char(0))
            .append(char(interval*2-2)).append(char(0));
    writeData();
}

void Stimulator::StartChannel(int width, uchar current)
{
    command_data.clear();
    command_data.append(packetNum).append(char(32)).append(char(0)).append(char(width/256))
            .append(char(width%256)).append(current);
    writeData();
}

void Stimulator::StopChannel()
{
    command_data.clear();
    command_data.append(packetNum).append(char(34));
    writeData();
}

bool Stimulator::StimulaorConnect(QString comport)
{
    my_serialport->setPortName(comport);
    my_serialport->setBaudRate(460800);
    my_serialport->setDataBits(QSerialPort::Data8);
    my_serialport->setParity(QSerialPort::EvenParity);
    my_serialport->setStopBits(QSerialPort::OneStop);
    my_serialport->setFlowControl(QSerialPort::NoFlowControl);
    if (my_serialport->open(QIODevice::ReadWrite))
    {
        //电刺激仪初始化命令
        InitAck();
        //选择电刺激模式
        GetStimulationMode();
        //打开看门狗
        my_watchdog->start(1200);

        return true;
    }
    else
    {
        return false;
    }
}

void Stimulator::StimulaorDisconnect()
{
    my_watchdog->stop();
    my_serialport->close();
    packetNum = 0;
}

void Stimulator::setWatchDog()
{
    WatchDog();
}
