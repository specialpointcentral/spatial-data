#ifndef _NODE_H_
#define _NODE_H_
#include "mbr.h"
#include <string>
#include <cstdint>
#include <cmath>

enum Node_type
{
    leaf,
    root,
    invd,
};

struct node
{
    using Point = std::array<double, 2>;

    node() {}
    node(const node &n) : _type(n._type), _mbr(n._mbr)
    {
        if (_type == leaf)
            _id = n._id;
        else
            _ptr = n._ptr;
    }
    node(std::vector<Point> ps, int id) : _type(leaf), _id(id)
    {
        _mbr.set_mbr(ps);
    }

    void set_invalid() {
        _type = invd;
    }
    bool is_valid() {
        return _type != invd;
    }
    bool is_leaf() {
        return _type == leaf;
    }

    void set_root(void *node_ptr, mbr &node_mbr)
    {
        _type = root;
        _ptr = node_ptr;
        _mbr = node_mbr;
    }
    void set_root(void *node_ptr, std::size_t n)
    {
        node *__node = (node *)node_ptr;
        std::vector<Point> ps;
        for (int i = 0; i < n; ++i)
        {
            if (__node[i].is_valid())
            {
                ps.push_back(__node[i]._mbr.get_xy_low());
                ps.push_back(__node[i]._mbr.get_xy_high());
            }
        }
        mbr __mbr(ps);
        set_root(node_ptr, __mbr);
    }
    bool overlap(const Point &x, const Point &y)
    {
        return _mbr.overlap(x, y);
    }

    double get_distance(const Point &p)
    {
        /*  1 | 2 | 3
         * -----------
         *  4 | 5 | 6
         * -----------
         *  7 | 8 | 9
         */
        double x_left = p[0] - _mbr._x_low;
        double x_right = p[0] - _mbr._x_high;
        double y_top = p[1] - _mbr._y_low;
        double y_bottom = p[1] - _mbr._y_high;
        double x2 = 0, y2 = 0;
        if (x_left < 0 || x_right > 0)
            x2 = std::min(std::pow(x_left, 2), std::pow(x_right, 2));
        if (y_top < 0 || y_bottom > 0)
            y2 = std::min(std::pow(y_top, 2), std::pow(y_bottom, 2));

        return x2 + y2;
    }

    Point get_value()
    {
        return _mbr.get_center();
    }

    int get_id()
    {
        if (is_leaf())
            return _id;
        else
            return std::numeric_limits<int>::max();
    }

    node *get_leaf()
    {
        if (!is_leaf() && is_valid())
            return (node *)_ptr;
        else
            return nullptr;
    }

    std::string print_node()
    {
        // [isnonleaf, node-id, [[id1, MBR1], [id2, MBR2], ..., [idn, MBRn]]]
        // print [id1, MBR1]
        std::string s;
        s.clear();
        s += "[";
        if (is_leaf())
            s += std::to_string(_id);
        else
            s += std::to_string((uint64_t)_ptr);
        s += ",";
        s += _mbr.print_mbr();
        s += "]";
        return s;
    }

    Node_type _type;
    union
    {
        int _id;
        void *_ptr;
    };
    mbr _mbr;
};

#endif
