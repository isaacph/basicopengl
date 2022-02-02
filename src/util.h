#ifndef _UTIL_H
#define _UTIL_H
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cassert>

std::string readFile(const std::string_view& file);
std::vector<unsigned char> readImage(const std::string_view& file, int& width, int& height);
int divRoundDown(int a, int b);
int modRoundDown(int a, int b);
inline int floorInt(float num) {
    return num < 0 ? (int) num - 1 : (int) num;
}

inline float limitMagnitude(float x, float max) {
    if (x > max) return max;
    else if (x < -max) return -max;
    return x;
}

#endif
