#pragma once
#ifndef INTERVAL_H
#define INTERVAL_H
#include <limits>
const double localInfinity = std::numeric_limits<double>::infinity();

class interval {
public:
    double min, max;
    
    interval() : min(+localInfinity), max(-localInfinity) {} // Default interval is empty

    interval(double _min, double _max) : min(_min), max(_max) {}

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    static const interval empty, universe;
};

const static interval empty(+localInfinity, -localInfinity);
const static interval universe(-localInfinity, +localInfinity);

#endif
