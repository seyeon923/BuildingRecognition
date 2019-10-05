//#include <cstdio>
//#include <cerrno>
//#include <cstring>
//#include <string>
//#include <cstdlib>
//#include <direct.h>
//#include <fstream>
//#include <iostream>
//
//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
//
//#define OPENCV
//#include "yolo_v2_class.hpp"
//
//
//using namespace std;
//using namespace cv;
//
//std::vector<std::string> objects_names_from_file(std::string const filename);
//void alignImages(Mat& im1, Mat& im2, Mat& im1Reg, Mat& h, int maxFeatures = 500, float goodMatchPercent = 0.15f);
//
//int main(int argc, char* argv[]) {
//	string dataFile = "obj.data";
//	string cfgFile = "yolov3-UnivCenterMultiDivFace.cfg";
//	string weightsFile = "yolov3-train_best.weights";
//	string namesFile = "obj.names";
//	string imageFile = "img/UniversityCenterPos10002.jpg";
//
//	if (argc > 4) {	// <command> <namesFile> <cfgFile> <weightsFile> <imageFile>
//		namesFile = argv[1];
//		cfgFile = argv[2];
//		weightsFile = argv[3];
//		imageFile = argv[4];
//	}
//	else if (argc > 1) imageFile = argv[1]; // or <command> <imageFile>
//
//	float const thresh = (argc > 5) ? stof(argv[5]) : 0.25;
//
//	cv::Mat image = cv::imread(imageFile, cv::IMREAD_COLOR);
//	if (image.empty()) {
//		cout << "Could not open or find the image" << endl;
//		return 1;
//	}
//	cout << "Image " << imageFile << " is loaded successfully" << endl;
//
//	Detector detector(cfgFile, weightsFile);
//
//	vector<string> objNames = objects_names_from_file(namesFile);
//
//	vector<bbox_t> bboxes = detector.detect(image, thresh);
//
//	vector<Mat> warpedVec, refVec;
//	vector<string> clsNamesDetected;
//	for (auto bbox : bboxes) {
//		Rect rect(bbox.x - (int)(bbox.w * 0.05), bbox.y - (int)(bbox.h * 0.05), (int)(bbox.w * 1.05), (int)(1.05 * bbox.h));
//		Mat detected = image(rect);
//		string clsName = objNames[bbox.obj_id];
//		string refImageName = "ref_" + clsName + ".jpg";
//		Mat ref = imread(refImageName, IMREAD_COLOR);
//		if (ref.empty()) {
//			cout << "image " << refImageName << " loading fail!\n";
//			continue;
//		}
//		Mat warped(ref.size(), CV_8UC3);
//		Mat H(Size(3, 3), CV_32FC1);
//		
//		clsNamesDetected.push_back(clsName);
//		refVec.push_back(ref);
//		alignImages(detected, ref, warped, H);
//		warpedVec.push_back(warped);
//	}
//
//	// mark detections on the image
//	for (auto bbox : bboxes) {
//		Rect rect(bbox.x, bbox.y, bbox.w, bbox.h);
//		string clsName = objNames[bbox.obj_id];
//		rectangle(image, rect, Scalar(0, 0, 255), 2);
//		putText(image, clsName, Point(rect.x, rect.y + 20), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
//	}
//	
//	// show original image
//	cv::namedWindow("Original", cv::WINDOW_NORMAL);
//	resizeWindow("Original", 1280, 720);
//	imshow("Original", image);
//
//	// show warped image
//	for (int i = 0; i < warpedVec.size(); i++) {
//		string windowName = i + "-" + clsNamesDetected[i];
//		namedWindow(windowName, WINDOW_NORMAL);
//		resizeWindow(windowName, refVec[i].size());
//		Mat frame(Size(refVec[i].cols * 2, refVec[i].rows), CV_8UC3);
//		hconcat(warpedVec[i], refVec[i], frame);
//		imshow(windowName, frame);
//	}
//
//	waitKey(0);
//	return 0;
//}
//
//std::vector<std::string> objects_names_from_file(std::string const filename) {
//	std::ifstream file(filename);
//	std::vector<std::string> file_lines;
//	if (!file.is_open()) return file_lines;
//	for (std::string line; getline(file, line);) file_lines.push_back(line);
//	std::cout << "object names loaded \n";
//	return file_lines;
//}
//
//void alignImages(Mat& im1, Mat& im2, Mat& im1Reg, Mat& h, int maxFeatures = 500, float goodMatchPercent = 0.15f){
//	// Convert images to grayscale
//	Mat im1Gray, im2Gray;
//	cvtColor(im1, im1Gray, CV_BGR2GRAY);
//	cvtColor(im2, im2Gray, CV_BGR2GRAY);
//
//	// Variables to store keypoints and descriptors
//	std::vector<KeyPoint> keypoints1, keypoints2;
//	Mat descriptors1, descriptors2;
//
//	// Detect ORB features and compute descriptors.
//	Ptr<Feature2D> orb = ORB::create(maxFeatures);
//	orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
//	orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);
//
//	// Match features.
//	std::vector<DMatch> matches;
//	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
//	matcher->match(descriptors1, descriptors2, matches, Mat());
//
//	// Sort matches by score
//	std::sort(matches.begin(), matches.end());
//
//	// Remove not so good matches
//	const int numGoodMatches = matches.size() * goodMatchPercent;
//	matches.erase(matches.begin() + numGoodMatches, matches.end());
//
//
//	//// Draw top matches
//	//Mat imMatches;
//	//drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
//	//imwrite("matches.jpg", imMatches);
//
//
//	// Extract location of good matches
//	std::vector<Point2f> points1, points2;
//
//	for (size_t i = 0; i < matches.size(); i++)
//	{
//		points1.push_back(keypoints1[matches[i].queryIdx].pt);
//		points2.push_back(keypoints2[matches[i].trainIdx].pt);
//	}
//
//	// Find homography
//	h = findHomography(points1, points2, RANSAC);
//
//	// Use homography to warp image
//	warpPerspective(im1, im1Reg, h, im2.size());
//
//}