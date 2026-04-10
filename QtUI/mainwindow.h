#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLibrary>
#include <QTimer>
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct TCPData {
    UINT32 localAddress;
    UINT16 localPort;
    UINT32 remoteAddress;
    UINT16 remotePort;
};

typedef BOOL (*CreateTcpInterceptionServiceFunc)();
typedef BOOL (*DeleteTcpInterceptionServiceFunc)();
typedef BOOL (*GetTCPDataFunc)(PVOID, ULONG*);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadTCPData();

private:
    Ui::MainWindow *ui;
    QLibrary dll;
    CreateTcpInterceptionServiceFunc createServiceInstance;
    DeleteTcpInterceptionServiceFunc deleteServiceInstance;
    GetTCPDataFunc getTCPData;
    QTimer *timer;
};

#endif // MAINWINDOW_H



