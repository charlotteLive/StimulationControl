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

    void on_rectPulseData_clicked();

private:
    void data_plot();

    Ui::MainWindow *ui;
    bool flagConnect;
    bool isStart;
    Stimulator* my_stimulator;
    QTimer* getData;
    IOnLine *pOnline;	// 传感器接口的指针
    Slidemodel *slide;

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
