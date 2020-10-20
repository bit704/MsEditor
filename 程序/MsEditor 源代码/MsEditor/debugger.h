#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QDialog>
#include <QHBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QTextCodec>

#include <Qsci/qsciscintilla.h>

namespace Ui {
class Debugger;
}

class Debugger : public QDialog
{
    Q_OBJECT

public:
    explicit Debugger(QWidget *parent = 0);
    ~Debugger();

private:
    Ui::Debugger *ui;
    QsciScintilla *editor;
    QProcess *cmd;

private slots:
    void addMarker(int,int,Qt::KeyboardModifiers);

    void on_pushButton_clearMarker_clicked();

    void setText(QString);

    void runGDB(QString);

    void cmdOutput();

    void cmdError();
    void on_pushButton_sendGDB_clicked();
    void on_pushButton_run_clicked();
    void on_pushButton_continue_clicked();
    void on_pushButton_next_clicked();
    void on_pushButton_step_clicked();
    void on_pushButton_showVariable_clicked();
};

#endif // DEBUGGER_H
