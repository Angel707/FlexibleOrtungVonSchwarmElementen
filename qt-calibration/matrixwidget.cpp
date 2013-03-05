#include "matrixwidget.h"

MatrixWidget::MatrixWidget(QWidget * parent) :
	QTableWidget(parent)
{
	this->horizontalHeader()->hide();
	this->verticalHeader()->hide();
	this->horizontalHeader()->setDefaultSectionSize(30);
	this->verticalHeader()->setDefaultSectionSize(30);
}

void MatrixWidget::showMatrixCV(const cv::Mat & matrix)
{
	int cols = matrix.cols;
	int rows = matrix.rows;

	this->setRowCount(rows);
	this->setColumnCount(cols);

	for (int c = 0; c < cols; c++)
	{
		for (int r = 0; r < rows; r++)
		{
			//matrix.at(r,c);
			QTableWidgetItem *newItem =
					new QTableWidgetItem(tr("%1").arg((r+1)*(c+1)));
			this->setItem(r, c, newItem);
		}
	}
}
