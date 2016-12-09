#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "stimulator.h"
#include <QString>
#include <QTimer>
#include <QVector>
#include "slidemodel.h"

#import "OnLineInterface.dll" no_namespace

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Stimulator* my_stimulator;
    typedef void (MainWindow::*PFUNC)(double);

private:
    void data_plot();
    void init_channels();
    void init_desire_trajectory();
    void step_trajectory(double time);
    void sine_trajectory(double time);
    void walk_trajectory(double time);
    void exten_trajectory(double time);

private slots:
    void on_connect_clicked();
    void showData(QString);
    void rect_stop();
    void data_update();

    void on_singlePulse_clicked();

    void on_rectPulse_clicked();

    void on_ControlStart_clicked();

    void on_action_triggered();

    void on_action_2_triggered();

    void on_action_3_triggered();

    void on_doubleStimulation_triggered();

    void on_action_trajectory_triggered();

    void on_sineWave_clicked();

    void on_desiredTrajectory_currentIndexChanged(int index);

    void on_sinePulseTimeout();

private:

    Ui::MainWindow *ui;
    bool flagConnect;
    bool isStart;
    QTimer* getData;
    QTimer* sinePulse;
    IOnLine *pOnline;	// 传感器接口的指针
    Slidemodel *slide;
    PFUNC desireTraj;

    unsigned int key;
    double value;
    double dValue;
    double desire;
    double desire2;
    double desire3;

    QVector<double> angleDesire;
    QVector<double> aDesire;
    QVector<double> dthDesire;
    QVector<double> ddthDesire;
    QVector<double> angleMeasure;
    QVector<double> dataWidth;

};

#endif // MAINWINDOW_H
