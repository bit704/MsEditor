#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

namespace Ui {
class replaceDialog;
}

class replaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit replaceDialog(QWidget *parent = 0);
    ~replaceDialog();
signals:
    void sendReplaceDataToMain(QString seastr,QString repstr,int state,int begin);
private slots:
    void on_pushButton_clicked();

private:
    Ui::replaceDialog *ui;
};

#endif // REPLACEDIALOG_H
