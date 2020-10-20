#ifndef TAB_H
#define TAB_H

#include <QWidget>
#include <QCompleter>
#include <QAbstractItemModel>
#include <QTextCursor>
#include <QDebug>
#include <QMap>
#include <highlighter.h>

namespace Ui {
class Tab;
}

class Tab : public QWidget
{
    Q_OBJECT

public:
    explicit Tab(QWidget *parent = 0);
    explicit Tab(int,QWidget *parent = 0);
    ~Tab();

private slots:
    void setText(QString,int);

    void prepareFetchTextForSaveAs(int);

    void prepareFetchTextForSave(int);

    void operating(int);

    void receiveSearchDataForTab(QString,int,int state,int);

    void receiveReplaceDataForTab(QString,QString,int,int state,int);

    void on_plainTextEdit_cursorPositionChanged();

    void on_plainTextEdit_blockCountChanged(int newBlockCount);

    void prepareFetchTextForFunctionTree(int);

    void on_pushButton_clicked();

    void theTextChanged();

    void jumpToFunction(int,int);

    void jumpToLine(int);

    void hideBlock(int,int,int);

    void showBlock(int,int,int);

    void prepareTextForDebug(int);

signals:
    void fetchTextForSaveAs(QString,int);
    void fetchTextForSave(QString,int);

    void tabCursorPositionChanged(QString,QString);
    void tabBlockCountChanged(QString);

    void fetchTextForFunctionTree(QString);

    void sendIndexOfChangedTab(int);

    void fetchTextForDebug(QString);

private:
    Ui::Tab *ui;
    int ownIndex;
    QAbstractItemModel *modelFromFile(const QString& fileName);

    QCompleter *completer = nullptr;
    Highlighter *highlighter; //高亮类

};

#endif // TAB_H
