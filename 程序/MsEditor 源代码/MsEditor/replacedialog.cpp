#include "replacedialog.h"
#include "ui_replacedialog.h"

replaceDialog::replaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::replaceDialog)
{
    ui->setupUi(this);

    QFont font = ui->lineEdit->font();
    font.setPointSize(10);
    font.setFamily("Arial");
    ui->lineEdit->setFont(font);

    QFont font2 = ui->lineEdit_2->font();
    font2.setPointSize(10);
    font2.setFamily("Arial");
    ui->lineEdit_2->setFont(font2);

}

replaceDialog::~replaceDialog()
{
    delete ui;
}

void replaceDialog::on_pushButton_clicked()
{
    QString str_to_search = ui->lineEdit->text();
    QString str_replace = ui->lineEdit_2->text();
    int state = 0;
    int begin = 0;
    if(ui->checkBox_2->isChecked()){//区分大小写
        state += 2;
    }
    qDebug("send\n");
    emit sendReplaceDataToMain(str_to_search,str_replace,state,begin);
}
