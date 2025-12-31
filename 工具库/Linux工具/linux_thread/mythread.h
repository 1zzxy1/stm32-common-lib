#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

class MyThread : public QThread
{
Q_OBJECT
public:
    MyThread();
    int xx;
    int yy;
protected:
void run();
signals:
    void ttt(int x,int y);

};

#endif // MYTHREAD_H
