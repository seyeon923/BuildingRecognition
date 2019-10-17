#include "utility.hpp"

using namespace std;

cv::Scalar objIdToColor(int objId) {
	int const colors[6][3] = { { 1,0,1 },{ 0,0,1 },{ 0,1,1 },{ 0,1,0 },{ 1,1,0 },{ 1,0,0 } };
	int const offset = objId * 123457 % 6;
	int const color_scale = 150 + (objId * 123457) % 100;
	cv::Scalar color(colors[offset][0], colors[offset][1], colors[offset][2]);
	color *= color_scale;
	return color;
}

std::string spaceToUnderBar(const std::string& str) {
	string ret = str;
	for (size_t pos = ret.find(' '); pos != string::npos; pos = ret.find(' '))
		ret[pos] = '_';
	return ret;
}