#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <regex>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    staLabel_row = new QLabel;
    staLabel_col = new QLabel;
    staLabel_num = new QLabel;
    staLabel_col->setText("列：0");
    staLabel_row->setText("行：0");
    staLabel_num->setText("总行数：0");
    ui->statusBar->addWidget(staLabel_row);
    ui->statusBar->addWidget(staLabel_col);
    ui->statusBar->addWidget(staLabel_num);
    searchdialog = new searchDialog(this);
    replacedialog = new replaceDialog(this);


    ui->tabWidget->setMovable(true); //设置tab页面可拖动
    ui->tabWidget->clear(); //初始化时清空所有窗口
    ui->tabWidget->setTabsClosable(true); //设置tab页面可关闭
    connect(ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int))); //接受删除信号，删除tab页面

    connect(searchdialog,SIGNAL(sendSearchDataToMain(QString,int,int)),this,SLOT(receiveSearchDataForMain(QString,int,int)));  //接受搜索信号
    connect(replacedialog,SIGNAL(sendReplaceDataToMain(QString,QString,int,int)),this,SLOT(receiveReplaceDataForMain(QString,QString,int,int)));  //接受替换信号

    ui->plainTextEdit_compile->setMaximumHeight(0); //初始时隐藏编译信息框

    connect(ui->treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(itemOpen(QTreeWidgetItem*,int)));
    //文件树项目点击信号

    flag=GBK;
}

MainWindow::~MainWindow() //析构
{
    delete ui;
}

void MainWindow::removeSubTab(int index) //删除指定index的tab页面
{
    ui->tabWidget->removeTab(index);
}

void MainWindow::on_actionOpen_triggered() //打开文件
{
    QString openPath = QDir::currentPath(); //打开时的初始文件目录
    QString title = "打开文件"; //对话框标题
    QString filter="程序文件(*.cpp *.h);;文本文件(*.txt);;所有文件(*.*)"; //文件过滤器
    QString openfilePath=QFileDialog::getOpenFileName(this,title,openPath,filter);

    //qDebug()<<openfilePath<<endl;

    if (openfilePath.isEmpty())
    {
        QMessageBox::warning(this,"提示","打开文件为空");
        return;
    }

    //用IODevice方式打开文本文件
    QFile   openFile(openfilePath);

    if (!openFile.exists()) //文件不存在
    {
        QMessageBox::warning(this,"提示","文件不存在");
        return;
    }
    if (!openFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"提示","文件不能以只读或文本形式打开");
        return;
    }

    QByteArray  strBytes = openFile.readAll();
    QString str;
    if(flag==GBK)
    {
        qtc=QTextCodec::codecForName("GBK");
        str = qtc->toUnicode(strBytes);
    }
    else if(flag==UTF_8)
    {
        str = strBytes;
    }

    openFile.close();


    QStringList sl = openfilePath.split("/");
    int length = sl.length();
    QString singleFileName = sl.at(length - 1);

    Tab *tab = new Tab(ui->tabWidget->currentIndex()+1);
    int index = ui->tabWidget->insertTab(ui->tabWidget->currentIndex()+1,tab,singleFileName);
    ui->tabWidget->setCurrentIndex(index);

    filePath.insert(index,openfilePath);

    connect(this,SIGNAL(give(QString,int)),tab,SLOT(setText(QString,int)));
    connect(this,SIGNAL(pleaseFetchTextForSaveAs(int)),tab,SLOT(prepareFetchTextForSaveAs(int)));
    connect(tab,SIGNAL(fetchTextForSaveAs(QString,int)),this,SLOT(get_saveAs(QString,int)));
    connect(this,SIGNAL(pleaseFetchTextForSave(int)),tab,SLOT(prepareFetchTextForSave(int)));
    connect(tab,SIGNAL(fetchTextForSave(QString,int)),this,SLOT(get_save(QString,int)));
    connect(this,SIGNAL(operate(int)),tab,SLOT(operating(int)));
    connect(this,SIGNAL(sendSearchDataToTab(QString,int,int,int)),tab,SLOT(receiveSearchDataForTab(QString,int,int,int)));
    connect(this,SIGNAL(sendReplaceDataToTab(QString,QString,int,int,int)),tab,SLOT(receiveReplaceDataForTab(QString,QString,int,int,int)));
    connect(tab,SIGNAL(tabCursorPositionChanged(QString,QString)),this,SLOT(receiveTabCursorPositionChanged(QString,QString)));
    connect(tab,SIGNAL(tabBlockCountChanged(QString)),this,SLOT(receiveTabBlockCountChanged(QString)));
    connect(this,SIGNAL(pleaseFetchTextForFunctionTree(int)),tab,SLOT(prepareFetchTextForFunctionTree(int)));
    connect(tab,SIGNAL(fetchTextForFunctionTree(QString)),this,SLOT(get_flushFunctionTree(QString)));
    connect(tab,SIGNAL(sendIndexOfChangedTab(int)),this,SLOT(receiveIndexOfChangedTab(int)));
    connect(this,SIGNAL(pleaseJumpToFunction(int,int)),tab,SLOT(jumpToFunction(int,int)));
    connect(this,SIGNAL(pleaseHideBlock(int,int,int)),tab,SLOT(hideBlock(int,int,int)));
    connect(this,SIGNAL(pleaseShowBlock(int,int,int)),tab,SLOT(showBlock(int,int,int)));
    connect(this,SIGNAL(pleaseFetchTextForDebug(int)),tab,SLOT(prepareTextForDebug(int)));
    connect(tab,SIGNAL(fetchTextForDebug(QString)),this,SLOT(receiveTextForDebug(QString)));

    emit give(str,index);

    creatFunctionTree(str);

    QString text = filePath.find(index).value(); //保存，去掉星号
    int l = text.lastIndexOf("/");
    text=text.right(text.size()-l-1);
    ui->tabWidget->setTabText(index,text);

    return;
}

void MainWindow::on_actionOpenProject_triggered() //打开项目
{
    QString projectPath = QDir::currentPath(); //打开时的初始文件目录
    QString title = "打开项目"; //对话框标题
    QString projectfilePath=QFileDialog::getExistingDirectory(this,title,projectPath);

    QString rootname=projectfilePath;
    int l = rootname.lastIndexOf("/");
    rootname = rootname.right(rootname.size()-l-1);
    projectNameToPath.insert(rootname,projectfilePath);
    ui->label_nowproject->setText(rootname);

    QTreeWidgetItem* root = new QTreeWidgetItem(QStringList()<<rootname);
    root->setIcon(0, QIcon(":/folder.png"));
    root->setCheckState(1, Qt::Checked);
    addAllFile(root,rootname,projectfilePath);        //遍历添加目录下所有文件
    ui->treeWidget->insertTopLevelItem(0,root);
    ui->treeWidget->setHeaderLabel("项目");

    //qDebug()<<fileNameToPath<<endl;

}

void MainWindow::on_actionCloseProject_triggered() //关闭项目
{
    ui->treeWidget->clear();
    ui->label_nowproject->setText("无");
}

void MainWindow::addAllFile(QTreeWidgetItem *root,QString rootname,QString path)     //制作文件树
{

    QDir dir(path);    //获取当前目录
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks,
                                                     QDir::Size | QDir::Reversed);   //获取文件信息列表

    QTreeWidgetItem* hroot = new QTreeWidgetItem(QStringList()<<"Headers");
    hroot->setIcon(0, QIcon(":/folder.png"));
    QTreeWidgetItem* sroot = new QTreeWidgetItem(QStringList()<<"Sources");
    sroot->setIcon(0, QIcon(":/folder.png"));
    root->addChild(hroot);
    root->addChild(sroot);
    QStringList childrenNameList;

    for (int i = 0; i < file_list.size(); ++i)  //将当前目录中所有文件添加到treewidget中
    {
        QFileInfo fileInfo = file_list.at(i);
        QString name=fileInfo.fileName();

        fileNameToPath.insert(name,fileInfo.absoluteFilePath());

        QTreeWidgetItem* child = new QTreeWidgetItem(QStringList()<<name);
        child->setCheckState(1, Qt::Checked);

        int l = name.lastIndexOf(".");
        QString suffix = name.right(name.size()-l-1);
        if(suffix == "h" )
        {
            child->setIcon(0, QIcon(":/h.png"));
            hroot->addChild(child);
        }
        else if(suffix == "cpp")
        {
            child->setIcon(0, QIcon(":/cpp.png"));
            sroot->addChild(child);
            childrenNameList.append(fileInfo.absoluteFilePath());

        }
    }
    projectToChildren.insert(rootname,childrenNameList);
    return;


// 默认项目文件下只有.hpp,.cpp等文件而没有子文件夹，故不再向下递归以。在项目窗口中将所有文件按照c++项目结构分类展示。
//    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);   //获取文件夹信息

//    for(int i = 0; i != folder_list.size(); i++)         //递归添加各个子目录到上一级目录
//    {
//        QFileInfo folderinfo = folder_list.at(i);
//        QString path = folderinfo.absoluteFilePath();
//        QString name=folderinfo.fileName();
//        QTreeWidgetItem* childroot = new QTreeWidgetItem(QStringList()<<name);
//        childroot->setIcon(0, QIcon(":/folder.png"));
//        childroot->setCheckState(1, Qt::Checked);
//        root->addChild(childroot);              //将当前目录添加成path的子项
//        addAllFile(childroot,path);          //进行递归
//     }
}

void MainWindow::itemOpen(QTreeWidgetItem* item,int column) //打开被双击的item
{

    QString text = item->text(column);
    if(fileNameToPath.find(text) == fileNameToPath.end())
    {
        if(text == "Headers" || text == "Sources") return;
        else ui->label_nowproject->setText(text);
        return;
    }
    QString itemPath = fileNameToPath.find(item->text(column)).value();
    //用IODevice方式打开文本文件
    QFile   itemFile(itemPath);

    if (!itemFile.exists()) //文件不存在
    {
        QMessageBox::warning(this,"提示","文件不存在");
        return;
    }
    if (!itemFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"提示","文件不能以只读或文本形式打开");
        return;
    }

    QByteArray  strBytes = itemFile.readAll();
    QString str;
    if(flag==GBK)
    {
        qtc=QTextCodec::codecForName("GBK");
        str = qtc->toUnicode(strBytes);
    }
    else if(flag==UTF_8)
    {
        str = strBytes;
    }

    itemFile.close();

    QStringList sl = itemPath.split("/");
    int length = sl.length();
    QString singleFileName = sl.at(length - 1);

    Tab *tab = new Tab(ui->tabWidget->currentIndex()+1);
    int index = ui->tabWidget->insertTab(ui->tabWidget->currentIndex()+1,tab,singleFileName);
    ui->tabWidget->setCurrentIndex(index);

    filePath.insert(index,itemPath);

    connect(this,SIGNAL(give(QString,int)),tab,SLOT(setText(QString,int)));
    connect(this,SIGNAL(pleaseFetchTextForSaveAs(int)),tab,SLOT(prepareFetchTextForSaveAs(int)));
    connect(tab,SIGNAL(fetchTextForSaveAs(QString,int)),this,SLOT(get_saveAs(QString,int)));
    connect(this,SIGNAL(pleaseFetchTextForSave(int)),tab,SLOT(prepareFetchTextForSave(int)));
    connect(tab,SIGNAL(fetchTextForSave(QString,int)),this,SLOT(get_save(QString,int)));
    connect(this,SIGNAL(operate(int)),tab,SLOT(operating(int)));
    connect(this,SIGNAL(sendSearchDataToTab(QString,int,int,int)),tab,SLOT(receiveSearchDataForTab(QString,int,int,int)));
    connect(this,SIGNAL(sendReplaceDataToTab(QString,QString,int,int,int)),tab,SLOT(receiveReplaceDataForTab(QString,QString,int,int,int)));
    connect(tab,SIGNAL(tabCursorPositionChanged(QString,QString)),this,SLOT(receiveTabCursorPositionChanged(QString,QString)));
    connect(tab,SIGNAL(tabBlockCountChanged(QString)),this,SLOT(receiveTabBlockCountChanged(QString)));
    connect(this,SIGNAL(pleaseFetchTextForFunctionTree(int)),tab,SLOT(prepareFetchTextForFunctionTree(int)));
    connect(tab,SIGNAL(fetchTextForFunctionTree(QString)),this,SLOT(get_flushFunctionTree(QString)));
    connect(tab,SIGNAL(sendIndexOfChangedTab(int)),this,SLOT(receiveIndexOfChangedTab(int)));
    connect(this,SIGNAL(pleaseJumpToFunction(int,int)),tab,SLOT(jumpToFunction(int,int)));
    connect(this,SIGNAL(pleaseHideBlock(int,int,int)),tab,SLOT(hideBlock(int,int,int)));
    connect(this,SIGNAL(pleaseShowBlock(int,int,int)),tab,SLOT(showBlock(int,int,int)));
    connect(this,SIGNAL(pleaseFetchTextForDebug(int)),tab,SLOT(prepareTextForDebug(int)));
    connect(tab,SIGNAL(fetchTextForDebug(QString)),this,SLOT(receiveTextForDebug(QString)));

    emit give(str,index);

    creatFunctionTree(str);

    QString name = filePath.find(index).value(); //保存，去掉星号
    int l = name.lastIndexOf("/");
    name=name.right(name.size()-l-1);
    ui->tabWidget->setTabText(index,name);

    return;
}

void MainWindow::on_actionSaveAs_triggered()  //另存为
{

    int index = ui->tabWidget->currentIndex();
    emit pleaseFetchTextForSaveAs(index);
    return;
}

void MainWindow::get_saveAs(QString str, int index) //接受到文本框发动的信息后进行另存为操作
{
    QString curPath=QDir::currentPath(); //获取当前目录
    QString title="另存为一个文件"; //对话框标题
    QString filter="程序文件(*.cpp *.h);;文本文件(*.txt);;所有文件(*.*)"; //文件过滤器
    QString savefilePath=QFileDialog::getSaveFileName(this,title,curPath,filter);
    if (savefilePath.isEmpty())
       return;

    //用IODevice方式保存文本文件
    QFile saveFile(savefilePath);
    saveFile.open(QIODevice::ReadWrite | QIODevice::Text );

    QByteArray strBytes;
    if(flag==GBK)
    {
        qtc = QTextCodec::codecForName("GBK");
        strBytes= qtc->fromUnicode(str); //转为ANSI GBK编码
    }
    else if(flag==UTF_8)
    {
        strBytes = str.toUtf8();
    }

    saveFile.write(strBytes,strBytes.length());     //写入文件

    saveFile.close();

    creatFunctionTree(str);

    QString text = filePath.find(index).value(); //保存，去掉星号
    int l = text.lastIndexOf("/");
    text=text.right(text.size()-l-1);
    ui->tabWidget->setTabText(index,text);
}

void MainWindow::on_actionSave_triggered() //保存文件
{
    int index = ui->tabWidget->currentIndex();
    emit  pleaseFetchTextForSave(index);

}

void MainWindow::get_save(QString str, int index) //接受到文本框发动的信息后进行保存操作
{
    QFile saveFile(filePath.find(index).value());
    saveFile.open(QIODevice::ReadWrite | QIODevice::Text );

    QByteArray strBytes;
    if(flag==GBK)
    {
        qtc = QTextCodec::codecForName("GBK");
        strBytes= qtc->fromUnicode(str); //转为ANSI GBK编码
    }
    else if(flag==UTF_8)
    {
        strBytes = str.toUtf8();
    }

    saveFile.write(strBytes,strBytes.length());     //写入文件

    saveFile.close();

    creatFunctionTree(str);

    QString text = filePath.find(index).value(); //保存，去掉星号
    int l = text.lastIndexOf("/");
    text=text.right(text.size()-l-1);
    ui->tabWidget->setTabText(index,text);
}


void MainWindow::on_actionNew_triggered() //新建文件
{
    QString curPath=QDir::currentPath(); //获取当前目录
    QString title="新建一个文件"; //对话框标题
    QString filter="程序文件(*.cpp *.h);;文本文件(*.txt);;所有文件(*.*)"; //文件过滤器
    QString newfilePath=QFileDialog::getSaveFileName(this,title,curPath,filter);
    if (newfilePath.isEmpty())
       return;

    //用IODevice方式保存文本文件
    QFile newFile(newfilePath);
    if(newFile.exists())
    {
        QMessageBox::warning(this,"提示","文件存在");
    }else{
        newFile.open( QIODevice::ReadWrite | QIODevice::Text );
    }
    newFile.close();

    QStringList sl = newfilePath.split("/");
    int length = sl.length();
    QString singleFileName = sl.at(length - 1);

    Tab *tab = new Tab(ui->tabWidget->currentIndex()+1);
    int index = ui->tabWidget->insertTab(ui->tabWidget->currentIndex()+1,tab,singleFileName);
    ui->tabWidget->setCurrentIndex(index);

    filePath.insert(index,newfilePath);

    connect(this,SIGNAL(give(QString,int)),tab,SLOT(setText(QString,int)));
    connect(this,SIGNAL(pleaseFetchTextForSaveAs(int)),tab,SLOT(prepareFetchTextForSaveAs(int)));
    connect(tab,SIGNAL(fetchTextForSaveAs(QString,int)),this,SLOT(get_saveAs(QString,int)));
    connect(this,SIGNAL(pleaseFetchTextForSave(int)),tab,SLOT(prepareFetchTextForSave(int)));
    connect(tab,SIGNAL(fetchTextForSave(QString,int)),this,SLOT(get_save(QString,int)));
    connect(this,SIGNAL(operate(int)),tab,SLOT(operating(int)));
    connect(this,SIGNAL(sendSearchDataToTab(QString,int,int,int)),tab,SLOT(receiveSearchDataForTab(QString,int,int,int)));
    connect(this,SIGNAL(sendReplaceDataToTab(QString,QString,int,int,int)),tab,SLOT(receiveReplaceDataForTab(QString,QString,int,int,int)));
    connect(tab,SIGNAL(tabCursorPositionChanged(QString,QString)),this,SLOT(receiveTabCursorPositionChanged(QString,QString)));
    connect(tab,SIGNAL(tabBlockCountChanged(QString)),this,SLOT(receiveTabBlockCountChanged(QString)));
    connect(this,SIGNAL(pleaseFetchTextForFunctionTree(int)),tab,SLOT(prepareFetchTextForFunctionTree(int)));
    connect(tab,SIGNAL(fetchTextForFunctionTree(QString)),this,SLOT(get_flushFunctionTree(QString)));
    connect(tab,SIGNAL(sendIndexOfChangedTab(int)),this,SLOT(receiveIndexOfChangedTab(int)));
    connect(this,SIGNAL(pleaseJumpToFunction(int,int)),tab,SLOT(jumpToFunction(int,int)));
    connect(this,SIGNAL(pleaseHideBlock(int,int,int)),tab,SLOT(hideBlock(int,int,int)));
    connect(this,SIGNAL(pleaseShowBlock(int,int,int)),tab,SLOT(showBlock(int,int,int)));
    connect(this,SIGNAL(pleaseFetchTextForDebug(int)),tab,SLOT(prepareTextForDebug(int)));
    connect(tab,SIGNAL(fetchTextForDebug(QString)),this,SLOT(receiveTextForDebug(QString)));

    emit give("",index);

    QString text = filePath.find(index).value(); //保存，去掉星号
    int l = text.lastIndexOf("/");
    text=text.right(text.size()-l-1);
    ui->tabWidget->setTabText(index,text);

    return;
}

void MainWindow::on_actionClose_triggered() //关闭文件
{
    int index = ui->tabWidget->currentIndex();
    emit give("",index);
    on_actionSave_triggered();
    removeSubTab(index);
}



void MainWindow::on_actionCopy_triggered() //复制
{
    emit operate(0);
}

void MainWindow::on_actionPaste_triggered() //粘贴
{
    emit operate(1);
}

void MainWindow::on_actionCut_triggered() //剪切
{
    emit operate(2);
}

void MainWindow::on_actionUndo_triggered() //撤销
{
    emit operate(3);
}

void MainWindow::on_actionComplile_triggered() //编译
{
    if(ui->tabWidget->count() == 0)
    {
        QMessageBox::warning(this,"警告","还没有打开任何文件");
        return;
    }

    on_actionSave_triggered(); //自动保存
    ui->plainTextEdit_compile->setMaximumHeight(220); //打开编译信息框

    int index = ui->tabWidget->currentIndex();
    QString nowFilePath = filePath.find(index).value();
    QString exe = nowFilePath;
    exe.replace(".cpp","");
    exe+=".exe";
    QString cmd = QString("g++ %1 -o %2 -g").arg(nowFilePath).arg(exe) + " 2> tmp.txt";   //使用 2> 输出错误信息
    int r=system(cmd.toStdString().c_str());
    if(r == 0)
    {
        QFileInfo info(exe);
        float size=info.size();
        qDebug()<<size<<endl;
        QString unit ="B";
        if(size>1024)
        {
            size/=1024;
            unit ="KB";
            if(size>1024)
            {
                size/=1024;
                unit ="MB";
                if(size>1024)
                {
                    size/=1024;
                    unit ="GB";
                }
            }
        }
        ui->plainTextEdit_compile->setPlainText("编译成功 \n"
                                           "输出文件名："+exe+"\n"
                                           "最后修改时间："+info.lastModified().toString("yyyy-MM-dd hh:mm:ss")+"\n"
                                           "文件大小："+QString::number(size)+unit+"\n");
        //QMessageBox::information(this,"提示","编译成功");
    }
    else
    {
        QFile tmpFile("tmp.txt");
        tmpFile.open(QIODevice::ReadWrite | QIODevice::Text);
        ui->plainTextEdit_compile->setPlainText(tmpFile.readAll());
        tmpFile.close();
        QMessageBox::information(this,"提示","编译失败");
    }
    system("del tmp.txt");
}


void MainWindow::on_actionCompileProject_triggered()  //项目编译
{
    QString projectName = ui->label_nowproject->text();
    if(projectName == "无")
    {
        QMessageBox::warning(this,"警告","还没有打开任何项目");
        return;
    }
    on_actionSave_triggered(); //自动保存
    ui->plainTextEdit_compile->setMaximumHeight(220); //打开编译信息框

    QString projectPath = projectNameToPath.find(projectName).value();
    QString cmd = "g++ ";
    QStringList list = projectToChildren.find(projectName).value();
    for(int i=0;i<list.size();i++)
    {
        cmd += list[i];
        cmd += " ";
    }
    QString exe = projectPath + "/" +projectName+".exe";
    cmd += QString(" -o %1 -g").arg(exe);
    cmd += " 2> tmp.txt";
    qDebug()<<cmd<<endl;
    int r=system(cmd.toStdString().c_str());
    if(r == 0)
    {
        QFileInfo info(exe);
        float size=info.size();
        qDebug()<<size<<endl;
        QString unit ="B";
        if(size>1024)
        {
            size/=1024;
            unit ="KB";
            if(size>1024)
            {
                size/=1024;
                unit ="MB";
                if(size>1024)
                {
                    size/=1024;
                    unit ="GB";
                }
            }
        }
        ui->plainTextEdit_compile->setPlainText("编译成功 \n"
                                           "输出文件名："+exe+"\n"
                                           "最后修改时间："+info.lastModified().toString("yyyy-MM-dd hh:mm:ss")+"\n"
                                           "文件大小："+QString::number(size)+unit+"\n");
        //QMessageBox::information(this,"提示","编译成功");
    }
    else
    {
        QFile tmpFile("tmp.txt");
        tmpFile.open(QIODevice::ReadWrite | QIODevice::Text);
        ui->plainTextEdit_compile->setPlainText(tmpFile.readAll());
        tmpFile.close();
        QMessageBox::information(this,"提示","编译失败");
    }
    system("del tmp.txt");
}

void MainWindow::on_actionRun_triggered() //运行
{
    if(ui->tabWidget->count() == 0)
    {
        QMessageBox::warning(this,"警告","还没有打开任何文件");
        return;
    }

    int index = ui->tabWidget->currentIndex();
    QString nowFilePath = filePath.find(index).value();
    QString exe = nowFilePath;
    exe.replace(".cpp","");
    exe += ".exe";
    QFile exefile(exe);
    if(!exefile.exists())
    {
        QMessageBox::warning(this,"提示","还没有进行编译");
    }
    QString cmd = QString("%1").arg(exe) + " && pause";
    //qDebug()<<cmd<<endl;
    int r=system(cmd.toStdString().c_str());
    if(r == 0)
    {
        //QMessageBox::information(this,"提示","运行成功");
    }
    else
    {
        QMessageBox::information(this,"提示","运行失败");
    }
}

void MainWindow::on_actionRunProject_triggered()  //项目运行
{
    QString projectName = ui->label_nowproject->text();
    if(projectName == "无")
    {
        QMessageBox::warning(this,"警告","还没有打开任何项目");
        return;
    }
    QString projectPath = projectNameToPath.find(projectName).value();
    QString exe = projectPath + "/" +projectName+".exe";
    QString cmd = QString("%1").arg(exe) + " && pause";
    qDebug()<<cmd<<endl;
    int r=system(cmd.toStdString().c_str());
    if(r == 0)
    {
        //QMessageBox::information(this,"提示","运行成功");
    }
    else
    {
        QMessageBox::information(this,"提示","运行失败");
    }

}


void MainWindow::on_actionCR_triggered() //编译运行
{
    on_actionComplile_triggered();
    on_actionRun_triggered();
}

void MainWindow::receiveSearchDataForMain(QString data,int state,int begin) //从搜索对话框接收搜索数据，发送给指定页面
{
    int index = ui->tabWidget->currentIndex();
    emit sendSearchDataToTab(data,index,state,begin);
}



void MainWindow::receiveReplaceDataForMain(QString sear, QString rep,int state,int begin) //接受替换数据
{
    int index = ui->tabWidget->currentIndex();
    emit sendReplaceDataToTab(sear,rep,index,state,begin);
}

void MainWindow::on_actionSearch_triggered() //搜索
{
    searchdialog->setModal(false);  //设置为非模态对话框,在其没有关闭前，用户可以与其它窗口交互
    searchdialog->show();
}

void MainWindow::on_actionReplace_triggered() //替换
{
    replacedialog->setModal(false);
    replacedialog->show();
}

void MainWindow::on_pushButton_compile_clicked() //点击编译信息按钮,切换编译信息界面弹出/收回状态
{
    int height = ui->plainTextEdit_compile->height();
    if(height>0)
    {
        ui->plainTextEdit_compile->setMaximumHeight(0);
    }
    else if(height == 0)
    {
        ui->plainTextEdit_compile->setMaximumHeight(220);
        ui->plainTextEdit_compile->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    }
}

void MainWindow::receiveTabCursorPositionChanged(QString str_row, QString str_col) //状态栏行列变化信号
{
    staLabel_row->setText(str_row);
    staLabel_col->setText(str_col);
}

void MainWindow::receiveTabBlockCountChanged(QString str_num) //状态栏总行数变化信号
{
   staLabel_num->setText(str_num);
}

void MainWindow::creatFunctionTree(QString qstr) //创建函数树
{
    //qDebug()<<"enter_creatFunctionTree"<<endl;
    QStringList rows = qstr.split("\n");
    //qDebug()<<rows<<endl;

    QList<QTreeWidgetItem*> functionList;

    std::smatch result;
    std::regex pattern("(int|void|float|double)\\s+(\\w+|\\w+::\\w+)\\s*(\\(.*?\\))(?=\\s*)"); //找到函数头
    int num = rows.size();
    int pre=0;
    for(int i=0;i<num;++i)
    {
        std::string str = rows.at(i).toStdString();

        std::string::const_iterator iterStart = str.begin();
        std::string::const_iterator iterEnd = str.end();
        QString temp;

        while(std::regex_search(iterStart,iterEnd,result,pattern))
        {
            temp = QString::fromStdString(result[0]);
            if(!functionStart.isEmpty())
            {
                functionEnd.insert(pre,i-1);
            }
            functionStart.insert(temp,i+1);
            pre=i+1;
            functionStatus.insert(temp,false);
            QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<temp);
            item->setIcon(0, QIcon(":/function.png"));
            functionList.append(item);
            //qDebug()<<temp<<endl;
            iterStart=result[0].second;
        }
    }
    functionEnd.insert(pre,num-1);
    //qDebug()<<functionStart<<endl;
    ui->treeWidget_function->clear();
    ui->treeWidget_function->addTopLevelItems(functionList);
}

void MainWindow::on_tabWidget_currentChanged(int index) //页面切换，发出信号
{
    if(index==-1) ui->treeWidget_function->clear();
    emit pleaseFetchTextForFunctionTree(index);
}

void MainWindow::get_flushFunctionTree(QString str) //收到文本，更新函数树
{
    creatFunctionTree(str);
}

void MainWindow::on_treeWidget_function_itemDoubleClicked(QTreeWidgetItem *item, int column) //函数树的项目被双击时
{
    QString functionName = item->text(column);
    int start = functionStart.find(functionName).value();
    int end = functionEnd.find(start).value();

    if(!functionStatus.find(functionName).value())
    {
        emit pleaseHideBlock(ui->tabWidget->currentIndex(),start,end);
        functionStatus.find(functionName).value()=true;
    }
    else
    {
        emit pleaseShowBlock(ui->tabWidget->currentIndex(),start,end);
        functionStatus.find(functionName).value()=false;
    }

}

void MainWindow::receiveIndexOfChangedTab(int index) //收到文本发生变化的页面的index
{
    QString text = filePath.find(index).value();
    int l = text.lastIndexOf("/");
    text=text.right(text.size()-l-1);
    text = "*"+text;
    ui->tabWidget->setTabText(index,text);
}

void MainWindow::on_actionGBK_triggered()
{
    flag = GBK;
}

void MainWindow::on_actionUTF_8_triggered()
{
    flag = UTF_8;
}

void MainWindow::on_treeWidget_function_itemClicked(QTreeWidgetItem *item, int column) //函数树的项目被单击时
{
    QString function = item->text(column);
    int line = functionStart.find(function).value();
    //qDebug()<<line<<endl;
    int index = ui->tabWidget->currentIndex();
    emit pleaseJumpToFunction(index,line);
}

void MainWindow::on_actionCRProject_triggered()
{
    on_actionCompileProject_triggered();
    on_actionRunProject_triggered();
}

void MainWindow::on_actionDebug_triggered() //调试功能
{
    if(filePath.find(ui->tabWidget->currentIndex()) == filePath.end())
    {
        QMessageBox::warning(this,"提示","还未打开任何文件还未编译当前页面文件，无法调试");
        return;
    }

    QString exename = filePath.find(ui->tabWidget->currentIndex()).value();
    exename.replace(".cpp",".exe");
    QFile exefile(exename);
    if(!exefile.exists())
    {
        QMessageBox::warning(this,"提示","还未编译当前页面文件，无法调试");
        return;
    }

    debugger = new Debugger(this);

    debugger->show();

    connect(this,SIGNAL(giveTextToDebugger(QString)),debugger,SLOT(setText(QString)));
    connect(this,SIGNAL(giveExeName(QString)),debugger,SLOT(runGDB(QString)));

    int index = ui->tabWidget->currentIndex();
    emit pleaseFetchTextForDebug(index); //请求文本
    emit giveExeName(exename);


}

void MainWindow::receiveTextForDebug(QString str) //把文本发给debug窗口
{
    emit giveTextToDebugger(str);
}
