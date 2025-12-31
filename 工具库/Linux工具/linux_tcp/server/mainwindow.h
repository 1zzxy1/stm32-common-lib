#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
//#include <QObject>
#include <QTextCodec>
namespace Ui {
    class MainWindow;
}

class aaa : public QObject
{
  Q_OBJECT
  public:
    QTcpSocket *c;

  signals:
    void sig(QString str);
private slots:

    void sendtoall();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTcpServer *tcpServer;
    QTextCodec *local;
private:
    Ui::MainWindow *ui;


private slots:

    void newlogin();

    void newmsgreceive();

    void setstr(QString str);
};

#endif // MAINWINDOW_H
