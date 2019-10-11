#ifndef __UTILITY_HPP
#define __UTILITY_HPP

template<class T>
void clearPointerVec(vector<T*>& vec) {
	for (T* pEle : vec)
		delete pEle;
	vec.clear();
}

#endif