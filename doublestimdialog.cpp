#include "doublestimdialog.h"
#include "ui_doublestimdialog.h"

DoubleStimDialog::DoubleStimDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DoubleStimDialog)
{
    ui->setupUi(this);
}

DoubleStimDialog::~DoubleStimDialog()
{
    delete ui;
}
