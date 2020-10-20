#include "tab.h"
#include "ui_tab.h"
#include <QFile>
#include <QStringListModel>
#include <QMessageBox>

Tab::Tab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab)
{
    ui->setupUi(this);
}

Tab::Tab(int index,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab)
{
    ui->setupUi(this);
    ownIndex = index; //记录自己的标号
    highlighter = new Highlighter(ui->plainTextEdit->document());  //高亮

    QFont font = ui->plainTextEdit->font();
    font.setPointSize(10);
    font.setFamily("Arial");
    ui->plainTextEdit->setFont(font);
    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/wordlist.txt"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    ui->plainTextEdit->setCompleter(completer);

    connect(ui->plainTextEdit,SIGNAL(textChanged()),this,SLOT(theTextChanged()));
}

Tab::~Tab()
{
    delete ui;
}

void Tab::setText(QString str,int index) //设置文本
{
    if(index != ownIndex) return;
    ui->plainTextEdit->setPlainText(str);
}

void Tab::prepareFetchTextForSaveAs(int index)
{
    if(index != ownIndex) return;
    emit fetchTextForSaveAs(ui->plainTextEdit->toPlainText(),index);
}

void Tab::prepareFetchTextForSave(int index)
{
    if(index != ownIndex) return;
    emit fetchTextForSave(ui->plainTextEdit->toPlainText(),index);
}

void Tab::operating(int op)
{
    switch (op) {
    case 0:
        ui->plainTextEdit->copy();
        break;
    case 1:
        ui->plainTextEdit->paste();
        break;
    case 2:
        ui->plainTextEdit->cut();
        break;
    case 3:
        ui->plainTextEdit->undo();
        break;
    default:
        break;
    }
}

void Tab::receiveSearchDataForTab(QString data,int index,int state,int begin)//开始搜索指定字符串
{
    if(index != ownIndex)
        return;
    QString real_search_str = data;
    QByteArray ba=real_search_str.toLatin1();
    char *c=ba.data();
    qDebug("%s\n",c);
    qDebug("%d\n",state);
    if(real_search_str != NULL){
        bool found = false;
        QTextDocument *document = ui->plainTextEdit->document();
        QTextCursor highlight_cursor(document);
        QTextCursor cursor(document);
        if(begin == 1){
            highlight_cursor.setPosition(ui->plainTextEdit->textCursor().position());
        }
        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setBackground(Qt::yellow);
        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()){
            switch (state) {
            case 0:
                highlight_cursor = document->find(real_search_str,highlight_cursor);//向前搜索、不区分大小写、不全字匹配
                break;
            case 1:
                highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindWholeWords);//向前搜索、不区分大小写、全字匹配
                break;
            case 2:
                //向前搜索、区分大小写、不全字匹配
                highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindCaseSensitively);
                break;
            case 3:
                //向前搜索、不区分大小写、全字匹配
                highlight_cursor = document->find(real_search_str,highlight_cursor,
                                                  QTextDocument::FindWholeWords|QTextDocument::FindCaseSensitively);
                break;
            case -4:
                //向后搜索、不区分大小写、不全字匹配
                highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward);
                break;
            case -3:
                //向后搜索、不区分大小写、全字匹配
                highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindWholeWords);
                break;
            case -2:
                //向后搜索、区分大小写、不全字匹配
                highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindCaseSensitively);
                break;
            case -1:
                //向后搜索、区分大小写、全字匹配
                highlight_cursor = document->find(real_search_str,highlight_cursor,
                                                  QTextDocument::FindBackward|QTextDocument::FindWholeWords|QTextDocument::FindCaseSensitively);
                break;
            }
            if (!highlight_cursor.isNull())
            {
                if(!found)
                {
                    found = true;
                }
                highlight_cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();

        if(found == false){
            qDebug("not found");
        }
    }
}

void Tab::receiveReplaceDataForTab(QString sear, QString rep, int index, int state,int begin)//开始替换指定字符串
{
    if(index != ownIndex) return;
    QStringList qslist;
    QTextDocument* doc=ui->plainTextEdit->document(); //文本对象
    int row_num=doc->blockCount () ;//回车符是一个 block
    if(state >= 0){
        for (int i=0; i<row_num;i++)
        {
            QTextBlock textLine=doc->findBlockByNumber (i) ; // 文本中的一段
            QString str=textLine.text();//将该段转换为QString
            if(state == 2)//区分大小写且不全字匹配
                str.replace(sear,rep);
            else if(state == 0){//不区分大小写且不全字匹配
                str.replace(sear,rep,Qt::CaseInsensitive);
            }
            qslist.append(str);
        }
        QStringList::Iterator it;
        for(it = qslist.begin();it != qslist.end();it++){
            if(it == qslist.begin()){
                ui->plainTextEdit->setPlainText(*it);
            }
            else{
                ui->plainTextEdit->appendPlainText(*it);
            }
        }
    }
    qDebug("suc!");
    QMessageBox::information(NULL,"information","ok");
}

void Tab::on_plainTextEdit_cursorPositionChanged()
{
    QTextCursor cursor;
    QString str_row = "行：";
    QString str_col = "列：";
    QString number;
    cursor = ui->plainTextEdit->textCursor();
    int rowNumber = cursor.blockNumber() + 1;//获取光标所在行
    int colNumber = cursor.columnNumber() + 1;//获取光标所在列
    number.setNum(rowNumber,10);
    str_row.append(number);
    number.setNum(colNumber,10);
    str_col.append(number);

    emit tabCursorPositionChanged(str_row,str_col);
}

void Tab::on_plainTextEdit_blockCountChanged(int newBlockCount)
{
    QString str_num = "总行数：";
    QString number;
    str_num.append(number.setNum(newBlockCount));

    emit tabBlockCountChanged(str_num);
}

QAbstractItemModel *Tab::modelFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << QString::fromUtf8(line.trimmed());
    }

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, completer);
}

void Tab::prepareFetchTextForFunctionTree(int index)
{
    if(index!=ownIndex) return;
    emit fetchTextForFunctionTree(ui->plainTextEdit->toPlainText());
}

void Tab::on_pushButton_clicked() //根据用户得输入跳转到指定行
{
    int line = ui->lineEdit->text().toInt(); //要跳到第几行
    jumpToLine(line-1); //实际上行是从0开始的，但用户看到的是从1开始
}

void Tab::theTextChanged() //文本发生改变
{
    //qDebug()<<"changed"<<endl;
    emit sendIndexOfChangedTab(ownIndex);
}

void Tab::jumpToFunction(int index, int line) //跳转到指定函数
{
    if(index != ownIndex) return;
    jumpToLine(line);
}

void Tab::jumpToLine(int line) //跳转到指定行
{
    int position = ui->plainTextEdit->document()->findBlockByNumber(line).position(); //找到这行开头的字符处于整个文件中的位置
    //qDebug()<<lineNumber<<" "<<position;
    QTextCursor tc = QTextCursor(ui->plainTextEdit->document());
    tc.setPosition(position,QTextCursor::MoveAnchor);
    ui->plainTextEdit->setTextCursor(tc);
}

void Tab::hideBlock(int index,int start,int end)
{
    if(index!=ownIndex) return;
    //qDebug()<<start<<" "<<end<<endl;
    for(int i=start;i<=end;++i)
    {
        QTextBlock qtb = ui->plainTextEdit->document()->findBlockByNumber(i);
        qtb.setVisible(false);
    }
    ui->plainTextEdit->viewport()->update(); //重绘
    ui->plainTextEdit->document()->adjustSize(); //重新适应大小
}

void Tab::showBlock(int index,int start,int end)
{
    if(index!=ownIndex) return;
    //qDebug()<<start<<" "<<end<<endl;
    for(int i=start;i<=end;++i)
    {
        QTextBlock qtb = ui->plainTextEdit->document()->findBlockByNumber(i);
        qtb.setVisible(true);
    }
    ui->plainTextEdit->viewport()->update(); //重绘
    ui->plainTextEdit->document()->adjustSize(); //重新适应大小
}

void Tab::prepareTextForDebug(int index)
{
    if(index!=ownIndex) return;
    emit fetchTextForDebug(ui->plainTextEdit->toPlainText());
}
