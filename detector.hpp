#ifndef __DETECTOR_HPP
#define __DETECTOR_HPP

#define OPENCV
#include <yolo_v2_class.hpp>

#include "gis.hpp"

class _Marker;
class _Surface;
class _Building;

class _Marker {
public:
	int index;
	_Surface* pSurface;

	_Marker(int index, _Surface* pSurface) :index(index), pSurface(pSurface) {}
};

class _Surface {
public:
	_Surface(std::string name, _Building* pBuilding) :name(name), pBuilding(pBuilding) {}

	std::string name;
	std::vector<_Marker> markers;
	_Building* pBuilding;
};

class _Building {
public:
	_Building(std::string name) :name(name) {}

	std::string name;
	std::vector<_Surface*> pSurfaces;
};

class WinDetector : public Detector{
	bool success;
	std::vector<_Building*> pBuildings;
	std::vector<_Surface*> markerIndexToSurfaceAddr;
	std::string markerNamesFileName;
	std::string windowNamesFileName;
	std::string buildingInfoDir;

public:
	std::vector<std::string> windowNames;
	std::vector<std::string> markerNames;
	cv::Size2i lastDetectedImageSize;

private:
	int setWindowNamesFromFile(const std::string& filename);
	int setMarkerNamesFormFile(const std::string& filename);
	int setBuildingInfoFromFile(const std::string& filename);
	int setByDataFile(const std::string& filename);
	void clearBuildingsInfo();
	int parseDataFile(const std::string& filename);

public:
	WinDetector(const std::string& cfgFileName, const std::string& weightFileName, const std::string& markerNamesFileName,
		const std::string& windowNamesFileName, const std::string& buildingInfoFileName, int gpu_id = 0) :
		Detector(cfgFileName, weightFileName, gpu_id) {
		if (setMarkerNamesFormFile(markerNamesFileName) < 0) {
			success = false;
			this->~WinDetector();
			return;
		}
		if (setWindowNamesFromFile(windowNamesFileName) < 0) {
			success = false;
			this->~WinDetector();
			return;
		}
		if (setBuildingInfoFromFile(buildingInfoFileName) < 0) {
			success = false;
			this->~WinDetector();
			return;
		}
		success = true;
	}
	WinDetector(const std::string& dataFileName, const std::string& cfgFileName, const std::string& weightFileName, int gpuId = 0) :
		Detector(cfgFileName, weightFileName, gpuId) {
		if (setByDataFile(dataFileName) < 0) {
			success = false;
			this->~WinDetector();
			return;
		}
		success = true;
	}
	~WinDetector() { clearBuildingsInfo(); }

	operator bool() { return success; }

	void printBuildings();
	int detect(const std::string& image_filename, WindowStructure& winStruct,
		bool showMarker = false, float thresh = 0.2, bool use_mean = false);
};

void alignImages(cv::Mat& im1, cv::Mat& im2, cv::Mat& im1Reg, cv::Mat& h, int maxFeatures = 500, float goodMatchPercent = 0.15f);

// set WindowStructure with vector<bbox_t>
void setWindowStructure(const std::vector<bbox_t>& bboxes, WindowStructure& winStruct);

#endif