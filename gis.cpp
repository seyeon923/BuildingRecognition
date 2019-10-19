#include "gis.hpp"
#include "utility.hpp"

#include <opencv2/calib3d.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cmath>

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

double getIOU(const std::vector<cv::Point2i>& vertices1, const std::vector<cv::Point2i>& vertices2) {
	int area1 = getDoubledArea(vertices1);
	int area2 = getDoubledArea(vertices2);
	int intersection = getDoubledIntersectedArea(vertices1, vertices2);
	int unionA = area1 + area2 - intersection;
	return (double)intersection / unionA;
}

double getIOU(const WindowI& window1, const WindowI& window2) {
	std::vector<cv::Point2i> vertices1, vertices2;
	vertices1 = window1.getVertices();
	vertices2 = window2.getVertices();
	return getIOU(vertices1, vertices2);
}

double getIOU(std::vector<WindowI>& windows, std::vector<WindowI>& groundTruth) {
	if (windows.size() == 0)
		return 0;
	double totalIOU = 0;
	int addCount = 0;

	auto pred = [](WindowI win1, WindowI win2) { return win1.id < win2.id; };
	sort(windows.begin(), windows.end(), pred);
	sort(groundTruth.begin(), groundTruth.end(), pred);

	// remove redundant window of windows, remain it which has more IOU
	auto result = windows.begin();
	auto first = windows.begin();
	while (++first != windows.end()) {
		if (result->id == first->id) {
			WindowI beCompared;
			for (WindowI window : groundTruth)
				if (window.id == result->id)
					beCompared = window;
			if (getIOU(*result, beCompared) < getIOU(*first, beCompared))
				*result = *first;
		}
		else
			*(++result) = *first;
	}
	windows.resize(result - windows.begin() + 1);

	auto iterWin = windows.begin();
	auto iterGT = groundTruth.begin();
	while (iterWin != windows.end() && iterGT != groundTruth.end()) {
		if (iterWin->id == iterGT->id) {
			totalIOU += getIOU(*iterWin, *iterGT);
			addCount++;
			iterWin++;
			iterGT++;
		}
		else if (iterWin->id < iterGT->id)
			iterWin++;
		else {
			addCount++;
			iterGT++;
		}
	}
	addCount += groundTruth.end() - iterGT;
	addCount += windows.end() - iterWin;
	return addCount == 0 ? 0 : totalIOU / addCount;
}

double getIOU(WindowStructure& winStruct, WindowStructure& groundTruth) {
	std::vector<WindowI> windows1, windows2;
	winStruct.getWindows(windows1);
	groundTruth.getWindows(windows2);
	return getIOU(windows1, windows2);
}

void markerRelToAbsol(const MarkerD& relMarker, MarkerI& absolMarker, int width, int height) {
	absolMarker.id = relMarker.id;
	absolMarker.location.x = (int)(relMarker.location.x * width);
	absolMarker.location.y = (int)(relMarker.location.y * height);
}

void windowRelToAbsol(const Window<double>& relWindow, Window<int>& absolWindow, int width, int height) {
	absolWindow.id = relWindow.id;
	for (int i = 0; i < 4; i++) {
		absolWindow.vertices[i].x = (int)(relWindow.vertices[i].x * width);
		absolWindow.vertices[i].y = (int)(relWindow.vertices[i].y * height);
	}
}

int readMarkers(const std::string& fileName, vector<MarkerD>& markers) {
	ifstream ifs(fileName);
	if (!ifs.is_open())
		return -1;
	for (string line; getline(ifs, line);) {
		stringstream ss(line);
		MarkerD marker;
		ss >> marker.id;
		ss >> marker.location.x;
		ss >> marker.location.y;
		markers.push_back(marker);
	}
	ifs.close();
	return 0;
}

int readWindows(const std::string& fileName, vector<Window<double>*>& windows) {
	ifstream ifs(fileName);
	if (!ifs.is_open())
		return -1;
	for (string line; getline(ifs, line);) {
		stringstream ss(line);
		Window<double>* pWindow = new Window<double>();
		ss >> pWindow->id;
		ss >> pWindow->vertices[0].x;
		ss >> pWindow->vertices[0].y;
		ss >> pWindow->vertices[1].x;
		ss >> pWindow->vertices[1].y;
		ss >> pWindow->vertices[2].x;
		ss >> pWindow->vertices[2].y;
		ss >> pWindow->vertices[3].x;
		ss >> pWindow->vertices[3].y;
		windows.push_back(pWindow);
	}
	return 0;
}

int readWindows(const std::string& fileName, WindowStructure& windowStruct, int width, int height) {
	vector<Window<double>*> windows;
	int ret = readWindows(fileName, windows);
	if (ret < 0)
		return ret;
	windowStruct.set(windows, width, height);
	return 0;
}

void WindowStructure::set(const vector<Window<double>*>& windows, int width, int height) {
	this->ids.clear();
	this->vertices.clear();

	for (Window<double>* pWindow : windows) {
		Window<int> windowI;
		windowI.id = pWindow->id;
		for (int i = 0; i < 4; i++) {
			windowI.vertices[i].x = (int)(pWindow->vertices[i].x * width);
			windowI.vertices[i].y = (int)(pWindow->vertices[i].y * height);
		}
		pushWindow(windowI);
	}
}

void WindowStructure::set(const vector<Window<double>>& windows, int width, int height) {
	this->ids.clear();
	this->vertices.clear();

	for (Window<double> window : windows) {
		Window<int> windowI;
		windowI.id = window.id;
		for (int i = 0; i < 4; i++) {
			windowI.vertices[i].x = (int)(window.vertices[i].x * width);
			windowI.vertices[i].y = (int)(window.vertices[i].y * height);
		}
		pushWindow(windowI);
	}
}

void WindowStructure::set(const vector<Window<int>>& windows) {
	this->ids.clear();
	this->vertices.clear();

	for (WindowI window : windows) {
		ids.push_back(window.id);
		vertices.push_back(window.vertices[0]);
		vertices.push_back(window.vertices[1]);
		vertices.push_back(window.vertices[2]);
		vertices.push_back(window.vertices[3]);
	}
}

bool WindowStructure::isValidWindow(int index, int width, int height) const {
	return vertices[(size_t)index * 4].x >= 0 && vertices[(size_t)index * 4].x < width &&
		vertices[(size_t)index * 4].y >= 0 && vertices[(size_t)index * 4].y < height &&
		vertices[(size_t)index * 4 + 1].x >= 0 && vertices[(size_t)index * 4 + 1].x < width &&
		vertices[(size_t)index * 4 + 1].y >= 0 && vertices[(size_t)index * 4 + 1].y < height &&
		vertices[(size_t)index * 4 + 2].x >= 0 && vertices[(size_t)index * 4 + 2].x < width &&
		vertices[(size_t)index * 4 + 2].y >= 0 && vertices[(size_t)index * 4 + 2].y < height &&
		vertices[(size_t)index * 4 + 3].x >= 0 && vertices[(size_t)index * 4 + 3].x < width &&
		vertices[(size_t)index * 4 + 3].y >= 0 && vertices[(size_t)index * 4 + 3].y < height;
}

void WindowStructure::checkVaildWindow(int width, int height, vector<bool>& valids) const {
	valids.resize(size());
	for (int i = 0; i < this->size(); i++) 
		valids[i] = isValidWindow(i, width, height);
}

void WindowStructure::perspectiveXform(Mat& homographyMat) {
	vector<Point2f> xformedPoints(this->size() * 4);
	vector<Point2f> originPoints(this->size() * 4);

	for (int i = 0; i < this->size(); i++) {
		originPoints[(size_t)i * 4].x = (float)this->vertices[(size_t)i * 4].x;
		originPoints[(size_t)i * 4].y = (float)this->vertices[(size_t)i * 4].y;
		originPoints[(size_t)i * 4 + 1].x = (float)this->vertices[(size_t)i * 4 + 1].x;
		originPoints[(size_t)i * 4 + 1].y = (float)this->vertices[(size_t)i * 4 + 1].y;
		originPoints[(size_t)i * 4 + 2].x = (float)this->vertices[(size_t)i * 4 + 2].x;
		originPoints[(size_t)i * 4 + 2].y = (float)this->vertices[(size_t)i * 4 + 2].y;
		originPoints[(size_t)i * 4 + 3].x = (float)this->vertices[(size_t)i * 4 + 3].x;
		originPoints[(size_t)i * 4 + 3].y = (float)this->vertices[(size_t)i * 4 + 3].y;
	}
	perspectiveTransform(originPoints, xformedPoints, homographyMat);
	for (int i = 0; i < this->size(); i++) {
		this->vertices[(size_t)i * 4].x = (int)xformedPoints[(size_t)i * 4].x;
		this->vertices[(size_t)i * 4].y = (int)xformedPoints[(size_t)i * 4].y;
		this->vertices[(size_t)i * 4 + 1].x = (int)xformedPoints[(size_t)i * 4 + 1].x;
		this->vertices[(size_t)i * 4 + 1].y = (int)xformedPoints[(size_t)i * 4 + 1].y;
		this->vertices[(size_t)i * 4 + 2].x = (int)xformedPoints[(size_t)i * 4 + 2].x;
		this->vertices[(size_t)i * 4 + 2].y = (int)xformedPoints[(size_t)i * 4 + 2].y;
		this->vertices[(size_t)i * 4 + 3].x = (int)xformedPoints[(size_t)i * 4 + 3].x;
		this->vertices[(size_t)i * 4 + 3].y = (int)xformedPoints[(size_t)i * 4 + 3].y;
	}
}

int getMarkerMatchHomography(vector<MarkerI>& srcMarkers, vector<MarkerI>& dstMarkers, Mat& h) {
	h = Mat(Size(3, 3), CV_64FC1);
	sort(srcMarkers.begin(), srcMarkers.end());
	sort(dstMarkers.begin(), dstMarkers.end());

	vector<Point2f> srcPoints, dstPoints;
	for (vector<MarkerI>::const_iterator pSrcMarkerIter = srcMarkers.begin(), pDstMarkerIter = dstMarkers.begin();
		pSrcMarkerIter != srcMarkers.end() && pDstMarkerIter != dstMarkers.end(); ) {
		MarkerI srcMarker = *pSrcMarkerIter;
		MarkerI dstMarker = *pDstMarkerIter;
		if (srcMarker == dstMarker) {
			srcPoints.push_back(srcMarker.location);
			dstPoints.push_back(dstMarker.location);
			pSrcMarkerIter++;
			pDstMarkerIter++;
		}
		else if (srcMarker < dstMarker)
			pSrcMarkerIter++;
		else
			pDstMarkerIter++;
	}

	if (srcPoints.size() < 4) {
		wcerr << "Matched Marker have to be more than or equal to 4" << endl;
		return -1;
	}

	/*cout << "src points" << endl;
	for (auto point : srcPoints)
		cout << point.x << ", " << point.y << endl;
	cout << endl << "dst points" << endl;
	for (auto point : dstPoints)
		cout << point.x << ", " << point.y << endl;*/
	h = findHomography(srcPoints, dstPoints, RANSAC);
	return 0;
}

void drawWindows(cv::Mat& img, const WindowStructure& winStruct, const vector<string>& windowNames) {
	vector<bool> valids;
	winStruct.checkVaildWindow(img.size().width, img.size().height, valids);
	for (int i = 0; i < winStruct.size(); i++)
		if (valids[i])
			winStruct.drawWindow(img, i, windowNames);
}

void WindowStructure::drawWindow(cv::Mat& img, int index, const vector<string>& windowNames) const {
	putText(img, windowNames[this->ids[(size_t)index]], vertices[(size_t)index * 4], FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255));
	line(img, vertices[(size_t)index * 4], vertices[(size_t)index * 4 + 1], Scalar(0, 0, 255), 2);
	line(img, vertices[(size_t)index * 4 + 1], vertices[(size_t)index * 4 + 2], Scalar(0, 0, 255), 2);
	line(img, vertices[(size_t)index * 4 + 2], vertices[(size_t)index * 4 + 3], Scalar(0, 0, 255), 2);
	line(img, vertices[(size_t)index * 4 + 3], vertices[(size_t)index * 4], Scalar(0, 0, 255), 2);
}

void WindowStructure::getWindows(std::vector<WindowI>& windows) const {
	for (int i = 0; i < this->size(); i++) {
		WindowI window;
		window.id = ids[i];
		window.vertices[0] = vertices[(size_t)4 * i];
		window.vertices[1] = vertices[(size_t)4 * i + 1];
		window.vertices[2] = vertices[(size_t)4 * i + 2];
		window.vertices[3] = vertices[(size_t)4 * i + 3];
		windows.push_back(window);
	}
}

int getDoubledArea(const WindowI& window) {
	vector<Point2i> vertices(window.vertices, window.vertices + 4);
	return getDoubledArea(vertices);
}

int getDoubledArea(const std::vector<cv::Point2i>& points) {
	if (points.size() < 3)
		return 0;
	int positive, negative;
	positive = negative = 0;
	for (int i = 0; i < points.size()- 1; i++) {
		positive += points[i].x * points[i + 1].y;
		negative -= points[i].y * points[i + 1].x;
	}
	positive += points[points.size() - 1].x * points[0].y;
	negative -= points[points.size() - 1].y * points[0].x;
	int dArea = positive + negative;
	return dArea > 0 ? dArea : -dArea;
}

int getDoubledIntersectedArea(const std::vector<cv::Point2i>& polygon1, const std::vector<cv::Point2i>& polygon2) {
	if (polygon1.size() < 3 || polygon2.size() < 3) // if both polygons aren't ploygon
		return 0;

	vector<Point2i> vertices; // vertices of intersected polygon

	// find polygon1's vertices inside polygon2
	for (Point2i point : polygon1)
		if (isInside(point, polygon2))
			vertices.push_back(point);

	// find polygon2's vertices inside polygon1
	for (Point2i point : polygon2)
		if (isInside(point, polygon1))
			vertices.push_back(point);

	// find intersected points between two polygons
	for (int i = 0; i < polygon1.size(); i++) {
		for (int j = 0; j < polygon2.size(); j++) {
			Point2i p1Poly1, p2Poly1, p1Poly2, p2Poly2, interPoint;
			p1Poly1 = polygon1[i];
			p1Poly2 = polygon2[j];
			if (i == polygon1.size() - 1)
				p2Poly1 = polygon1[0];
			else
				p2Poly1 = polygon1[i + 1];
			if (j == polygon2.size() - 1)
				p2Poly2 = polygon2[0];
			else
				p2Poly2 = polygon2[j + 1];
			if (findIntersectedPointOfLine(p1Poly1, p2Poly1, p1Poly2, p2Poly2, interPoint))
				vertices.push_back(interPoint);
		}
	}

	// remove same vertices
	sort(vertices.begin(), vertices.end(), [](Point2i p1, Point2i p2) {
		if (p1.x == p2.x)
			return p1.y < p2.y;
		else
			return p1.x < p2.x;
	});
	vector<Point2i>::iterator end = unique(vertices.begin(), vertices.end(), [](Point2i p1, Point2i p2) {
		return p1.x == p2.x && p1.y == p2.y;
	});
	vertices.resize(end - vertices.begin());

	if (vertices.size() < 3) // there's no intersected space
		return 0;

	// get center point of vertices
	int cX, cY;
	cX = cY = 0;
	for (Point2i point : vertices) {
		cX += point.x;
		cY += point.y;
	}
	cX = cX / vertices.size();
	cY = cY / vertices.size();

	// sort in order of angles at polar coordinates which center is (cX, cY)
	sort(vertices.begin(), vertices.end(), [cX, cY](Point2i p1, Point2i p2) {
		p1.x -= cX;
		p1.y -= cY;
		p2.x -= cX;
		p2.y -= cY;
		return atan2((double)p1.y, (double)p1.x) < atan2((double)p2.y, (double)p2.x);
	});

	return getDoubledArea(vertices);
}

bool findIntersectedPointOfLine(const cv::Point2i p1ofLine1, const cv::Point2i p2ofLine1,
	const cv::Point2i p1ofLine2, const cv::Point2i p2ofLine2, cv::Point2i& intersectedPoint) {
	// find x coordinate bound
	int leftXofLine1 = p1ofLine1.x > p2ofLine1.x ? p2ofLine1.x : p1ofLine1.x;
	int rightXofLine1 = p1ofLine1.x > p2ofLine1.x ? p1ofLine1.x : p2ofLine1.x;
	int leftXofLine2 = p1ofLine2.x > p2ofLine2.x ? p2ofLine2.x : p1ofLine2.x;
	int rightXofLine2 = p1ofLine2.x > p2ofLine2.x ? p1ofLine2.x : p2ofLine2.x;
	int minX = leftXofLine1 > leftXofLine2 ? leftXofLine1 : leftXofLine2;
	int maxX = rightXofLine1 > rightXofLine2 ? rightXofLine2 : rightXofLine1;

	//find y coordinate bound
	int minYofLine1 = p1ofLine1.y > p2ofLine1.y ? p2ofLine1.y : p1ofLine1.y;
	int minYofLine2 = p1ofLine2.y > p2ofLine2.y ? p2ofLine2.y : p1ofLine2.y;
	int maxYofLine1 = p1ofLine1.y > p2ofLine1.y ? p1ofLine1.y : p2ofLine1.y;
	int maxYofLine2 = p1ofLine2.y > p2ofLine2.y ? p1ofLine2.y : p2ofLine2.y;
	int minY = minYofLine1 > minYofLine2 ? minYofLine1 : minYofLine2;
	int maxY = maxYofLine1 > maxYofLine2 ? maxYofLine2 : maxYofLine1;

	Point2i interPoint;
	if (findIntersectedPointOfSLine(p1ofLine1, p2ofLine1, p1ofLine2, p2ofLine2, interPoint)) {
		if (minX <= interPoint.x && interPoint.x <= maxX &&
			minY <= interPoint.y && interPoint.y <= maxY) {
			intersectedPoint = interPoint;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool findIntersectedPointOfSLine(const cv::Point2i p1ofLine1, const cv::Point2i p2ofLine1,
	const cv::Point2i p1ofLine2, const cv::Point2i p2ofLine2, cv::Point2i& intersectedPoint) {
	if (p2ofLine1.x - p1ofLine1.x == 0) { // if line1's inclination is infinity
		if (p2ofLine2.x - p1ofLine2.x == 0) // both line's inclinations are infinity
			return false;

		// only line1's inclination is infinity
		double m2 = (double)(p2ofLine2.y - p1ofLine2.y) / (p2ofLine2.x - p1ofLine2.x);
		int y = (int)(m2 * (p1ofLine1.x - p1ofLine2.x) + p1ofLine2.y);
		intersectedPoint.x = p1ofLine1.x;
		intersectedPoint.y = y;
		return true;
	}
	else {
		double m1 = (double)(p2ofLine1.y - p1ofLine1.y) / (p2ofLine1.x - p1ofLine1.x);
		if (p2ofLine2.x - p1ofLine2.x == 0) { // if only line2's inclination is infinity
			intersectedPoint.x = p1ofLine2.x;
			intersectedPoint.y = (int)(m1 * (intersectedPoint.x - p1ofLine1.x) + p1ofLine1.y);
			return true;
		}

		// both lines' inclinations aren't infinity
		double m2 = (double)(p2ofLine2.y - p1ofLine2.y) / (p2ofLine2.x - p1ofLine2.x);

		double absM1 = m1 > 0 ? m1 : -m1;
		double absM2 = m2 > 0 ? m2 : -m2;
		double diff = absM1 - absM2;
		if (-0.01 < diff && diff < 0.01) // same inclination
			return false;
		Mat mat(2, 2, CV_64FC1);
		mat.at<double>(0, 0) = m1;
		mat.at<double>(0, 1) = -1;
		mat.at<double>(1, 0) = m2;
		mat.at<double>(1, 1) = -1;
		Mat inv = mat.inv();
		intersectedPoint.x = (int)(inv.at<double>(0, 0) * (m1 * p1ofLine1.x - p1ofLine1.y)
			+ inv.at<double>(0, 1) * (m2 * p1ofLine2.x - p1ofLine2.y));
		intersectedPoint.y = (int)(inv.at<double>(1, 0) * (m1 * p1ofLine1.x - p1ofLine1.y)
			+ inv.at<double>(1, 1) * (m2 * p1ofLine2.x - p1ofLine2.y));
		return true;
	}
}

bool isInside(const cv::Point2i& point, const std::vector<cv::Point2i>& points) {
	if (points.size() < 3)
		return false;
	int polygonDArea = getDoubledArea(points);
	int compArea = 0;
	for (int i = 0; i < points.size() - 1; i++) {
		vector<Point2i> tmp;
		tmp.push_back(point);
		tmp.push_back(points[i]);
		tmp.push_back(points[i + 1]);
		compArea += getDoubledArea(tmp);
	}
	vector<Point2i> tmp;
	tmp.push_back(point);
	tmp.push_back(points[points.size() - 1]);
	tmp.push_back(points[0]);
	compArea += getDoubledArea(tmp);
	return compArea == polygonDArea;
}

bool isInside(const cv::Point2i& point, const WindowI& window) {
	return isInside(point, window.getVertices());
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

	GIS_DB db("172.16.162.143", 33060, "xtwicett", "rlatpdus", "campus_gis");
	vector<GIS_DB::Surface*> surfaces;
	db.selectFromSurface(surfaces, "");
	drawPlane(surfaces, "Buildings Outline");
	clearPointerVec(surfaces);
}

void drawMarkers(cv::Mat& img, const std::vector<MarkerI>& markers, const vector<string>& markerNames) {
	for (MarkerI marker : markers) {
		putText(img, markerNames[marker.id], marker.location, FONT_HERSHEY_COMPLEX, 1, objIdToColor(marker.id));
		circle(img, marker.location, 2, objIdToColor(marker.id), FILLED);
	}
}

WindowStructure& WindowStructure::operator+=(WindowStructure& other) {
	for (int i = 0; i < other.ids.size(); i++) {
		this->ids.push_back(other.ids[i]);
		for (int j = 0; j < 4; j++)
			this->vertices.push_back(other.vertices[i * 4 + j]);
	}
	return *this;
}

void transformTest() {
	/*string refWindowFileName = "ref_CheonnongHallFront1_window.txt";
	string refMarkerFileName = "ref_CheonnongHallFront1_marker.txt";
	string testImgFileName = "CheonnongHallPos110004.jpg";
	string testMarkerFileName = "CheonnongHallPos110004_marker.txt";

	Mat img = imread(testImgFileName);
	int width = img.size().width;
	int height = img.size().height;

	vector<MarkerD*> refRelMarkers, testRelMarkers;
	vector<MarkerI*> refAbMarkers, testAbMarkers;
	
	if (readMarkers(refMarkerFileName, refRelMarkers) < 0) {
		wcout << "Read markers from file " << refMarkerFileName.c_str() << " failed!" << endl;
		return;
	}
	if (readMarkers(testMarkerFileName, testRelMarkers) < 0) {
		wcout << "Read markers from file " << testMarkerFileName.c_str() << " failed!" << endl;
		return;
	}

	for (MarkerD* pRelMarker : refRelMarkers) {
		MarkerI* pAbMarker = new MarkerI();
		markerRelToAbsol(*pRelMarker, *pAbMarker, width, height);
		refAbMarkers.push_back(pAbMarker);
	}
	clearPointerVec(refRelMarkers);

	for (MarkerD* pRelMarker : testRelMarkers) {
		MarkerI* pAbMarker = new MarkerI();
		markerRelToAbsol(*pRelMarker, *pAbMarker, width, height);
		testAbMarkers.push_back(pAbMarker);
	}
	clearPointerVec(testRelMarkers);

	Mat H;
	getMarkerMatchHomography(refAbMarkers, testAbMarkers, H);

	WindowStructure winStruct;
	readWindows(refWindowFileName, winStruct, width, height);*/
	
	/*winStruct.perspectiveXform(H);
	drawWindows(img, winStruct);

	namedWindow(testImgFileName, WINDOW_NORMAL);
	imshow(testImgFileName, img);
	waitKey();*/
}