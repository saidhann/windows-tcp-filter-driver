#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QHostAddress>
#include <QString>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dll("ComunicationLibrary.dll") // Replace with the actual DLL name
    , createServiceInstance(nullptr)
    , deleteServiceInstance(nullptr)
    , getTCPData(nullptr)
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Load the DLL
    if (!dll.load()) {
        QMessageBox::critical(this, "Error", "Failed to load DLL");
        return;
    }

    // Resolve the CreateTcpInterceptionService function
    createServiceInstance = (CreateTcpInterceptionServiceFunc)dll.resolve("CreateTcpInterceptionService");
    if (!createServiceInstance) {
        QMessageBox::critical(this, "Error", "Failed to resolve CreateTcpInterceptionService function");
        return;
    }

    // Resolve the DeleteTcpInterceptionService function
    deleteServiceInstance = (DeleteTcpInterceptionServiceFunc)dll.resolve("DeleteTcpInterceptionService");
    if (!deleteServiceInstance) {
        QMessageBox::critical(this, "Error", "Failed to resolve DeleteTcpInterceptionService function");
        return;
    }

    // Resolve the GetTCPData function
    getTCPData = (GetTCPDataFunc)dll.resolve("GetTCPData");
    if (!getTCPData) {
        QMessageBox::critical(this, "Error", "Failed to resolve GetTCPData function");
        return;
    }

    // Create the service
    if (!createServiceInstance()) {
        QMessageBox::critical(this, "Error", "Failed to create service");
        DWORD errorCode = GetLastError();
        QMessageBox::critical(this, "Error code: ", QString::number(errorCode));
        return;
    }

    // Set up the table widget
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels({"Local Address", "Local Port", "Remote Address", "Remote Port"});

    // Load TCP data initially
    loadTCPData();

    // Set up a timer to call loadTCPData every 30 seconds
    connect(timer, &QTimer::timeout, this, &MainWindow::loadTCPData);
    timer->start(1000); // 30 seconds
}

MainWindow::~MainWindow()
{
    if (deleteServiceInstance) {
        deleteServiceInstance();
    }
    delete ui;
}

void MainWindow::loadTCPData()
{
    TCPData data[100];
    ULONG dataSize = sizeof(data);
    auto result = getTCPData(data, &dataSize);

    //QMessageBox::critical(this, "result",  QString::number(result));
    //QMessageBox::critical(this, "datasize",  QString::number(dataSize));


    if (dataSize && result) {
        int rowCount = dataSize;
        int currentRowCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(currentRowCount + rowCount);

        for (int i = 0; i < rowCount; ++i) {
            ui->tableWidget->setItem(currentRowCount + i, 0, new QTableWidgetItem(QHostAddress(data[i].localAddress).toString()));
            ui->tableWidget->setItem(currentRowCount + i, 1, new QTableWidgetItem(QString::number(data[i].localPort)));
            ui->tableWidget->setItem(currentRowCount + i, 2, new QTableWidgetItem(QHostAddress(data[i].remoteAddress).toString()));
            ui->tableWidget->setItem(currentRowCount + i, 3, new QTableWidgetItem(QString::number(data[i].remotePort)));
        }
    }
    if(!result) QMessageBox::critical(this, "Error", "Failed to get TCP data");
}


