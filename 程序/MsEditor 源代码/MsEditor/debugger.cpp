#include "debugger.h"
#include "ui_debugger.h"

Debugger::Debugger(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Debugger)
{
    ui->setupUi(this);

    editor=new QsciScintilla(this);
    editor->setReadOnly(true);

    //行号提示
    editor->setMarginType(0,QsciScintilla::NumberMargin);//设置0号页边显示行号
    editor->setMarginLineNumbers(0,true);//对0号页边启用行号
    editor->setMarginWidth(0,30);//设置页边宽度

    editor->setMarginType(1,QsciScintilla::SymbolMargin); //设置1号页边显示符号
    editor->SendScintilla(QsciScintilla::SCI_MARKERSETFORE, 1,QColor(Qt::black)); //置标记前景和背景标记
    editor->SendScintilla(QsciScintilla::SCI_MARKERSETBACK, 1,QColor(Qt::red));

    editor->SendScintilla(QsciScintilla::SCI_SETMARGINMASKN,1,0x02); //1号页边显示2号标记

    editor->SendScintilla(editor->SCI_SETMARGINSENSITIVEN,1,true); //设置1号页边可以点击


    connect(editor,SIGNAL(marginClicked(int,int,Qt::KeyboardModifiers)),this,SLOT(addMarker(int,int,Qt::KeyboardModifiers)));

    QVBoxLayout *pLayout = ui->verticalLayout;
    pLayout->addWidget(editor);
    pLayout->setContentsMargins(0,0,0,0);

    cmd = new QProcess(this);
    connect(cmd,SIGNAL(readyReadStandardOutput()),this,SLOT(cmdOutput()));
    connect(cmd,SIGNAL(readyReadStandardOutput()),this,SLOT(cmdError()));
    cmd->start("cmd.exe");
    cmd->waitForStarted();
}

Debugger::~Debugger()
{
    cmd->close();
    cmd->waitForFinished();//等待程序关闭
    delete ui;
}

void Debugger::addMarker(int m, int n, Qt::KeyboardModifiers)
{
    //m为被点击的页边的编号（列），n为行
    int maskn=editor->SendScintilla(QsciScintilla::SCI_MARKERGET,n);
    //qDebug()<<n<<endl;
    if(maskn == 0)
    {
        editor->SendScintilla(QsciScintilla::SCI_MARKERADD,n,m);
        QString str = "break "+QString::number(n+1)+"\r\n";
        //qDebug()<<str;
        cmd->write(str.toStdString().c_str());
    }
    else
    {
        //editor->SendScintilla(QsciScintilla::SCI_MARKERDELETE,n,m);
    }
}



void Debugger::on_pushButton_clearMarker_clicked() //清空所有断点标记
{
    editor->SendScintilla(QsciScintilla::SCI_MARKERDELETEALL,1,0x02); //清空1号页边所有2号标记
}

void Debugger::setText(QString str)
{
    editor->setText(str);
}

void Debugger::runGDB(QString exename)
{

    QString str("gdb "+exename+"\r\n");
    qDebug()<<str;
    cmd->write(str.toStdString().c_str());
}

void Debugger::cmdOutput()
{
   QByteArray  strBytes = cmd->readAllStandardOutput();
   QTextCodec *qtc=QTextCodec::codecForName("GBK");
   QString str = qtc->toUnicode(strBytes);
   ui->textBrowser->setText(str);
}

void Debugger::cmdError()
{
    //ui->textBrowser->append(cmd->readAllStandardError().data());
}

void Debugger::on_pushButton_sendGDB_clicked()
{
    QString str = ui->lineEdit_sendToGDB->text()+"\r\n";
    cmd->write(str.toStdString().c_str());
}

void Debugger::on_pushButton_run_clicked()
{
    QString str = "run\r\n";
    cmd->write(str.toStdString().c_str());
}

void Debugger::on_pushButton_continue_clicked()
{
    QString str = "continue\r\n";
    cmd->write(str.toStdString().c_str());
}

void Debugger::on_pushButton_next_clicked()
{
    QString str = "next\r\n";
    cmd->write(str.toStdString().c_str());
}

void Debugger::on_pushButton_step_clicked()
{
    QString str = "step\r\n";
    cmd->write(str.toStdString().c_str());
}

void Debugger::on_pushButton_showVariable_clicked()
{
    QString str = "print "+ui->lineEdit_variable->text()+"\r\n";
    cmd->write(str.toStdString().c_str());
    //qDebug()<<str.toStdString().c_str();
}
