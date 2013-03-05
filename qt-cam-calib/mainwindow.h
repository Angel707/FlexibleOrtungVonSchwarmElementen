#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QtGui>

#include <opencv2/opencv.hpp>
#include "../Camera/Camera.h"
#include "../Camera/CamCalibration.h"
#include "../Camera/cameraview.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButtonAddCamera_clicked();

private:
	Ui::MainWindow *ui;
	int timer;

	void timerEvent(QTimerEvent*);
};

#endif // MAINWINDOW_H
