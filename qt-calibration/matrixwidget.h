#ifndef MATRIXWIDGET_H
#define MATRIXWIDGET_H

#include <QObject>
#include <QtGui>
#include <opencv2/opencv.hpp>

class MatrixWidget : public QTableWidget
{
	Q_OBJECT
public:
	MatrixWidget(QWidget * parent = 0);
	~MatrixWidget(){}
	void showMatrixCV(const cv::Mat & matrix);

signals:

public slots:

private:

};

#endif // MATRIXWIDGET_H
