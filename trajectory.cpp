#include "trajectory.h"
#include "ui_trajectory.h"

trajectory::trajectory(IOnLine *_pOnline, QWidget *parent) :
    QDialog(parent),
    pOnline(_pOnline),
    ui(new Ui::trajectory)
{
    ui->setupUi(this);
    //绘图参数
    ui->customPlot->addGraph(); // blue line
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
    ui->customPlot->axisRect()->setupFullAxesBox();
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    ui->customPlot->yAxis->setRange(-180,180);

}

trajectory::~trajectory()
{
    delete ui;
}

void trajectory::on_sine_clicked()
{
    //40ms触发一次
    if(key%4==0)
    {
        long gData;
        pOnline->OnLineStatus(0, ONLINE_GETVALUE, &gData);
        if (gData >= 0)
        {
            double time = key/100.0;
            value = -(gData-4000)/4000.0*180;
            dValue = (value - valueLast)/0.04;
            valueLast = value;

            ui->customPlot->graph(0)->addData(time, value);
            ui->customPlot->graph(0)->removeDataBefore(time-15);
            ui->customPlot->graph(0)->rescaleValueAxis();
            ui->customPlot->graph(1)->addData(time, desire);
            ui->customPlot->graph(1)->removeDataBefore(time-15);
            ui->customPlot->graph(1)->rescaleValueAxis(true);
            ui->customPlot->xAxis->setRange(time+0.25, 15, Qt::AlignRight);
            ui->customPlot->replot();
        }
    }
    key++;
}

void trajectory::on_step_clicked()
{

}
