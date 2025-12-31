#include "mainwindow.h"
#include "ui_mainwindow.h"

int xx,yy;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_2_clicked()
{
    aa=new MyThread();
    aa->start();

    connect(aa,SIGNAL(ttt(int,int)),this,SLOT(tttslot(int,int)));

}
void MainWindow::tttslot(int x,int y)
{

            ui->pushButton->move(x,y);

}

void MainWindow::on_pushButton_3_clicked()
{
    aa->terminate();
}
