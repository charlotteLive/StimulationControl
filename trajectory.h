#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <QDialog>
#include <QTimer>
#include "qcustomplot.h"

#import "OnLineInterface.dll" no_namespace

namespace Ui {
class trajectory;
}

class trajectory : public QDialog
{
    Q_OBJECT

public:
    explicit trajectory(IOnLine *_pOnline, QWidget *parent = 0);
    ~trajectory();

private slots:
    void on_sine_clicked();

    void on_step_clicked();

    void plot_sine();

private:
    Ui::trajectory *ui;
    IOnLine *pOnline;	// 传感器接口的指针
    unsigned int key;
    QTimer *sine_plot;
};

#endif // TRAJECTORY_H
