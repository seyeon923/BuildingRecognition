#ifndef __UTILITY_HPP
#define __UTILITY_HPP

#include <opencv2/core/types.hpp>
#include <vector>
#include <string>

template<class T>
void clearPointerVec(std::vector<T*>& vec) {
	for (T* pEle : vec)
		delete pEle;
	vec.clear();
}

cv::Scalar objIdToColor(int objId);
std::string spaceToUnderBar(const std::string& str);

#endif