#ifndef __YOLO_HPP
#define __YOLO_HPP

#define OPENCV
#include "yolo_v2_class.hpp"

std::vector<std::string> objects_names_from_file(std::string const filename);
void alignImages(Mat& im1, Mat& im2, Mat& im1Reg, Mat& h, int maxFeatures = 500, float goodMatchPercent = 0.15f);


#endif