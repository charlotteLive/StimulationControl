#ifndef DOUBLESTIMDIALOG_H
#define DOUBLESTIMDIALOG_H

#include <QDialog>

namespace Ui {
class DoubleStimDialog;
}

class DoubleStimDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DoubleStimDialog(QWidget *parent = 0);
    ~DoubleStimDialog();

private:
    Ui::DoubleStimDialog *ui;
};

#endif // DOUBLESTIMDIALOG_H
