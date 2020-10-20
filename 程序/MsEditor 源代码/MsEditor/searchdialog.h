#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

namespace Ui {
class searchDialog;
}

class searchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit searchDialog(QWidget *parent = 0);
    ~searchDialog();
signals:
    void  sendSearchDataToMain(QString,int,int);
private slots:
    void  on_search_button_clicked();

private:
    Ui::searchDialog *ui;
};

#endif // SEARCHDIALOG_H
