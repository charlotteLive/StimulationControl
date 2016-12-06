#include "doublestimdialog.h"
#include "ui_doublestimdialog.h"
#include <QVector>

DoubleStimDialog::DoubleStimDialog(Stimulator *_stimulator, QWidget *parent) :
    QDialog(parent),
    my_stimulator(_stimulator),
    ui(new Ui::DoubleStimDialog)
{
    ui->setupUi(this);
}

DoubleStimDialog::~DoubleStimDialog()
{
    delete ui;
}

void DoubleStimDialog::on_pushButton_clicked()
{
    uchar channels = 1*ui->channel_1->isChecked() + 2*ui->channel_2->isChecked();
    my_stimulator->InitChannelList(40,channels);

    QVector<int> widths;
    widths<< ui->width_1->value() << ui->width_2->value();
    QVector<uchar> currents;
    currents<< ui->current_1->value() << ui->current_2->value();
    my_stimulator->StartChannel(widths,currents);
}

void DoubleStimDialog::on_pushButton_2_clicked()
{
    my_stimulator->StopChannel();

    my_stimulator->InitChannelList(40,4);
    my_stimulator->StartChannel(ui->width_3->value(),ui->current_3->value());
}
