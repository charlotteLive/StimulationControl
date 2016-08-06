#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QMessageBox>
#include <QtMath>
#include <QFile>
#include <QTextStream>
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    flagConnect(false),
    isStart(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->currentPage->setCurrentIndex(0);

    //串口显示连接
    ui->comBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->comBox->addItem(info.portName());
    }
    my_stimulator = new Stimulator(this);
    connect(my_stimulator,SIGNAL(recData(QString)),this,SLOT(showData(QString)));

    //连接传感器采集盒
    OleInitialize(NULL);	// Initializes the COM library (similar to AfxOleInit() in Windows)
    CoCreateInstance(__uuidof(OnLine), NULL, CLSCTX_INPROC_SERVER, __uuidof(IOnLine), (void**)&pOnline);    // To use the interface class, create an instance:
    if (pOnline == NULL)
    {
        QMessageBox::information(this,"Error","Cannot find OnLineInterface.dll.\n\n");
    }
    else
    {
        ui->statusBar->showMessage("DataLink Console Test Application!\n");
    }

    //绘图参数
    ui->customPlot->addGraph(); // blue line
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
    ui->customPlot->axisRect()->setupFullAxesBox();
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    ui->customPlot->yAxis->setRange(-180,180);

    //获取传感器数据的定时器
    getData = new QTimer(this);
    connect(getData, SIGNAL(timeout()), this, SLOT(data_update()));

    //预先分配存储内存以提高效率，最多100s的数据
    angleDesire.reserve(3000);
    angleMeasure.reserve(3000);
    dataWidth.reserve(3000);

    //读取期望轨迹
    QFile file("../StimulationControl/angle_walk2.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return ;
    QTextStream in(&file);
    while(!in.atEnd()){
        QString line = in.readLine();
        QStringList list = line.split("\t");
        aDesire.append(list.at(0).toDouble());
        dthDesire.append(list.at(1).toDouble());
        ddthDesire.append(list.at(2).toDouble());
    }
    file.close();
}

MainWindow::~MainWindow()
{
    if (pOnline != NULL)
        pOnline->Release();	// release the interface class instance
    delete ui;
}

void MainWindow::on_connect_clicked()
{
    QString comport = ui->comBox->currentText();
    //flag为真时断开，为假时连接
    if(!flagConnect)
    {
        if(my_stimulator->StimulaorConnect(comport))
        {
            flagConnect = true;
            ui->connect->setText(QStringLiteral("断开"));
            ui->statusBar->showMessage(QStringLiteral("连接成功"),2000);
        }
        else ui->statusBar->showMessage(QStringLiteral("连接失败"),2000);
    }
    else
    {
        my_stimulator->StimulaorDisconnect();
        flagConnect = false;
        ui->connect->setText(QStringLiteral("连接"));
        ui->statusBar->showMessage(QStringLiteral("断开连接"),2000);
    }
}

void MainWindow::showData(QString s)
{
    ui->statusBar->showMessage(s);
}

void MainWindow::on_singlePulse_clicked()
{
    int width = ui->sWidth->value();
    uchar current = ui->sCurrent->value();
    my_stimulator->SinglePulse(width,current);
}

void MainWindow::on_rectPulse_clicked()
{
    //通道初始化，周期默认为40ms
    my_stimulator->InitChannelList();

    //延时200ms
    QTime t;
    t.start();
    while(t.elapsed()<200)
        QCoreApplication::processEvents();

    //开始指令
    int width = ui->sWidth->value();
    uchar current = ui->sCurrent->value();
    my_stimulator->StartChannel(width,current);

    //定时关闭
    QTimer::singleShot(3000,this,SLOT(rect_stop()));
}

void MainWindow::rect_stop()
{
    my_stimulator->StopChannel();
    pOnline->OnLineStatus(0, ONLINE_STOP, NULL);
}

//闭环控制按钮的响应函数
void MainWindow::on_ControlStart_clicked()
{
    //isStart为真则断开连接并记录数据，为假则开始采集控制
    if (isStart)
    {
        pOnline->OnLineStatus(0, ONLINE_STOP, NULL);
        ui->ControlStart->setText("start");
        isStart = false;
        getData->stop();
        my_stimulator->my_watchdog->start(1200);
        ui->connect->setEnabled(true);

        data_plot();

        QFile file("angleData.txt");
        if (!file.open(QIODevice::Append | QIODevice::Text)) return;
        QTextStream out(&file);
        int dataNum = angleMeasure.size();
        out<<"the number of data is :"<<dataNum<<"\n";
        for(int i = 0;i<dataNum;i++){
            out<<angleDesire[i]<<'\t'<<angleMeasure[i]<<'\t'<<dataWidth[i]<<'\n';
        }
        angleDesire.clear();
        angleMeasure.clear();
        dataWidth.clear();
    }
    else
    {
        pOnline->OnLineStatus(0, ONLINE_START, NULL);
        ui->ControlStart->setText("stop");
        isStart = true;
        getData->start(10);
        my_stimulator->my_watchdog->stop();
        ui->connect->setEnabled(false);
        key = 0;

        //创建滑模控制器对象
        slide = new Slidemodel(3,10);
    }
}

void MainWindow::data_update()
{
    //40ms触发一次
    if(key%4==0)
    {
        long gData;
        pOnline->OnLineStatus(0, ONLINE_GETVALUE, &gData);
        if (gData < 0){
            QMessageBox::information(this,"Error","Cannot communicate with DataLINK!\n  Is DataLINK running and connected to the DataLINK hardware?");
            getData->stop();
        }
        else{
            static double valueLast = 0;
            double time = key/100.0;
            value = -(gData-4000)/4000.0*180;
            dValue = (value - valueLast)/0.04;
            valueLast = value;

            //正弦参考角度
//            desire = 25 + 15*qSin(time-1.57);desire2 = 15*qCos(time-1.57);desire3 = -15*qSin(time-1.57);
            //阶跃参考角度
//            desire = 40;desire2 = 0;desire3 = 0;
            //伸膝轨迹
//            desire = 40 - 35*qExp(-0.4*time);desire2 = -14*qExp(-0.4*time);desire3 = 5.6*qExp(-0.4*time);
            //自定义轨迹，行走轨迹
            desire = aDesire[key/4];desire2 = dthDesire[key/4];desire3 = ddthDesire[key/4];

            ui->customPlot->graph(0)->addData(time, value);
            ui->customPlot->graph(0)->removeDataBefore(time-10);
            ui->customPlot->graph(0)->rescaleValueAxis();
            ui->customPlot->graph(1)->addData(time, desire);
            ui->customPlot->graph(1)->removeDataBefore(time-10);
            ui->customPlot->graph(1)->rescaleValueAxis(true);
            ui->customPlot->xAxis->setRange(time+0.25, 10, Qt::AlignRight);
            ui->customPlot->replot();

            angleDesire.append(desire);
            angleMeasure.append(value);

            desire = qDegreesToRadians(desire);
            desire2 = qDegreesToRadians(desire2);
            desire3 = qDegreesToRadians(desire3);
            value = qDegreesToRadians(value);
            dValue = qDegreesToRadians(dValue);
            int pWidth = slide->computeOut(desire,desire2,desire3,value,dValue);
            my_stimulator->SinglePulse(pWidth,ui->sCurrent->value());
            dataWidth.append(double(pWidth));

            QString s;
            s.sprintf("width is = %d",pWidth);
            ui->lineEdit->setText(s);
        }
    }
    key++;
}


void MainWindow::on_action_triggered()
{
    ui->currentPage->setCurrentIndex(0);
}

void MainWindow::on_action_2_triggered()
{
    ui->currentPage->setCurrentIndex(1);
}

void MainWindow::data_plot()
{
    QVector<double> stiTime(angleDesire.size());
    for(int i=0;i<angleDesire.size();i++)   stiTime[i] = 0.04*i;
    ui->anglePlot->addGraph();
    ui->anglePlot->graph(0)->setPen(QPen(Qt::red));
    ui->anglePlot->graph(0)->setData(stiTime, angleDesire);
    ui->anglePlot->addGraph();
    ui->anglePlot->graph(1)->setData(stiTime,angleMeasure);

    ui->widthPlot->addGraph();
    ui->widthPlot->graph(0)->setData(stiTime,dataWidth);

    ui->anglePlot->yAxis->setRange(0,60);
    ui->anglePlot->graph(0)->rescaleAxes(true);
    ui->widthPlot->graph(0)->rescaleAxes();
    ui->anglePlot->replot();
    ui->widthPlot->replot();

    ui->currentPage->setCurrentIndex(1);
}

void MainWindow::on_rectPulseData_clicked()
{
    on_rectPulse_clicked();
    pOnline->OnLineStatus(0, ONLINE_START, NULL);
}
