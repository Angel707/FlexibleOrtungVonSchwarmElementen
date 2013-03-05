#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>

#include <cstdio>
#include <cstddef>
#include <cstdarg>
#include <cstring>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// Table
	ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("x")));
	ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("y")));
	ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("z")));
	ui->tableWidget->setVerticalHeaderItem(0, new QTableWidgetItem(tr("r2d2")));
	ui->tableWidget->setVerticalHeaderItem(1, new QTableWidgetItem(tr("robo")));

	timer = startTimer(1000);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButtonAddCamera_clicked()
{
	qDebug() << "Button" << ui->pushButtonAddCamera->text() << "clicked!";
	killTimer(timer);

	Camera * cam = Camera::QOpenCalibrationFile(this);
	if (!cam) return;
	ui->camView->addCamera(cam);

	timer = startTimer(100);
	qDebug() << "END: Button" << ui->pushButtonAddCamera->text() << "clicked!";
}

void MainWindow::timerEvent(QTimerEvent*)
{
	ui->camView->recordingUpdate();
}
