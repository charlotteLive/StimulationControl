#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QMessageBox>
#include <QtMath>
#include <QFile>
#include <QTextStream>
#include "qcustomplot.h"
#include "doublestimdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    flagConnect(false),
    isStart(false),
    desireTraj(&MainWindow::step_trajectory),
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

    init_channels();
    init_desire_trajectory();
    sinePulse = new QTimer(this);
    connect(sinePulse,SIGNAL(timeout()),this,SLOT(on_sinePulseTimeout()));

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

void MainWindow::init_channels()
{
    ui->buttonGroup->setId(ui->radioButton_1, 0);
    ui->buttonGroup->setId(ui->radioButton_2, 1);
    ui->buttonGroup->setId(ui->radioButton_3, 2);
    ui->buttonGroup->setId(ui->radioButton_4, 3);
    ui->buttonGroup->setId(ui->radioButton_5, 4);
    ui->buttonGroup->setId(ui->radioButton_6, 5);
    ui->buttonGroup->setId(ui->radioButton_7, 6);
    ui->buttonGroup->setId(ui->radioButton_8, 7);
}

void MainWindow::init_desire_trajectory()
{
    ui->desiredTrajectory->addItem(QStringLiteral("阶跃轨迹"));
    ui->desiredTrajectory->addItem(QStringLiteral("正弦轨迹"));
    ui->desiredTrajectory->addItem(QStringLiteral("伸膝轨迹"));
    ui->desiredTrajectory->addItem(QStringLiteral("步行轨迹"));

}

void MainWindow::on_action_triggered()
{
    ui->currentPage->setCurrentIndex(0);
}

void MainWindow::on_action_2_triggered()
{
    ui->currentPage->setCurrentIndex(1);
}

void MainWindow::on_action_3_triggered()
{
    ui->currentPage->setCurrentIndex(2);
}

void MainWindow::on_doubleStimulation_triggered()
{
    DoubleStimDialog dStim(my_stimulator);
    dStim.exec();
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
    my_stimulator->SinglePulse(width,current,ui->buttonGroup->checkedId());
}

void MainWindow::on_rectPulse_clicked()
{
    //通道初始化，周期默认为40ms
    char channel = 1>>ui->buttonGroup->checkedId();
    my_stimulator->InitChannelList(40,channel);

    //延时200ms
    QTime t;
    t.start();
    while(t.elapsed()<200)
        QCoreApplication::processEvents();

    //开始指令
    int width = ui->sWidth->value();
    uchar current = ui->sCurrent->value();
    my_stimulator->StartChannel(width,current);

    if(ui->isCollection->isChecked())
    {
        pOnline->OnLineStatus(0, ONLINE_START, NULL);
    }

    //定时关闭
    QTimer::singleShot(1000*ui->stimulationTime->value(),this,SLOT(rect_stop()));
}

void MainWindow::rect_stop()
{
    my_stimulator->StopChannel();
    if(ui->isCollection->isChecked())
    {
         pOnline->OnLineStatus(0, ONLINE_STOP, NULL);
    }
}

void MainWindow::on_sineWave_clicked()
{
    key = 0;
    sinePulse->start(10);
    if(ui->isCollection->isChecked())
    {
         pOnline->OnLineStatus(0, ONLINE_START, NULL);
    }
}

void MainWindow::on_sinePulseTimeout()
{
    if(key%4==0)
    {
        int time = key * 0.01;
        int width = ui->sine_1->value() + (ui->sine_2->value())*qSin(6.28/(ui->sine_3->value())*time-1.57);
        my_stimulator->SinglePulse(width,ui->sCurrent->value(),ui->buttonGroup->checkedId());
        if(key >= static_cast<unsigned int>(ui->stimulationTime->value())*100)
        {
            sinePulse->stop();
            if(ui->isCollection->isChecked())
            {
                 pOnline->OnLineStatus(0, ONLINE_STOP, NULL);
            }
        }
    }
    ++key;
}

//闭环控制按钮的响应函数
void MainWindow::on_ControlStart_clicked()
{
    //isStart为真则断开连接并记录数据，为假则开始采集控制
    if (isStart)
    {
        pOnline->OnLineStatus(0, ONLINE_STOP, NULL);
        ui->ControlStart->setText(QStringLiteral("开始"));
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
        ui->ControlStart->setText(QStringLiteral("停止"));
        isStart = true;
        key = 0;
        getData->start(10);
        my_stimulator->my_watchdog->stop();
        ui->connect->setEnabled(false);

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
            on_ControlStart_clicked();
        }
        else{
            static double valueLast = 0;
            double time = key/100.0;
            value = -(gData-4000)/4000.0*180;
            dValue = (value - valueLast)/0.04;
            valueLast = value;

            //通过函数指针完成轨迹的选择
            (this->*desireTraj)(time);

            ui->customPlot->graph(0)->addData(time, value);
            ui->customPlot->graph(0)->removeDataBefore(time-15);
            ui->customPlot->graph(0)->rescaleValueAxis();
            ui->customPlot->graph(1)->addData(time, desire);
            ui->customPlot->graph(1)->removeDataBefore(time-15);
            ui->customPlot->graph(1)->rescaleValueAxis(true);
            ui->customPlot->xAxis->setRange(time+0.25, 15, Qt::AlignRight);
            ui->customPlot->replot();

            angleDesire.append(desire);
            angleMeasure.append(value);

            desire = qDegreesToRadians(desire);
            desire2 = qDegreesToRadians(desire2);
            desire3 = qDegreesToRadians(desire3);
            value = qDegreesToRadians(value);
            dValue = qDegreesToRadians(dValue);
            int pWidth = slide->computeOut(desire,desire2,desire3,value,dValue);
            my_stimulator->SinglePulse(pWidth,ui->sCurrent_2->value());
            dataWidth.append(double(pWidth));

            QString s;
            s.sprintf("width is = %d",pWidth);
            ui->lineEdit->setText(s);
        }
    }
    key++;
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

    ui->currentPage->setCurrentIndex(2);
}

void MainWindow::on_desiredTrajectory_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        desireTraj = &MainWindow::step_trajectory;
        break;
    case 1:
        desireTraj = &MainWindow::sine_trajectory;
        break;
    case 2:
        desireTraj = &MainWindow::exten_trajectory;
        break;
    case 3:
        desireTraj = &MainWindow::walk_trajectory;
        break;
    default:
        break;
    }
}

void MainWindow::step_trajectory(double time)
{
    time = time;
    desire = 40;desire2 = 0;desire3 = 0;
}

void MainWindow::sine_trajectory(double time)
{
    desire = 25 + 15*qSin(time-1.57);
    desire2 = 15*qCos(time-1.57);
    desire3 = -15*qSin(time-1.57);
}

void MainWindow::walk_trajectory(double time)
{
    int key = time*100;
    desire = aDesire[key/4];
    desire2 = dthDesire[key/4];
    desire3 = ddthDesire[key/4];
}

void MainWindow::exten_trajectory(double time)
{
    desire = 40 - 35*qExp(-0.4*time);
    desire2 = -14*qExp(-0.4*time);
    desire3 = 5.6*qExp(-0.4*time);
}
