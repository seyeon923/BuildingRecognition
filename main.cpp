#include <vector>
#include <cstdio>
#include <iostream>
#include <filesystem>

#include "mysql.hpp"
#include "detector.hpp"

using namespace std;

void doCmdTest(WinDetector& detector, const char* imgdir = nullptr);
void doCmdIOU(WinDetector& detector, const string& testImgDir);

enum CMD { TEST, IOU, UNKNOWN};

int main(int argc, char* argv[]) {
	if (argc < 5) {
		cout << "Usage: program.exe <cmd> <data file> <YOLO cfg file> <weights file> [test imgs dir]" << endl;
		return 0;
	}
	
	string cmdS(argv[1]);
	int cmd = UNKNOWN;
	if (cmdS.compare("test") == 0)
		cmd = TEST;
	else if (cmdS.compare("iou") == 0) {
		if (argc < 6) {
			cout << "test img directory isn't designated" << endl;
			cout << "Usage: program.exe iou <data file> <YOLO cfg file> <weights file> <test imgs dir>" << endl;
			return 0;
		}
		cmd = IOU;
	}
	else {
		cout << "Unknown command " << cmdS << endl;
		return 0;
	}

	WinDetector detector(argv[2], argv[3], argv[4]);
	if (!detector) {
		cerr << "fail to initialize detector" << endl;
		return -1;
	}
	switch (cmd) {
	case TEST:
		if (argc > 5)
			doCmdTest(detector, argv[5]);
		else
			doCmdTest(detector);
		break;
	case IOU:
		doCmdIOU(detector, argv[5]);
		break;
	default:
		cout << "Unknown command " << cmdS << endl;
		return 0;
	}

	/*vector<cv::Point2i> vertices(4);
	vertices[0].x = 0;  vertices[0].y = 0;
	vertices[1].x = 10;  vertices[1].y = 0;
	vertices[2].x = 10; vertices[2].y = 10;
	vertices[3].x = 0; vertices[3].y = 10;
	cout << "doubled Area: " << getDoubledArea(vertices) << endl;
	if (isInside(cv::Point2i(25, 5), vertices))
		cout << "ture" << endl;
	else
		cout << "false" << endl;

	vector<cv::Point2i> v2;
	v2.push_back(cv::Point2i(5, 5));
	v2.push_back(cv::Point2i(-5, -5));
	v2.push_back(cv::Point2i(15, -5));

	cout << "intersected Area: " << getDoubledIntersectedArea(vertices, v2) << endl;
	cout << "IOU: " << getIOU(vertices, v2) << endl;;

	WindowStructure winStruct1, winStruct2;
	readWindows("ref_CheonnongHallFront1_window_half.txt", winStruct1, 1920, 1080);
	readWindows("ref_CheonnongHallFront1_window.txt", winStruct2, 1920, 1080);
	cout << "IOU: " << getIOU(winStruct1, winStruct2) << endl;*/

	return 0;
}

void doCmdTest(WinDetector& detector, const char* imgDir) {
	if (imgDir == nullptr) {
		string imgFileName;
		while (true) {
			WindowStructure winStruct;
			cout << "enter image file name: ";
			cin >> imgFileName;
			detector.detect(imgFileName, winStruct, true);
			cv::Mat img = cv::imread(imgFileName);
			drawWindows(img, winStruct, detector.windowNames);

			cv::namedWindow(imgFileName, CV_WINDOW_NORMAL);
			cv::imshow(imgFileName, img);
			cv::waitKey();
			cv::destroyWindow(imgFileName);
		}
	}
	else {
		for (const auto& entry : filesystem::directory_iterator(imgDir)) {
			WindowStructure winStruct;
			string imgFileName = entry.path().generic_string();
			if (imgFileName.substr(imgFileName.size() - 3).compare("jpg") == 0) {
				detector.detect(imgFileName, winStruct, true);
				cv::Mat img = cv::imread(imgFileName);
				drawWindows(img, winStruct, detector.windowNames);

				cv::namedWindow(imgFileName, CV_WINDOW_NORMAL);
				cv::imshow(imgFileName, img);
				cv::waitKey();
				cv::destroyWindow(imgFileName);
			}
		}
		return;		
	}
}

void doCmdIOU(WinDetector& detector, const string& testImgDir) {
	double totalIOU = 0;
	int count = 0;
	for (const auto& entry : std::filesystem::directory_iterator(testImgDir)) {
		string fileName = entry.path().generic_string();
		if (fileName.substr(fileName.size() - 3).compare("jpg") == 0) {
			WindowStructure winStruct, refWinSt;
			string fileNameWOExt = fileName.substr(0, fileName.size() - 4);
			detector.detect(fileName, winStruct, false);
			cv::Size2i imgSize = detector.lastDetectedImageSize;
			readWindows(fileNameWOExt + "_quadrangle.txt", refWinSt, imgSize.width, imgSize.height);
			double IOU = getIOU(winStruct, refWinSt);
			cout << "image " << fileName << " IOU: " << IOU << endl;
			totalIOU += IOU;
			count++;
		}
	}
	cout << "average IOU of " << count << " images: " << totalIOU / count << endl;
}