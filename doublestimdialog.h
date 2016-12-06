#ifndef DOUBLESTIMDIALOG_H
#define DOUBLESTIMDIALOG_H

#include <QDialog>
#include <stimulator.h>

namespace Ui {
class DoubleStimDialog;
}

class DoubleStimDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DoubleStimDialog(Stimulator *_stimulator, QWidget *parent = 0);
    ~DoubleStimDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::DoubleStimDialog *ui;
    Stimulator *my_stimulator;
};

#endif // DOUBLESTIMDIALOG_H
