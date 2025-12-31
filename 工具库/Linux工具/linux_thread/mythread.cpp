#include "mythread.h"
#include <QDebug>
#include <QTime>
void MyThread::run()
{
    while(1){
   // qDebug()<<"aaaaaa";
        xx=qrand()%400;

        yy=qrand()%300;


     emit ttt(xx,yy);
msleep(500);  //在线程内有效，如果线程外则无该函数，应该属于线程的方法。
}
}
MyThread::MyThread()
{
    QTime time;
    time= QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
}
