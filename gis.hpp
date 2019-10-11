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
	std::string name;
	cv::Point_<T> location;

	bool isValid() {
		return location.x >= 0 && location.y >= 0 &&
			!name.empty();
	}
	bool operator<(const Marker& other) {
		return this->name < other.name;
	}
	bool operator>(const Marker& other) {
		return this->name > other.name;
	}
	bool operator==(const Marker& other) {
		return this->name == other.name;
	}
	bool operator!=(const Marker& other) {
		return this->name != other.name;
	}
	bool operator<=(const Marker& other) {
		return this->name <= other.name;
	}
	bool operator>=(const Marker& other) {
		return this->name >= other.name;
	}
};

template<class T>
class Window {
public:
	std::string name;
	cv::Point_<T> vertices[4];

	bool isValid() {
		return vertices[0].x >= 0 && vertices[0].y >= 0 &&
			vertices[1].x >= 0 && vertices[1].y >= 0 &&
			vertices[2].x >= 0 && vertices[2].y >= 0 &&
			vertices[3].x >= 0 && vertices[3].y >= 0 && !name.empty();
	}
};

class WindowStructure {
	bool isValidWindow(int index, int width, int height) const;
public:
	vector<std::string> names;
	vector<cv::Point2i> vertices;

	WindowStructure(const vector<Window<double>*>& windows, int width, int height) {
		set(windows, width, height);
	}
	WindowStructure() {}

	size_t size() const { return names.size(); }
	void pushWindow(Window<int> window) {
		names.push_back(window.name);
		for (int i = 0; i < 4; i++)
			vertices.push_back(window.vertices[i]);
	}
	void set(const vector<Window<double>*>& windows, int width, int height);
	void checkVaildWindow(int width, int height, vector<bool>& valids) const;
	void perspectiveXform(cv::Mat& homographyMat);
	void drawWindow(cv::Mat& img, int index) const;
};

void markerRelToAbsol(const Marker<double>& relMarker, Marker<int>& absolMarker, int width, int height);
void windowRelToAbsol(const Window<double>& relWindow, Window<int>& absolWindow, int width, int height);

int readMarkers(const std::string& fileName, vector<Marker<double>*>& markers);
int readWindows(const std::string& fileName, vector<Window<double>*>& windows);
int readWindows(const std::string& fileName, WindowStructure& windowStruct, int width, int height);

int getMarkerMatchHomography(vector<Marker<int>*>& srcMarkers, vector<Marker<int>*>& dstMarkers, cv::Mat& h);

void drawWindows(cv::Mat& img, const WindowStructure& winStruct);

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

void gisTest();
void transformTest();
#endif