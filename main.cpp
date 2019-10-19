#include <vector>
#include <cstdio>
#include <iostream>
#include <filesystem>
#include <cmath>

#include "mysql.hpp"
#include "detector.hpp"

using namespace std;

void doCmdTest(WinDetector& detector, const char* imgdir = nullptr);
void doCmdIOU(WinDetector& detector, const string& testImgDir);
void doCmdIOUyolo(WinDetector& detector, const string& testImgDir);
void doCmdTestYolo(WinDetector& detector, const char* imgDir = nullptr);

enum CMD { TEST, IOU, TEST_YOLO, IOU_YOLO, UNKNOWN};

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
	else if (cmdS.compare("testyolo") == 0)
		cmd = TEST_YOLO;
	else if (cmdS.compare("iouyolo") == 0) {
		if (argc < 6) {
			cout << "test img directory isn't designated" << endl;
			cout << "Usage: program.exe iouyolo <data file> <YOLO cfg file> <weights file> <test imgs dir>" << endl;
			return 0;
		}
		cmd = IOU_YOLO;
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
	case TEST_YOLO:
		if (argc > 5)
			doCmdTestYolo(detector, argv[5]);
		else doCmdTestYolo(detector);
		break;
	case IOU_YOLO:
		doCmdIOUyolo(detector, argv[5]);
		break;
	default:
		cout << "Unknown command " << cmdS << endl;
		return 0;
	}

	return 0;
}

void doCmdTest(WinDetector& detector, const char* imgDir) {
	if (imgDir == nullptr) {
		string imgFileName;
		while (true) {
			WindowStructure winStruct, refStructure;
			cout << "enter image file name: ";
			cin >> imgFileName;
			string fileNameWOExt = imgFileName.substr(0, imgFileName.size() - 4);
			detector.detect(imgFileName, winStruct, true);
			cv::Mat img = cv::imread(imgFileName);
			drawWindows(img, winStruct, detector.windowNames);
			readWindows(fileNameWOExt + "_quadrangle.txt", refStructure, img.size().width, img.size().height);
			double IoU = getIOU(winStruct, refStructure);
			cout << "IoU: " << IoU << endl;

			cv::namedWindow(imgFileName, CV_WINDOW_NORMAL);
			cv::imshow(imgFileName, img);
			cv::waitKey();
			cv::destroyWindow(imgFileName);
		}
	}
	else {
		for (const auto& entry : filesystem::directory_iterator(imgDir)) {
			WindowStructure winStruct, refStructure;
			string imgFileName = entry.path().generic_string();
			string fileNameWOExt = imgFileName.substr(0, imgFileName.size() - 4);
			if (imgFileName.substr(imgFileName.size() - 3).compare("jpg") == 0) {
				detector.detect(imgFileName, winStruct, true);
				cv::Mat img = cv::imread(imgFileName);
				drawWindows(img, winStruct, detector.windowNames);
				readWindows(fileNameWOExt + "_quadrangle.txt", refStructure, img.size().width, img.size().height);
				double IoU = getIOU(winStruct, refStructure);
				cout << "IoU: " << IoU << endl;

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
			if (isinf(IOU))
				IOU = 0;
			totalIOU += IOU;
			count++;
		}
	}
	cout << "average IOU of " << count << " images: " << totalIOU / count << endl;
}

void doCmdIOUyolo(WinDetector& detector, const string& testImgDir) {
	Detector& yoloDetector = detector;
	double totalIOU = 0;
	int count = 0;
	for (const auto& entry : std::filesystem::directory_iterator(testImgDir)) {
		string fileName = entry.path().generic_string();
		if (fileName.substr(fileName.size() - 3).compare("jpg") == 0) {
			vector<bbox_t> bboxes;
			WindowStructure winStruct, refWinSt;
			string fileNameWOExt = fileName.substr(0, fileName.size() - 4);
			bboxes = yoloDetector.detect(fileName);
			setWindowStructure(bboxes, winStruct);
			cv::Size2i imgSize = detector.lastDetectedImageSize;
			readWindows(fileNameWOExt + "_quadrangle.txt", refWinSt, imgSize.width, imgSize.height);
			double IOU = getIOU(winStruct, refWinSt);
			cout << "image " << fileName << " IOU: " << IOU << endl;
			if (isinf(IOU))
				IOU = 0;
			totalIOU += IOU;
			count++;
		}
	}
	cout << "average IOU of " << count << " images: " << totalIOU / count << endl;
}

void doCmdTestYolo(WinDetector& detector, const char* imgDir) {
	Detector& yoloDetector = detector;
	if (imgDir == nullptr) {
		string imgFileName;
		while (true) {
			WindowStructure winStruct, refStructure;
			vector<bbox_t> bboxes;
			cout << "enter image file name: ";
			cin >> imgFileName;
			string fileNameWOExt = imgFileName.substr(0, imgFileName.size() - 4);
			bboxes = yoloDetector.detect(imgFileName);
			cv::Mat img = cv::imread(imgFileName);
			setWindowStructure(bboxes, winStruct);
			drawWindows(img, winStruct, detector.windowNames);
			readWindows(fileNameWOExt + "_quadrangle.txt", refStructure, img.size().width, img.size().height);
			double IoU = getIOU(winStruct, refStructure);
			cout << "IoU: " << IoU << endl;

			cv::namedWindow(imgFileName, CV_WINDOW_NORMAL);
			cv::imshow(imgFileName, img);
			cv::waitKey();
			cv::destroyWindow(imgFileName);
		}
	}
	else {
		for (const auto& entry : filesystem::directory_iterator(imgDir)) {
			string imgFileName = entry.path().generic_string();
			if (imgFileName.substr(imgFileName.size() - 3).compare("jpg") == 0) {
				WindowStructure winStruct, refStructure;
				vector<bbox_t> bboxes;
				string fileNameWOExt = imgFileName.substr(0, imgFileName.size() - 4);
				bboxes = yoloDetector.detect(imgFileName);
				setWindowStructure(bboxes, winStruct);
				cv::Mat img = cv::imread(imgFileName);
				drawWindows(img, winStruct, detector.windowNames);
				readWindows(fileNameWOExt + "_quadrangle.txt", refStructure, img.size().width, img.size().height);
				double IoU = getIOU(winStruct, refStructure);
				cout << "IoU: " << IoU << endl;

				cv::namedWindow(imgFileName, CV_WINDOW_NORMAL);
				cv::imshow(imgFileName, img);
				cv::waitKey();
				cv::destroyWindow(imgFileName);
			}
		}
		return;
	}
}