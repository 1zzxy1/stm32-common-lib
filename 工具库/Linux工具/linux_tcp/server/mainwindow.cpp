#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QList>

QList<QTcpSocket *> clients;

void aaa::sendtoall()
{

    QByteArray bbb=c->readAll();






    QString orinstr2=bbb;
QString orinstr=bbb;
    //QTextCodec::setCodecForTr(local);
    //QTextCodec::setCodecForLocale(local);
    //QTextCodec::setCodecForCStrings(local);


emit sig(c->peerAddress().toString()+orinstr2);

    foreach(QTcpSocket *cccc,clients)
          {
        QString str=cccc->peerAddress().toString()+orinstr;

        QByteArray block=str.toUtf8();

        cccc->write(block);
          }



}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
   local= QTextCodec::codecForLocale();
   QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
   QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
   QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));


    tcpServer = new QTcpServer(this);
    if(!tcpServer->listen(QHostAddress::LocalHost,6666))
    {
    qDebug()<<tcpServer->errorString();
    close();
     }
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(newlogin()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete tcpServer;
}
void MainWindow::newlogin()
{
   QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
   QString str="you have connected（ 连接成功）";


   QByteArray block=str.toUtf8();

   clientConnection->write(block);
   clients<<clientConnection;
   aaa *a=new aaa();
   a->c=clientConnection;


   connect(a->c,SIGNAL(readyRead()),a,SLOT(sendtoall()));
   connect(a,SIGNAL(sig(QString)),this,SLOT(setstr(QString)));
}
void MainWindow::newmsgreceive()
{
   // QMessageBox::information(this,"","new msg come");
}
void MainWindow::setstr(QString str)
{

    ui->plainTextEdit->appendPlainText(str);
}
