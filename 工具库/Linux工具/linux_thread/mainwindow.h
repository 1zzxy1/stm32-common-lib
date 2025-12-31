#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mythread.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    MyThread *aa;
private:
    Ui::MainWindow *ui;

private slots:
    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();
    void tttslot(int x,int y);
};

#endif // MAINWINDOW_H
