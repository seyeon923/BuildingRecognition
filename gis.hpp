#ifndef __GIS_HPP
#define __GIS_HPP

#include <vector>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "mysql.hpp"

constexpr double TOPLEFT_LATITUDE = 37.586620;
constexpr double TOPLEFT_LONGITUDE = 127.054646;
constexpr double BOTRIGHT_LATITUDE = 37.579896;
constexpr double BOTRIGHT_LONGITUDE = 127.064674;
constexpr int MAX_ALTITUDE = 150; // in meter
constexpr int MIN_ALTITUDE = 0; // in meter

constexpr double ORIGIN_LATITUDE = (TOPLEFT_LATITUDE + BOTRIGHT_LATITUDE) / 2;
constexpr double ORIGIN_LONGITUDE = (TOPLEFT_LONGITUDE + BOTRIGHT_LONGITUDE) / 2;
constexpr int ORIGIN_ALTITUDE = 0;

constexpr int EARTH_RADIUS = 6371000; // in meter
constexpr double PI = 3.14159265359;

// in meter
constexpr double DIST_PER_LATITUDE = 2 * PI * EARTH_RADIUS / 360;
const double DIST_PER_LONGITUDE = 2 * PI * EARTH_RADIUS * cos(ORIGIN_LATITUDE * PI / 180) / 360;
const double PLANE_WIDTH = (BOTRIGHT_LONGITUDE - TOPLEFT_LONGITUDE) * DIST_PER_LONGITUDE;
constexpr double PLANE_HEIGHT = (TOPLEFT_LATITUDE - BOTRIGHT_LATITUDE) * DIST_PER_LATITUDE;
const double DIST_PER_NORMALIZED_X = PLANE_WIDTH / 2;
constexpr double DIST_PER_NORMALIZED_Y = PLANE_HEIGHT / 2;
constexpr double DIST_PER_NORMALIZED_Z = MAX_ALTITUDE - MIN_ALTITUDE;

const double NORMALIZED_X_PER_LONGITUDE = DIST_PER_LONGITUDE / DIST_PER_NORMALIZED_X;
constexpr double NORMALIZED_Y_PER_LATITUDE = DIST_PER_LATITUDE / DIST_PER_NORMALIZED_Y;
constexpr double NORMALIZED_Z_PER_ALTITUDE = 1 / DIST_PER_NORMALIZED_Z;

constexpr int DEFAULT_PLANE_PLOT_WIDTH = 1280;
constexpr int DEFAULT_PLANE_PLOT_HEIGHT = 720;

template <class T>
class Coord2D {
public:
	T x, y;

	Coord2D(T x, T y) : x(x), y(y) {}
	Coord2D() : x(-1), y(-1) {}

	Coord2D operator+(const Coord2D& other) {
		Coord2D<T> ret(this->x + other.x, this->y + other.y);
		return ret;
	}
	Coord2D operator-(const Coord2D& other) {
		Coord2D<T> temp(this->x - other.x, this->y - other.y);
		return temp;
	}

	Coord2D scale(double ratioX, double ratioY) {
		Coord2D<T> ret((T)(this->x * ratioX), (T)(this->y * ratioY));
		return ret;
	}
	double getDist(const Coord2D& other) {
		return sqrt((this->x - other.x) * (this->x - other.x) +
			(this->y - other.y) * (this->y - other.y));
	}
};

template <class T>
class Coord3D {
public:
	T x, y, z;

	Coord3D(T x, T y, T z) : x(x), y(y), z(z) {}
	Coord3D(Coord2D<T> coord2D, T altitude) : x(coord2D.x), y(coord2D.y), z(altitude){}
	Coord3D() :x(-1), y(-1), z(-1) {}

	Coord3D operator+(const Coord3D& other) {
		Coord3D<T> ret(this->x + other.x, this->y + other.y, this->z + other.z);
		return ret;
	}
	Coord3D operator-(const Coord3D& other) {
		Coord3D<T> ret(this->x - other.x, this->y - other.y, this->z - other.z);
		return ret;
	}

	Coord3D scale(double ratioX, double ratioY, double ratioZ) {
		Coord3D<T> ret(this->x * ratioX, this->y * ratioY, this->z * ratioZ);
		return ret;
	}
	double getDist(const Coord3D& other) {
		return sqrt((this->x - other.x) * (this->x - other.x) + (this->y - other.y) * (this->y - other.y) +
			(this->z - other.z) * (this->z - other.z));
	}
};

template <class T>
class Marker {
public:
	int id;
	float prob;
	cv::Point_<T> location;

	Marker() {}
	Marker(int id, float prob, cv::Point_<T> location) :
		id(id), prob(prob), location(location) {}

	bool operator<(const Marker<T>& other) {
		return this->id < other.id;
	}
	bool operator>(const Marker<T>& other) {
		return this->id > other.id;
	}
	bool operator==(const Marker<T>& other) {
		return this->id == other.id;
	}
	bool operator!=(const Marker<T>& other) {
		return this->id != other.id;
	}
	bool operator<=(const Marker<T>& other) {
		return this->id <= other.id;
	}
	bool operator>=(const Marker<T>& other) {
		return this->id >= other.id;
	}
};

using MarkerI = Marker<int>;
using MarkerD = Marker<double>;


template<class T>
class Window {
public:
	int id;
	cv::Point_<T> vertices[4];

	std::vector<cv::Point_<T>> getVertices() const {
		std::vector<cv::Point_<T>> vec;
		vec.push_back(vertices[0]);
		vec.push_back(vertices[1]);
		vec.push_back(vertices[2]);
		vec.push_back(vertices[3]);
		return vec;
	}
};

using WindowI = Window<int>;
using WindowD = Window<double>;

// get doubled area of window
int getDoubledArea(const WindowI& window);
// get doubled area of polygon, points are vertices of polygon in order
int getDoubledArea(const std::vector<cv::Point2i>& points);
// get doubled intersected area of two polygons
int getDoubledIntersectedArea(const std::vector<cv::Point2i>& vertices1, const std::vector<cv::Point2i>& vertices2);
// get IOU of two polygons
double getIOU(const std::vector<cv::Point2i>& vertices1, const std::vector<cv::Point2i>& vertices2);
// get IOU of two windows
double getIOU(const WindowI& window1, const WindowI& window2);

// find intersected point between two lines, and return if exist
bool findIntersectedPointOfLine(const cv::Point2i p1ofLine1, const cv::Point2i p2ofLine1,
	const cv::Point2i p1ofLine2, const cv::Point2i p2ofLine2, cv::Point2i& intersectedPoint);
// find intersected point between two straight lines, and return if exist
bool findIntersectedPointOfSLine(const cv::Point2i p1ofLine1, const cv::Point2i p2ofLine1,
	const cv::Point2i p1ofLine2, const cv::Point2i p2ofLine2, cv::Point2i& intersectedPoint);

class WindowStructure {
	bool isValidWindow(int index, int width, int height) const;
public:
	vector<int> ids;
	vector<cv::Point2i> vertices;

	WindowStructure(const vector<Window<double>*>& windows, int width, int height) {
		set(windows, width, height);
	}
	WindowStructure(const vector<Window<double>>& windows, int width, int height) {
		set(windows, width, height);
	}
	WindowStructure(const vector<WindowI>& windows) { set(windows); }
	WindowStructure() {}

	WindowStructure& operator+=(WindowStructure& other);

	size_t size() const { return ids.size(); }
	void pushWindow(Window<int> window) {
		ids.push_back(window.id);
		for (int i = 0; i < 4; i++)
			vertices.push_back(window.vertices[i]);
	}
	void set(const vector<Window<double>*>& windows, int width, int height);
	void set(const vector<Window<double>>& windows, int width, int height);
	void set(const vector<Window<int>>& windows);
	void checkVaildWindow(int width, int height, vector<bool>& valids) const;
	void perspectiveXform(cv::Mat& homographyMat);
	void drawWindow(cv::Mat& img, int index, const std::vector<string>& windowNames) const;
	void getWindows(std::vector<WindowI>& windows) const;
};

// get IOU of two vector<WindowI>, groundTruth shall not be include same window(same id)
double getIOU(std::vector<WindowI>& windows, std::vector<WindowI>& groundTruth);
// get IOU of two WindowStructure, groundTruth shall not be include same window(same id)
double getIOU(WindowStructure& winStruct, WindowStructure& groundTruth);

void markerRelToAbsol(const MarkerD& relMarker, MarkerI& absolMarker, int width, int height);
void windowRelToAbsol(const Window<double>& relWindow, Window<int>& absolWindow, int width, int height);

int readMarkers(const std::string& fileName, vector<MarkerD>& markers);
int readWindows(const std::string& fileName, vector<Window<double>*>& windows);
int readWindows(const std::string& fileName, WindowStructure& windowStruct, int width, int height);

int getMarkerMatchHomography(vector<MarkerI>& srcMarkers, vector<MarkerI>& dstMarkers, cv::Mat& h);

void drawWindows(cv::Mat& img, const WindowStructure& winStruct, const std::vector<string>& windowNames);
void drawMarkers(cv::Mat& img, const std::vector<MarkerI>& markers, const std::vector<string>& markerNames);

// GPS coordinate to normalized coordinate (-1 ~ 1)
Coord2D<double> GPStoNormalized2D(double latitude, double longitude);
Coord3D<double> GPStoNormalized3D(double latitude, double longitude, double altitude);

// Normalized coordinate to Image Plane coordinates
cv::Point2d normalizedToImageCoord(double x, double y, int width, int height);

// calculate distance between two points
template <class T>
double getDist(const Coord2D<T>& point1, const Coord2D<T>& point2) {
	return sqrt((point1.x - point2.x) * (point1.x - point2.x) +
		(point1.y - point2.y) * (point1.y - point2.y));
}
template <class T>
double getDist(const Coord3D<T>& p1, const Coord3D<T>& p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) +
		(p1.z - p2.z) * (p1.z - p2.z));
}

// calculate real distance in meter between normalized coordinates
double getRealDist(const Coord2D<double>& p1, const Coord2D<double>& p2);
double getRealDist(const Coord3D<double>& p1, const Coord3D<double>& p2);

void drawPlane(const std::vector<Coord2D<double>>& points, std::string windowName,
	int width = DEFAULT_PLANE_PLOT_WIDTH, int height = DEFAULT_PLANE_PLOT_HEIGHT);
void drawPlane(const std::vector<GIS_DB::Surface*>& surfaces, std::string windowName,
	int width = DEFAULT_PLANE_PLOT_WIDTH, int height = DEFAULT_PLANE_PLOT_HEIGHT);

// check if point is inside of polygon, points are vertices of the polygon in order
bool isInside(const cv::Point2i& point, const std::vector<cv::Point2i>& points);
// check if point is inside of window
bool isInside(const cv::Point2i& point, const WindowI& window);


void gisTest();
void transformTest();

#endif