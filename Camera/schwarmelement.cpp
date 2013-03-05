#include "schwarmelement.h"

#include <QtDebug>
#include <assert.h>

SchwarmElement::SchwarmElement()
{
}

bool SchwarmElement::reconstruct(
	const cv::Mat & leftImage, const cv::Mat & rightImage,
	Camera * cam, const cv::Point2f & refPoint)
{
	assert(cam);
	assert(!leftImage.empty());
	assert(!rightImage.empty());
	assert(leftImage.size() == rightImage.size());

	// Match Keypoints -> Points 2D (Image left and right)
	RobustMatcher rmatcher;
	rmatcher.setConfidenceLevel(0.98); // default: 0.99
	rmatcher.setMinDistanceToEpipolar(1.0); // default: 3.0
	rmatcher.setRatio(0.65f); // default
	cv::Ptr<cv::FeatureDetector> pfd = new cv::SurfFeatureDetector(10);
	rmatcher.setFeatureDetector(pfd);
	// Match the two images
	std::vector<cv::DMatch> matches;
	std::vector<cv::Point2f> pointsLeft, pointsRight;
	cv::Mat F = rmatcher.match(leftImage, rightImage,
							   matches, pointsLeft, pointsRight);

	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(F, "F");
	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtVectorToString(pointsLeft, "pointsLeft");
	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtVectorToString(pointsRight, "pointsRight");
	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtVectorToString(matches, "filtered matches");

	// Berechne Disparity Image (linkes und rechtes Bild in einem, ähnlich Differenz)
	cv::Mat disparityImage;
	assert(leftImage.size() == rightImage.size());
	assert(leftImage.channels() == rightImage.channels());
	qDebug() << "-" << "SchwarmElement" << "reconstruct:"
			 << "Channels:" << leftImage.channels() << "type:" << leftImage.type();

	{
		cv::StereoBM sbm;
		cv::Mat leftI, rightI;
		cv::cvtColor(leftImage, leftI, CV_RGB2GRAY);
		cv::cvtColor(rightImage, rightI, CV_RGB2GRAY);
		sbm(leftI, rightI, disparityImage);
	}

	/*
	// Pixelkoordinatensystem verschieben zum Referenz-Punkt
	// positiver Referenz Punkt (z.B. Bildmitte)
	assert(pointsLeft.size() == pointsRight.size());
	for (uint i = 0; i < pointsLeft.size(); i++)
	{
		// example:
		// (0,0) - (50,50) = (-50,-50)
		// (50,60) - (50,50) = (0,10)
		pointsLeft[i] -= refPoint;
		pointsLeft[i] -= refPoint;
	}
	*/

	// E berechnen
	cv::Mat E;
	if (!cam->getEssentialMatrix(F, E))
	{
		qCritical() << "SchwarmElement" << "reconstruct(private):"
					<< "Essential Matrix E konnte nicht berechnet werden!";
		return false;
	}

	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(E, "E");

	// Kamera Matrix
	cv::Mat K;
	if (!cam->getCameraMatrix(K))
	{
		qCritical() << "SchwarmElement" << "reconstruct(private):"
					<< "Kamera Matrix K konnte nicht erhalten werden!";
		return false;
	}

	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(K, "K");


	// Dist.Coeff
	cv::Mat distCoeff;
	if (!cam->getDistCoeff(distCoeff))
	{
		qCritical() << "SchwarmElement" << "reconstruct(private):"
					<< "Distribution Koeffizienten konnte nicht erhalten werden!";
		return false;
	}

	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(distCoeff, "distCoeff");

	// Rotation und Translation aus E berechnen
	cv::Mat R, T;
	if (!calcExtrinsicsFromEssential(E, R, T, K, K, pointsLeft[0], pointsRight[0]))
	{
		qCritical() << "SchwarmElement" << "reconstruct(private):"
					<< "Rotation und Translation konnten nicht berechnet werden!";
		return false;
	}

	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(R, "Loesung R");
	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(T, "Loesung T");
	// Q berechnen
	cv::Mat R1, R2, P1, P2, Q;
	cv::stereoRectify(K, distCoeff, K, distCoeff,
					  leftImage.size(), R, T, R1, R2, P1, P2, Q);

	qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(Q, "Q");
	//qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(R1, "R1");
	//qDebug() << "-" << "SchwarmElement" << "reconstruct:" << CVLibWrapper::cvtMatrixToString(P1, "P1");

	// 2D -> 3D
	cv::Mat Points3D;
	cv::reprojectImageTo3D(disparityImage, Points3D, Q, true);

	qDebug() << "SchwarmElement" << "reconstruct(private):"
			 << "Es wurde ein Bild im Format (" << Points3D.cols << "x" << Points3D.rows << ")"
			 << "mit" << Points3D.channels() << "Channels und"
			 << Points3D.total() << "Elements berechnet.";


	//unsigned int numberOfPoints = 5;
	//CvPoint3D32f points3d[numberOfPoints];
	//CvPOSITObject * object = cvCreatePOSITObject(&points3d, numberOfPoints);
	return true;
}

bool SchwarmElement::calcExtrinsicsFromEssential(
	const cv::Mat & essential, cv::Mat & R, cv::Mat & T,
	const cv::Mat & K_Left, const cv::Mat & K_Right,
	const cv::Point2f & matchedPoint_Left, const cv::Point2f & matchedPoint_Right)
{
	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << "Zerlege E..";

	// Zerlege E, um mögliche Lösungen für R und T zu bestimmen (4 Lösungen)
	// E = U * Sigma * (V^T)
	cv::SVD e_usv(essential, cv::SVD::FULL_UV);
	assert(e_usv.u.rows == e_usv.u.cols && e_usv.u.rows == 3);

	Mat_<double> u = e_usv.u; // 3x3, CV_64FC1
	Mat_<double> vt = e_usv.vt; // 3x3, CV_64FC1

	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << "E zerlegt in U*S*Vt:";
	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << CVLibWrapper::cvtMatrixToString(u, "U");
	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << CVLibWrapper::cvtMatrixToString(vt, "V^T");

	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << "Det(U) =" << cv::determinant(u);
	// Determinanten müssen positiv sein!
	// Kritischer Fall: det() = 0 wird nicht betrachtet,
	// da er nicht vorkommen sollte. (Per assert abgefangen)
	if (cv::determinant(u) < 0)
	{
		for (int i = 0; i < u.rows; i++)
		{
			u(i,2) *= -1;
		}
		qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
				 << "neue Det(U) =" << cv::determinant(u);
	}
	assert(cv::determinant(u) > 0);

	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << "Det(V^T) =" << cv::determinant(vt);
	if (cv::determinant(vt) < 0)
	{
		for (int i = 0; i < vt.rows; i++)
		{
			vt(i,2) *= -1;
		}
		qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
				 << "neue Det(V^T) =" << cv::determinant(vt);
	}
	assert(cv::determinant(vt) > 0);

	// Erzeuge W
	double m[3][3] = {
		{0, -1, 0},
		{1, 0, 0},
		{0, 0, 1}
	};
	cv::Mat W = cv::Mat(3, 3, e_usv.u.type(), m);

	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << CVLibWrapper::cvtMatrixToString(W, "W");

	// Berechne mögliche R-Lösungen
	assert(W.cols == W.rows);
	assert(u.cols == W.rows);
	assert(W.cols == vt.rows);
	cv::Mat R1 = u * W * vt; // (3x3)*(3x3)*(3x3)=(3x3)
	cv::Mat R2 = u * W.t() * vt; // (3x3)*(3x3)*(3x3)=(3x3)

	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << CVLibWrapper::cvtMatrixToString(R1, "R1");
	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << CVLibWrapper::cvtMatrixToString(R2, "R2");

	assert(R1.size() == cv::Size(3,3));
	assert(R2.size() == cv::Size(3,3));

	// Berechne mögliche T-Lösungen
	cv::Mat T1 = u.col(2);
	cv::Mat T2 = T1 * (-1);

	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << CVLibWrapper::cvtMatrixToString(T1, "T1");
	qDebug() << "-" << "SchwarmElement" << "calcExtrinsicsFromEssential:"
			 << CVLibWrapper::cvtMatrixToString(T2, "T2");

	assert(T1.size() == cv::Size(1,3));
	assert(T2.size() == cv::Size(1,3));

	// Geometrische Interpretation der Lösungen:
	// Die richtige Lösung (Ri und Tj) liegt als einzige vor beiden Kameras

	if (isInFrontOfCameras(R = R1, T = T1, matchedPoint_Left, matchedPoint_Right, K_Left, K_Right))
		return true;
	if (isInFrontOfCameras(R = R1, T = T2, matchedPoint_Left, matchedPoint_Right, K_Left, K_Right))
		return true;
	if (isInFrontOfCameras(R = R2, T = T1, matchedPoint_Left, matchedPoint_Right, K_Left, K_Right))
		return true;
	return (isInFrontOfCameras(R = R2, T = T2, matchedPoint_Left, matchedPoint_Right, K_Left, K_Right));
}

double SchwarmElement::calculateDepth(const cv::Mat & R, const cv::Mat & T, const cv::Mat & Point3D)
{
	assert(R.size() == cv::Size(3,3));
	assert(T.size() == cv::Size(1,3));
	assert(Point3D.size() == cv::Size(1,3));
	cv::Mat T_tmp = cv::Mat::zeros(3,1, R.type());
	T_tmp = T;
	// (3x3)*(3x1) + (3x1) = (3x1)
	cv::Mat tmp = (R * Point3D) + T_tmp;
	return tmp.at<double>(2,1);
}

bool SchwarmElement::isInFrontOfCameras(
	const cv::Mat & R, const cv::Mat & T,
	const cv::Point2f & point_Left, const cv::Point2f & point_Right,
	const cv::Mat & K_Left, const cv::Mat & K_Right)
{
	// In diesem Abschnitt werden viele Matrizen erstellt und an bestimmten
	// Stellen mit Werteng efüllt. Es wurde eine Variante gewählt, die
	// nicht die schnellste, aber fehlerfrei funktioniert, sofern die Umformungen
	// "cv::Mat matrix = ...; ...; cv::Mat_<double> m1 = matrix;" funktioniert.
	// Bei anderen Varianten sollte dringend auf den erzeugten Inhalt geachtet
	// werden. Auch sollte darauf geachtet werden, dass cv::Mat m1 = matrix
	// nur eine "Referenz" erzeugt, aber den Inhalt nicht dupliziert!
	// Werte wie 123e-323 oder 234e+654 sind eher untypisch.
	// Temporäre Variablen wurde extra geschachtelt mit {..}, damit
	// der Speicherplatz schnell wieder frei gegeben wird.

	// Projection-Matrix P für die linke Kamera erzeugen
	// P = K * [R|t]; K,R: Matrix, t: Vector
	// initialisiert mit: R=eye-Matrix, t=0-Vektor
	cv::Mat Rt_Left = cv::Mat::eye(3, 4, R.type());
	assert(K_Left.cols == Rt_Left.rows);

	cv::Mat P_Left = K_Left * Rt_Left; // (ix3)*(3,4) = (ix4)
	assert(P_Left.cols == 4);

	qDebug() << "SchwarmElement" << "isInFrontOfCameras:"
			 << "Rt_Left-Matrix sowie P_Left-Matrix erstellt.";

	qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
			 << CVLibWrapper::cvtMatrixToString(Rt_Left, "Rt_Left");
	qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
			 << CVLibWrapper::cvtMatrixToString(P_Left, "P_Left");

	// Projection-Matrix P für die Lösung und Rechte Kamera erzeugen

	qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
			 << CVLibWrapper::cvtMatrixToString(R, "uebergebenes R");
	qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
			 << CVLibWrapper::cvtMatrixToString(T, "uebergebenes T");

	assert(R.size() == cv::Size(3,3)); // Size(width=cols,height=rows)
	assert(T.size() == cv::Size(1,3));

	cv::Mat Rt_Right;
	{
		cv::Mat_<double> R_tmp = R.clone();
		cv::Mat_<double> T_tmp = T.clone();
		double m[3][4] = {
			{R_tmp(0,0), R_tmp(0,1), R_tmp(0,2), T_tmp(0,0)},
			{R_tmp(1,0), R_tmp(1,1), R_tmp(1,2), T_tmp(1,0)},
			{R_tmp(2,0), R_tmp(2,1), R_tmp(2,2), T_tmp(2,0)}
		};
		Rt_Right = cv::Mat(3, 4, R.type(), m); // [R|t]-Vector

		qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
				 << CVLibWrapper::cvtMatrixToString(Rt_Right, "Rt_Right");
		assert(Rt_Right.size() == cv::Size(4,3));
	}

	cv::Mat P_Right = K_Right * Rt_Right; // P = K * [R|t]

	qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
			 << CVLibWrapper::cvtMatrixToString(P_Right, "P_Right");
	assert(P_Right.cols == 4);

	// Design Matrix für Triangulation-DLT erzeugen (aus Punkten und P-Matrizen)
	// TODO: cvTriangulatePoints() besser? -> noch keine OpenCV v2.3-Klassenimplementation
	cv::Mat designMatrix;
	{
		cv::Mat_<double> PL = P_Left.clone();
		cv::Mat_<double> PR = P_Right.clone();
		const double pLx = point_Left.x;
		const double pLy = point_Left.y;
		const double pRx = point_Right.x;
		const double pRy = point_Right.y;
		double m[4][4] = {
			{pLx*PL(2,0)-PL(0,0), pLx*PL(2,1)-PL(0,1), pLx*PL(2,2)-PL(0,2), pLx*PL(2,3)-PL(0,3)},
			{pLy*PL(2,0)-PL(1,0), pLy*PL(2,1)-PL(1,1), pLy*PL(2,2)-PL(1,2), pLy*PL(2,3)-PL(1,3)},
			{pRx*PR(2,0)-PR(0,0), pRx*PR(2,1)-PR(0,1), pRx*PR(2,2)-PR(0,2), pRx*PR(2,3)-PR(0,3)},
			{pRy*PR(2,0)-PR(1,0), pRy*PR(2,1)-PR(1,1), pRy*PR(2,2)-PR(1,2), pRy*PR(2,3)-PR(1,3)}
		};
		designMatrix = cv::Mat(4, 4, R.type(), m);

		qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
				 << CVLibWrapper::cvtMatrixToString(designMatrix, "designMatrix");
		assert(designMatrix.size() == cv::Size(4,4));
	}

	// DLT lösen (mittels SVD): designMatrix * H = 0-Vektor (A*x=0)
	cv::Mat H, X; // H: 4D-Vektor (3D-homogenisch), X: 3D-Vektor (3D-euklidisch)
	cv::SVD::solveZ(designMatrix, H);

	qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
			 << CVLibWrapper::cvtMatrixToString(H, "H");
	assert(H.size() == cv::Size(1,4));
	{
		cv::Mat_<double> H_tmp = H.clone();
		assert(H_tmp(4,1) != 0);
		double m[3][1] = {
			{H_tmp(0,0)/H_tmp(3,0)},
			{H_tmp(1,0)/H_tmp(3,0)},
			{H_tmp(2,0)/H_tmp(3,0)}
		};
		X = cv::Mat(3, 1, R.type(), m);

		qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
				 << CVLibWrapper::cvtMatrixToString(X, "X");

		assert(X.size() == cv::Size(1,3));
	}

	// Raumtiefe berechnen
	double d_Left = calculateDepth(R, T, X);
	double d_Right = calculateDepth(R, T, X);

	qDebug() << "-" << "SchwarmElement" << "isInFrontOfCameras:"
			 << "Tiefe (Links,Rechts):" << d_Left << d_Right;

	// Ist der Punkt vor beiden Kameras?
	if (d_Left > 0 && d_Right > 0) return true; // richtige Lösung
	else return false; // falsche Lösung
}
