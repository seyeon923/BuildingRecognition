#include "detector.hpp"
#include "utility.hpp"

#include <fstream>
#include <map>

using namespace cv;
using namespace std;

int WinDetector::detect(const string& imgFileName, WindowStructure& winStruct, bool showMarker, float thresh, bool useMean) {
	vector<bbox_t> bboxes = this->Detector::detect(imgFileName, thresh, useMean);

	vector<MarkerI> markers;
	for (bbox_t bbox : bboxes) {
		MarkerI marker;
		marker.id = bbox.obj_id;
		marker.prob = bbox.prob;
		marker.location.x = bbox.x + bbox.w / 2;
		marker.location.y = bbox.y + bbox.h / 2;
		//cout << markerNames[marker.id] << "(" << marker.prob << "): " << marker.location.x << ", " << marker.location.y << endl;
		markers.push_back(marker);
	}

	
	string imgFileNameWOExt = imgFileName.substr(0, imgFileName.rfind('.'));
	string imgFileExt = imgFileName.substr(imgFileName.rfind('.') + 1, string::npos);
	
	Mat img = imread(imgFileName);
	if (img.data == NULL) {
		cerr << "img file " << imgFileName << " load fail" << endl;
		showMarker = false;
	}

	if (showMarker) {
		Mat tmp = img.clone();
		drawMarkers(tmp, markers, markerNames);
		imwrite(imgFileNameWOExt + "_marker." + imgFileExt, tmp);
	}

	// remove redundant object
	sort(markers.begin(), markers.end());
	if (markers.begin() != markers.end()) {
		auto first = markers.begin();
		auto result = markers.begin();
		while (++first != markers.end()) {
			if (*first == *result) {
				if (result->prob < first->prob)
					* result = *first;
			}
			else
				*(++result) = *first;
		}
		markers.resize(result - markers.begin() + 1);
	}

	if (showMarker) {
		Mat tmp = img.clone();
		drawMarkers(tmp, markers, markerNames);
		imwrite(imgFileNameWOExt + "_noRedundantMarker." + imgFileExt, tmp);
	}

	// gruop markers of same surface
	map<_Surface*, vector<MarkerI>*> xformableGroup;
	for (MarkerI marker : markers) {
		if (xformableGroup.find(markerIndexToSurfaceAddr[marker.id]) == xformableGroup.end()) {
			vector<MarkerI>* vec = new vector<MarkerI>(1, marker);
			xformableGroup.insert(pair<_Surface*, vector<MarkerI>*>(markerIndexToSurfaceAddr[marker.id], vec));
		}
		else 
			xformableGroup.find(markerIndexToSurfaceAddr[marker.id])->second->push_back(marker);		
	}

	// remove if num of markers of a surface is less than 4	
	vector<_Surface*> willRemoveKeys;
	for (auto group : xformableGroup) {
		if (group.second->size() < 4) {
			delete group.second;
			willRemoveKeys.push_back(group.first);
		}
	}
	for (auto key : willRemoveKeys)
		xformableGroup.erase(key);
	
	for (auto group : xformableGroup) {
		vector<MarkerD> refRelMarkers;
		vector<MarkerI> refAbMarkers;
		readMarkers(buildingInfoDir + "/" + spaceToUnderBar(group.first->name) + ".markers", refRelMarkers);
		for (MarkerD relMarker : refRelMarkers) {
			MarkerI abMarker;
			markerRelToAbsol(relMarker, abMarker, img.size().width, img.size().height);
			refAbMarkers.push_back(abMarker);
		}

		Mat H;
		if (getMarkerMatchHomography(refAbMarkers, *(group.second), H) < 0) {
			cerr << "fail to get Homography of Surface " << group.first->name << endl;
			continue;
		}

		WindowStructure ws;
		readWindows(buildingInfoDir + "/" + spaceToUnderBar(group.first->name) + ".windows", ws, img.size().width, img.size().height);
		ws.perspectiveXform(H);
		winStruct += ws;
	}

	lastDetectedImageSize = img.size();

	return 0;
}


void WinDetector::clearBuildingsInfo() {
	for (_Building* pBuilding : pBuildings) {
		for (_Surface* pSurface : pBuilding->pSurfaces) {
			pSurface->markers.clear();
			delete pSurface;
		}
		pBuilding->pSurfaces.clear();
		delete pBuilding;
	}
	markerIndexToSurfaceAddr.clear();
}

int WinDetector::setBuildingInfoFromFile(const std::string& filename) {
	clearBuildingsInfo();
	pBuildings.clear();
	markerIndexToSurfaceAddr.clear();

	int maxIndex = 0;
	int numSurface;
	ifstream fs(filename);
	if (!fs.is_open()) {
		cerr << "fail to load file " << filename << endl;
		return -1;
	}

	// assign buildings
	for (string line; getline(fs, line);) {
		stringstream ss(line);
		string buildingName;

		ss >> numSurface;
		ss >> ws;
		getline(ss, buildingName);

		pBuildings.push_back(new _Building(buildingName));
		_Building* pLastPutBuilding = pBuildings.back();
		for (int i = 0; i < numSurface; i++) {
			string surfaceName;
			int numIndices;
			getline(fs, surfaceName);
			ss = stringstream(surfaceName);
			ss << surfaceName;
			ss >> numIndices;
			ss >> ws;
			getline(ss, surfaceName);
			pLastPutBuilding->pSurfaces.push_back(new _Surface(surfaceName, pLastPutBuilding));

			string indices;
			getline(fs, indices);
			stringstream istream(indices);
			_Surface* pLastPutSurface = pLastPutBuilding->pSurfaces.back();
			for (int i = 0; i < numIndices; i++) {
				int index;
				istream >> index;
				if (maxIndex < index)
					maxIndex = index;
				pLastPutSurface->markers.push_back(_Marker(index, pLastPutSurface));
			}
		}
	}

	// assign markerIndexToSurfaceAddr
	markerIndexToSurfaceAddr.resize(maxIndex + 1);
	for (_Building* pBuilding : pBuildings)
		for (_Surface* pSurface : pBuilding->pSurfaces)
			for (_Marker marker : pSurface->markers)
				markerIndexToSurfaceAddr[marker.index] = marker.pSurface;

	return 0;
}

int WinDetector::setByDataFile(const std::string& filename) {
	if (parseDataFile(filename) < 0)
		return -1;
	if (setMarkerNamesFormFile(markerNamesFileName) < 0) 
		return -1;
	if (setWindowNamesFromFile(windowNamesFileName) < 0)
		return -1;
	if (setBuildingInfoFromFile(buildingInfoDir + "/building.info") < 0)
		return -1;
	return 0;
}

int WinDetector::parseDataFile(const std::string& filename) {
	ifstream ifs(filename);
	if (!ifs.is_open()) {
		cerr << "fail to load file " << filename << endl;
		return -1;
	}

	vector<string> lines;
	for (string line; getline(ifs, line);)
		lines.push_back(line);
	for (string line : lines) {
		stringstream ss(line);
		string key;
		string equal;
		string value;
		ss >> key >> equal >> value;
		if (equal.compare("=") != 0) {
			cerr << "wrong format of data file " << filename << endl;
			return -1;
		}
		if (key.compare("windowNames") == 0)
			windowNamesFileName = value;
		else if (key.compare("markerNames") == 0)
			markerNamesFileName = value;
		else if (key.compare("buildingInfoDir") == 0) {
			buildingInfoDir = value;
			if (buildingInfoDir.back() == '/')
				buildingInfoDir.resize(buildingInfoDir.size() - 1);
		}
		else {
			cerr << "There is no key " << key << " to be set" << endl;
			return -1;
		}
	}
	return 0;
}

void WinDetector::printBuildings() {
	for (_Building* pBuilding : pBuildings) {
		cout << pBuilding->name << endl;
		for (_Surface* pSurface : pBuilding->pSurfaces) {
			cout << pSurface->name << " ";
			for (_Marker marker : pSurface->markers)
				cout << marker.index << " ";
			cout << endl;
		}
	}
}

int WinDetector::setWindowNamesFromFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		cerr << "fail to load names file " << filename << endl;
		return -1;
	}
	for (std::string line; getline(file, line);) windowNames.push_back(line);
	std::wcout << "window names loaded \n";
	return 0;
}

int WinDetector::setMarkerNamesFormFile(const std::string& filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "fail to load names file " << filename << endl;
		return -1;
	}
	for (string line; getline(file, line);) markerNames.push_back(line);
	return 0;
}

void alignImages(Mat& im1, Mat& im2, Mat& im1Reg, Mat& h, int maxFeatures, float goodMatchPercent) {
	// Convert images to grayscale
	Mat im1Gray, im2Gray;
	cvtColor(im1, im1Gray, CV_BGR2GRAY);
	cvtColor(im2, im2Gray, CV_BGR2GRAY);

	// Variables to store keypoints and descriptors
	std::vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;

	// Detect ORB features and compute descriptors.
	Ptr<Feature2D> orb = ORB::create(maxFeatures);
	orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
	orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);

	// Match features.
	std::vector<DMatch> matches;
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	matcher->match(descriptors1, descriptors2, matches, Mat());

	// Sort matches by score
	std::sort(matches.begin(), matches.end());

	// Remove not so good matches
	const int numGoodMatches = (int)(matches.size() * goodMatchPercent);
	matches.erase(matches.begin() + numGoodMatches, matches.end());


	//// Draw top matches
	//Mat imMatches;
	//drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
	//imwrite("matches.jpg", imMatches);


	// Extract location of good matches
	std::vector<Point2f> points1, points2;

	for (size_t i = 0; i < matches.size(); i++)
	{
		points1.push_back(keypoints1[matches[i].queryIdx].pt);
		points2.push_back(keypoints2[matches[i].trainIdx].pt);
	}

	// Find homography
	h = findHomography(points1, points2, RANSAC);

	// Use homography to warp image
	warpPerspective(im1, im1Reg, h, im2.size());

}

void setWindowStructure(const std::vector<bbox_t>& bboxes, WindowStructure& winStruct) {
	winStruct.ids.clear();
	winStruct.vertices.clear();

	for (bbox_t bbox : bboxes) {
		WindowI window;
		window.id = bbox.obj_id;
		window.vertices[0].x = bbox.x;
		window.vertices[0].y = bbox.y;
		window.vertices[1].x = bbox.x;
		window.vertices[1].y = bbox.y + bbox.h;
		window.vertices[2].x = bbox.x + bbox.w;
		window.vertices[2].y = bbox.y + bbox.h;
		window.vertices[3].x = bbox.x + bbox.w;
		window.vertices[3].y = bbox.y;
		winStruct.pushWindow(window);
	}
}