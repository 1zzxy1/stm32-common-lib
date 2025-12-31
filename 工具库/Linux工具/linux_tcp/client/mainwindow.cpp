#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    local=QTextCodec::codecForLocale();

     tcpSocket=new QTcpSocket(this);
int flag;

     do{
       flag=1;
      tcpSocket->abort();
     tcpSocket->connectToHost(QHostAddress("127.0.0.1"),6666);

     if(!tcpSocket->waitForConnected(2000)){
         flag=0;
         qDebug()<<"cannot connect";
         sleep(5);
     }
 }while(flag==0);
qDebug()<<"aaaaa";

     connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(newmsg()));



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{


QString str=ui->lineEdit->text();
QByteArray block=str.toUtf8();

   tcpSocket->write(block);


}
void MainWindow::newmsg()
{
    QByteArray bb=tcpSocket->readAll();

    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));




    QString str=bb;

   // QTextCodec::setCodecForTr(local);
   // QTextCodec::setCodecForLocale(local);
  //  QTextCodec::setCodecForCStrings(local);




   ui->plainTextEdit->appendPlainText(str);
}

void MainWindow::on_pushButton_2_clicked()
{
    qDebug()<<tcpSocket->state();
}
