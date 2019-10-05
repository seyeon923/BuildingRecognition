#include "gis.hpp"
#include <iostream>

using namespace std;
using namespace cv;


double getRealDist(const Coord2D<double>& p1, const Coord2D<double>& p2) {
	double distX = (p1.x - p2.x) * DIST_PER_NORMALIZED_X;
	double distY = (p1.y - p2.y) * DIST_PER_NORMALIZED_Y;
	return sqrt(distX * distX + distY * distY);
}

double getRealDist(const Coord3D<double>& p1, const Coord3D<double>& p2) {
	double distX = (p1.x - p2.x) * DIST_PER_NORMALIZED_X;
	double distY = (p1.y - p2.y) * DIST_PER_NORMALIZED_Y;
	double distZ = (p1.z - p2.z) * DIST_PER_NORMALIZED_Z;
	return sqrt(distX * distX + distY * distY + distZ * distZ);
}

Coord2D<double> GPStoNormalized2D(double latitude, double longitude) {
	Coord2D<double> ret(longitude - ORIGIN_LONGITUDE, latitude - ORIGIN_LATITUDE);
	ret = ret.scale(NORMALIZED_X_PER_LONGITUDE, NORMALIZED_Y_PER_LATITUDE);
	return ret;
}

Coord3D<double> GPStoNormalized3D(double latitude, double longitude, double altitude) {
	Coord3D<double> ret(longitude - ORIGIN_LONGITUDE, latitude - ORIGIN_LATITUDE,
		altitude - ORIGIN_ALTITUDE);
	ret = ret.scale(NORMALIZED_X_PER_LONGITUDE, NORMALIZED_Y_PER_LATITUDE,
		NORMALIZED_Z_PER_ALTITUDE);
	return ret;
}

void drawPlane(const vector<Coord2D<double>>& points, string windowName, int width, int height) {
	Mat plane = Mat::zeros(height, width, CV_8UC1);
	for (auto point : points) {
		Point2d p = normalizedToImageCoord(point.x, point.y, width, height);
		circle(plane, p, 2, Scalar(255), FILLED);
	}
	
	namedWindow(windowName, WINDOW_AUTOSIZE);
	imshow(windowName, plane);
	waitKey(0);
}

void drawPlane(const std::vector<GIS_DB::Surface*>& surfaces, std::string windowName,
	int width, int height) {
	Mat plane = Mat::zeros(height, width, CV_8UC1);
	Coord2D<double> normalizedP1, normalizedP2;
	Point2i imageP1, imageP2;
	for (GIS_DB::Surface* pSurface : surfaces) {
		normalizedP1 = GPStoNormalized2D(pSurface->botLeftLatitude, pSurface->botLeftLongitude);
		normalizedP2 = GPStoNormalized2D(pSurface->botRightLatitude, pSurface->botRightLongitude);
		imageP1 = normalizedToImageCoord(normalizedP1.x, normalizedP1.y, width, height);
		imageP2 = normalizedToImageCoord(normalizedP2.x, normalizedP2.y, width, height);
		line(plane, imageP1, imageP2, Scalar(255));
	}

	namedWindow(windowName, WINDOW_AUTOSIZE);
	imshow(windowName, plane);
	waitKey(0);
}

cv::Point2d normalizedToImageCoord(double x, double y, int width, int height) {
	Point2d p;
	p.x = (int)((x + 1) / 2 * width);
	p.y = (int)((-y + 1) / 2 * height);
	return p;
}

void gisTest() {
	wcout << fixed;
	wcout.precision(6);
	wcout << "ORIGIN_LATITUDE: " << ORIGIN_LATITUDE << endl;
	wcout << "ORIGIN_LONGITUDE: " << ORIGIN_LONGITUDE << endl;
	wcout << "DIST_PER_LATITUDE: " << DIST_PER_LATITUDE << endl;
	wcout << "DIST_PER_LONGITUDE: " << DIST_PER_LONGITUDE << endl;
	wcout << "PLANE_WIDTH: " << PLANE_WIDTH << endl;
	wcout << "PLANE_HEIGHT: " << PLANE_HEIGHT << endl;
	wcout << "DIST_PER_NORMALIZED_X: " << DIST_PER_NORMALIZED_X << endl;
	wcout << "DIST_PER_NORMALZIED_Y: " << DIST_PER_NORMALIZED_Y << endl;
	wcout << "DIST_PER_NORMALIZED_Z: " << DIST_PER_NORMALIZED_Z << endl;
	wcout << "NORMALZIED_X_PER_LONGITUDE: " << NORMALIZED_X_PER_LONGITUDE << endl;
	wcout << "NORMALZIED_Y_PER_LATITUDE: " << NORMALIZED_Y_PER_LATITUDE << endl;
	wcout << "NORMALIZED_Z_PER_ALTITUDE: " << NORMALIZED_Z_PER_ALTITUDE << endl << endl;

	const double circleCrossLatitude = 37.583461;
	const double circleCrossLongitude = 127.055194;
	const double circleCrossAltitude = 50;
	const double threeWayIntersection2LibraryLatitude = 37.584284;
	const double threeWayIntersection2LibraryLongitude = 127.061318;
	const double threeWayIntersection2LibraryAltitude = 100;

	Coord2D<double> circleCross = GPStoNormalized2D(circleCrossLatitude, circleCrossLongitude);
	Coord2D<double> threeWayIntersection2Library =
		GPStoNormalized2D(threeWayIntersection2LibraryLatitude, threeWayIntersection2LibraryLongitude);

	wcout << L"원형교차로 GPS: (" << circleCrossLatitude << ", " << circleCrossLongitude << ")" << endl;
	wcout << L"도서관삼거리: (" << threeWayIntersection2LibraryLatitude << ",  " << threeWayIntersection2LibraryLongitude << ")" << endl;
	wcout << L"원형교차로 Normalized: (" << circleCross.x << ", " << circleCross.y << ")" << endl;
	wcout << L"도서관삼거리 Normalized: (" <<
		threeWayIntersection2Library.x << ", " << threeWayIntersection2Library.y << ")" << endl;
	Coord2D<double> tmp = circleCross + threeWayIntersection2Library;
	wcout << "operator+: (" << tmp.x << ", " << tmp.y << ")\n";
	tmp = circleCross - threeWayIntersection2Library;
	wcout << "operator-: (" << tmp.x << ", " << tmp.y << ")" << endl;;
	wcout << L"원형교차로 - 도서관 앞 삼거리(getRealDist): " << getRealDist(circleCross, threeWayIntersection2Library) << endl;
	circleCross.x += 1;
	circleCross.y += 1;
	threeWayIntersection2Library.x++;
	threeWayIntersection2Library.y++;
	circleCross = circleCross.scale(DIST_PER_NORMALIZED_X, DIST_PER_NORMALIZED_Y);
	threeWayIntersection2Library = threeWayIntersection2Library.scale(DIST_PER_NORMALIZED_X, DIST_PER_NORMALIZED_Y);
	wcout << L"원형교차로 Real: (" << circleCross.x << ", " << circleCross.y << ")\n";
	wcout << L"도서관삼거리 Real: (" << threeWayIntersection2Library.x <<
		", " << threeWayIntersection2Library.y << ")\n";
	wcout << L"원형교차로 - 도서관삼거리(Coord2D::getDist): " << circleCross.getDist(threeWayIntersection2Library) << endl;
	wcout << L"원형교차로 - 도서관삼거리(getDist): " << getDist<double>(circleCross, threeWayIntersection2Library) << endl << endl;

	Coord3D<double> A = GPStoNormalized3D(circleCrossLatitude, circleCrossLongitude, circleCrossAltitude);
	Coord3D<double> B = GPStoNormalized3D(threeWayIntersection2LibraryLatitude, threeWayIntersection2LibraryLongitude,
		threeWayIntersection2LibraryAltitude);
	wcout << "A GPS: (" << circleCrossLatitude << ", " << circleCrossLongitude << ", " << circleCrossAltitude << ")\n";
	wcout << "B GPS: (" << threeWayIntersection2LibraryLatitude << ", " << threeWayIntersection2LibraryLongitude << ", " <<
		threeWayIntersection2LibraryAltitude << ")\n";
	wcout << "A Normalized: (" << A.x << ", " << A.y << ", " << A.z << ")\n";
	wcout << "B Normalized: (" << B.x << ", " << B.y << ", " << B.z << ")\n";
	Coord3D<double> tmp3D = A + B;
	wcout << "+: (" << tmp3D.x << ", " << tmp3D.y << ", " << tmp3D.z << ")\n";
	tmp3D = A - B;
	wcout << "-: (" << tmp3D.x << ", " << tmp3D.y << ", " << tmp3D.z << ")\n";
	wcout << "A - B getRealDist: " << getRealDist(A, B) << endl;
	A.x++, A.y++, B.x++, B.y++;
	A = A.scale(DIST_PER_NORMALIZED_X, DIST_PER_NORMALIZED_Y, DIST_PER_NORMALIZED_Z);
	B = B.scale(DIST_PER_NORMALIZED_X, DIST_PER_NORMALIZED_Y, DIST_PER_NORMALIZED_Z);
	wcout << "A Real: (" << A.x << ", " << A.y << ", " << A.z << ")\n";
	wcout << "B Real: (" << B.x << ", " << B.y << ", " << B.z << ")\n";
	wcout << "A - B Coord3D::getDist: " << A.getDist(B) << endl;
	wcout << "A - B getDist: " << getDist(A, B) << endl;;
	wcout.unsetf(ios::fixed);

	vector<Coord2D<double>> points;
	// 전농관
	points.push_back(GPStoNormalized2D(37.583595, 127.056211));
	points.push_back(GPStoNormalized2D(37.583304, 127.056211));
	points.push_back(GPStoNormalized2D(37.583310, 127.056380));
	points.push_back(GPStoNormalized2D(37.583531, 127.056399));
	points.push_back(GPStoNormalized2D(37.583546, 127.056761));
	points.push_back(GPStoNormalized2D(37.583454, 127.056884));
	points.push_back(GPStoNormalized2D(37.583690, 127.056871));
	points.push_back(GPStoNormalized2D(37.583688, 127.056769));
	points.push_back(GPStoNormalized2D(37.583654, 127.056764));
	points.push_back(GPStoNormalized2D(37.583645, 127.056397));

	// 음악관
	points.push_back(GPStoNormalized2D(37.583813, 127.055485));
	points.push_back(GPStoNormalized2D(37.583836, 127.055906));
	points.push_back(GPStoNormalized2D(37.584055, 127.055882));
	points.push_back(GPStoNormalized2D(37.584032, 127.055466));

	// 배봉관
	points.push_back(GPStoNormalized2D(37.584769, 127.059329));
	points.push_back(GPStoNormalized2D(37.584539, 127.059350));
	points.push_back(GPStoNormalized2D(37.584594, 127.059945));
	points.push_back(GPStoNormalized2D(37.584807, 127.059940));

	points.push_back(GPStoNormalized2D(37.584666, 127.060235));
	points.push_back(GPStoNormalized2D(37.584777, 127.061640));
	drawPlane(points, "Test");

	GIS_DB db("172.16.162.143", 33060, "xtwicett", "rlatpdus", "campus_gis");
	vector<GIS_DB::Surface*> surfaces;
	db.selectFromSurface(surfaces, "");
	drawPlane(surfaces, "Buildings Outline");
	clearPointerVec(surfaces);
}