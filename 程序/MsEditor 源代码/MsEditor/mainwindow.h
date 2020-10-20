#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QDateTime>
#include <qdir.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>
#include <QTreeWidget>
#include <QLabel>
#include <QMap>
#include "highlighter.h"
#include "searchdialog.h"
#include "replacedialog.h"
#include "tab.h"
#include "debugger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionSaveAs_triggered();

    void on_actionNew_triggered();

    void on_actionClose_triggered();

    void on_actionSave_triggered();

    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionCut_triggered();

    void on_actionUndo_triggered();

    void on_actionRun_triggered();

    void on_actionComplile_triggered();

    void on_actionCR_triggered();

    void on_actionSearch_triggered();

    void on_actionReplace_triggered();

    void on_pushButton_compile_clicked();

    void addAllFile(QTreeWidgetItem*,QString,QString);

    void on_actionOpenProject_triggered();

    void itemOpen(QTreeWidgetItem*,int);

    void on_actionCloseProject_triggered();

    void removeSubTab(int);

    void get_saveAs(QString,int); //从目标文本框取到字符串，开始另存为

    void get_save(QString,int); //从目标文本框取到字符串，开始保存

    void receiveSearchDataForMain(QString data,int state,int begin);   //从搜索框接收到消息

    void receiveReplaceDataForMain(QString sear,QString rep,int state,int begin); //从替换框接受到消息

    void receiveTabCursorPositionChanged(QString,QString);

    void receiveTabBlockCountChanged(QString);

    void on_actionCompileProject_triggered();

    void on_actionRunProject_triggered();

    void on_tabWidget_currentChanged(int index);

    void get_flushFunctionTree(QString);

    void receiveIndexOfChangedTab(int);

    void on_actionGBK_triggered();

    void on_actionUTF_8_triggered();

    void on_treeWidget_function_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_treeWidget_function_itemClicked(QTreeWidgetItem *item, int column);

    void on_actionCRProject_triggered();

    void on_actionDebug_triggered();

    void receiveTextForDebug(QString);

signals:
    void give(QString,int);  //把字符串发送给指定的文本框
    void pleaseFetchTextForSaveAs(int);  //告诉指定的文本框自己需要它的字符串：另存为
    void pleaseFetchTextForSave(int);  //告诉指定的文本框自己需要它的字符串：保存
    void operate(int); //告诉指定的文本框操作，0复制，1粘贴，2剪切，3撤销
    void sendSearchDataToTab(QString,int,int state,int);  //把从搜索框接受的信息转发给指定页面
    void sendReplaceDataToTab(QString,QString,int,int state,int);  //把从替换框接受的信息转发给指定页面
    void pleaseFetchTextForFunctionTree(int);  //告诉指定的文本框自己需要它的字符串：刷新函数树
    void pleaseJumpToFunction(int,int);  //告诉指定的文本框要跳转到指定函数位置
    void pleaseHideBlock(int,int,int); //告诉指定的文本框要隐藏哪一块
    void pleaseShowBlock(int,int,int); //告诉指定的文本框要展示

    void pleaseFetchTextForDebug(int);
    void giveTextToDebugger(QString);
    void giveExeName(QString);


private:

    void creatFunctionTree(QString);

    Ui::MainWindow *ui;
    QMap<int,QString> filePath; //记录不同页面和文件路径的关系

    QTextCodec *qtc;
    enum code
    {
        UTF_8,GBK
    };
    enum code flag;

    QLabel *staLabel_row;
    QLabel *staLabel_col;
    QLabel *staLabel_num;

    searchDialog *searchdialog;//搜索对话框
    replaceDialog *replacedialog;//替换对话框
    Debugger *debugger; //debug界面

    QMap<QString,QString> fileNameToPath; //项目列表中名字和目录的对应关系
    QMap<QString,QString> projectNameToPath; //项目名和项目路径的对应关系
    QMap<QString,QStringList> projectToChildren; //项目名和属于它的子文件们的路径的对应关系

    QMap<QString,int> functionStart; //函数名和所在行号的对应关系
    QMap<int,int> functionEnd; //函数开头和结尾的对应关系
    QMap<QString,bool> functionStatus; //此函数是否被隐藏
};

#endif // MAINWINDOW_H
