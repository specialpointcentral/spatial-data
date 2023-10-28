#ifndef _MBR_H_
#define _MBR_H_

#include <array>
#include <limits>
#include <string>
#include <vector>

struct mbr {
    using Point = std::array<double, 2>;

    mbr()
    {
        mbr(0, 0, 0, 0);
    }
    mbr(const mbr &_mbr) : _x_low(_mbr._x_low), _x_high(_mbr._x_high),
                           _y_low(_mbr._y_low), _y_high(_mbr._y_high) {}
    mbr(double x_low, double x_high,
        double y_low, double y_high) : _x_low(x_low), _x_high(x_high),
                                       _y_low(y_low), _y_high(y_high) {}
    mbr(std::vector<Point> &polygon)
    {
        set_mbr(polygon);
    }

    void set_mbr(std::vector<Point> &polygon)
    {
        _x_low = _y_low = std::numeric_limits<double>::max();
        _x_high = _y_high = std::numeric_limits<double>::lowest();
        for (auto p : polygon) {
            _x_low = std::min(p[0], _x_low);
            _x_high = std::max(p[0], _x_high);
            _y_low = std::min(p[1], _y_low);
            _y_high = std::max(p[1], _y_high);
        }
    }

    bool overlap(const Point &x, const Point &y)
    {
        bool xOverlap = (_x_low <= x[1]) && (_x_high >= x[0]);
        bool yOverlap = (_y_low <= y[1]) && (_y_high >= y[0]);

        return xOverlap && yOverlap;
    }

    std::string print_mbr()
    {
        std::string s;
        s += "[";
        s += std::to_string(_x_low) + "," + std::to_string(_x_high) + ",";
        s += std::to_string(_y_low) + "," + std::to_string(_y_high);
        s += "]";
        return s;
    }

    Point get_xy_low()
    {
        Point xylow;
        xylow[0] = _x_low;
        xylow[1] = _y_low;
        return xylow;
    }

    Point get_xy_high()
    {
        Point xyhigh;
        xyhigh[0] = _x_high;
        xyhigh[1] = _y_high;
        return xyhigh;
    }

    Point get_center()
    {
        Point center;
        center[0] = (_x_low + _x_high) * 0.5;
        center[1] = (_y_low + _y_high) * 0.5;
        return center;
    }

    double _x_low, _x_high, _y_low, _y_high;
};

#endif
