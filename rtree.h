#ifndef _RTREE_H_
#define _RTREE_H_

#include "node.h"
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <zorder_knn/less.hpp>

constexpr std::size_t capacity = 20;
constexpr std::size_t low_water = 8;
constexpr std::size_t node_cap = capacity;

struct NodeInfo {
    bool isNonLeaf;
    uint64_t nodeId; // contain id or ptr
    std::vector<std::pair<uint64_t, mbr>> childNodes;
    bool linked;
};
struct rtree {
    using Point = std::array<double, 2>;

    rtree() : _root(nullptr) {}
    void build_tree(std::vector<node> &nodes)
    {
        // first we need sort these nodes
        std::sort(nodes.begin(), nodes.end(), [](node &node1, node &node2) {
            zorder_knn::Less<Point, 2> zorder;
            return zorder(node1.get_value(), node2.get_value()); });
        // then we build the tree
        std::queue<node *> level_node;
        std::size_t level_node_size = nodes.size();
        std::size_t level = 0;
        // copy each node
        for (std::size_t i = 0; i < level_node_size; i = i + node_cap) {
            node *tile = (node *)malloc(sizeof(node) * capacity);
            // copy each node
            for (std::size_t j = 0; j < capacity; ++j) {
                if (j < node_cap && i + j < level_node_size)
                    tile[j] = nodes[i + j];
                else
                    tile[j].set_invalid();
            }
            level_node.push(tile);
        }
        // build the root
        while (++level) {
            // check if it is root
            level_node_size = level_node.size();
            std::cout << level_node_size << " node(s) at level " << level - 1 << std::endl;
            if (level_node_size == 1) {
                _root = level_node.front();
                break;
            }
            // build tree in each level
            for (std::size_t i = 0; i < level_node_size; i = i + node_cap) {
                node *tile = (node *)malloc(sizeof(node) * capacity);
                // set each node
                for (std::size_t j = 0; j < capacity; ++j) {
                    if (j < node_cap && i + j < level_node_size) {
                        tile[j].set_root(level_node.front(), node_cap);
                        level_node.pop();
                    } else
                        tile[j].set_invalid();
                }
                level_node.push(tile);
            }
        }
    }

    static NodeInfo paraseNode(std::string &node)
    {
        // Remove the "[]"
        std::string data = node.substr(1, node.length() - 2);

        // isNonLeaf
        size_t pos = data.find(',');
        bool isNonLeaf = !!std::stoi(data.substr(0, pos));
        data = data.substr(pos + 1);

        // node-id
        pos = data.find(',');
        uint64_t nodeId = std::stoull(data.substr(0, pos));
        data = data.substr(pos + 1);

        // childnodes
        std::vector<std::pair<uint64_t, mbr>> childNodes;
        // move "[]"
        data = data.substr(data.find('[') + 1, data.rfind(']') - data.find('[') - 1);

        pos = data.find('[');
        while (pos != std::string::npos) {
            // remvoe "[]"
            data = data.substr(pos + 1);

            // Parse the id and MBR
            pos = data.find(',');
            uint64_t _id = std::stoull(data.substr(0, pos));
            data = data.substr(pos + 1);

            pos = data.find(']');
            std::string s_mbr = data.substr(data.find('[') + 1, pos - 1);
            // mbr
            size_t pos_mbr = s_mbr.find(',');
            double x_low = std::stod(s_mbr.substr(0, pos_mbr));
            s_mbr = s_mbr.substr(pos_mbr + 1);

            pos_mbr = s_mbr.find(',');
            double x_high = std::stod(s_mbr.substr(0, pos_mbr));
            s_mbr = s_mbr.substr(pos_mbr + 1);

            pos_mbr = s_mbr.find(',');
            double y_low = std::stod(s_mbr.substr(0, pos_mbr));
            s_mbr = s_mbr.substr(pos_mbr + 1);

            double y_high = std::stod(s_mbr);

            mbr _mbr(x_low, x_high, y_low, y_high);

            data = data.substr(pos + 1);

            childNodes.push_back(std::make_pair(_id, _mbr));

            pos = data.find('[');
        }

        return {isNonLeaf, nodeId, childNodes, false};
    }

    void build_tree(const std::vector<std::string> &_nodes)
    {
        // [isnonleaf, node-id, [[id1, MBR1], [id2, MBR2], ..., [idn, MBRn]]]
        std::unordered_map<uint64_t, NodeInfo> nodeinfo;
        for (auto _node : _nodes) {
            NodeInfo info = paraseNode(_node);
            auto _node_id = info.nodeId;
            // malloc data
            node *tile = (node *)malloc(sizeof(node) * capacity);
            // save ptr into nodeid
            info.nodeId = (uint64_t)tile;
            // save the data
            nodeinfo[_node_id] = info;
        }
        // save node information into node
        for (auto it = nodeinfo.begin(); it != nodeinfo.end(); ++it) {
            // insert data
            auto _nodeinfo = it->second;
            auto _tile = (node *)_nodeinfo.nodeId;
            for (int i = 0; i < capacity; ++i) {
                if (i >= _nodeinfo.childNodes.size()) {
                    _tile[i].set_invalid();
                    continue;
                }
                // leaf => id, root => ptr
                auto _node_id = _nodeinfo.childNodes[i].first;
                auto _node_mbr = _nodeinfo.childNodes[i].second;
                if (_nodeinfo.isNonLeaf) {
                    auto _next_node = (node *)nodeinfo[_node_id].nodeId;
                    _tile[i].set_root(_next_node, _node_mbr);
                    // mark it is linked
                    nodeinfo[_node_id].linked = true;
                } else
                    _tile[i].set_leaf(_node_id, _node_mbr);
            }
        }
        // find root
        int contain = 0;
        for (auto it = nodeinfo.begin(); it != nodeinfo.end(); ++it) {
            // insert data
            auto _nodeinfo = it->second;
            if (!_nodeinfo.linked) {
                _root = (node *)_nodeinfo.nodeId;
                break;
            }
        }
    }

    ~rtree()
    {
        // free each node
        std::queue<node *> nodes;
        if (_root == nullptr)
            return;
        nodes.push(_root);
        while (!nodes.empty()) {
            node *cur = nodes.front();
            // enqueue each leaf
            for (std::size_t i = 0; i < capacity; ++i) {
                node *next = cur[i].get_leaf();
                if (next != nullptr)
                    nodes.push(next);
            }
            free(cur);
            nodes.pop();
        }
    }

    std::string print_tree()
    {
        std::vector<std::string> s;
        std::queue<node *> nodes;
        if (_root == nullptr)
            return std::string("");
        s.clear();
        nodes.push(_root);
        while (!nodes.empty()) {
            std::string _s;
            node *cur = nodes.front();
            // print [isnonleaf, node-id, [[id1, MBR1], [id2, MBR2], ..., [idn, MBRn]]]
            _s += "[";
            // print head => isnonleaf, node-id
            _s += std::to_string(!cur->is_leaf()) + ",";
            // node-id
            _s += std::to_string((uint64_t)cur) + ",";
            // enqueue each leaf
            _s += "[";
            for (std::size_t i = 0; i < capacity; ++i) {
                if (!cur[i].is_valid())
                    continue;
                // insert into s
                _s += cur[i].print_node();
                // join child into queue
                node *next = cur[i].get_leaf();
                if (next != nullptr)
                    nodes.push(next);
                if (i != capacity - 1)
                    _s += ",";
            }
            _s += "]";
            nodes.pop();
            _s += "]";
            s.push_back(_s);
        }

        std::string s_tree;
        s_tree.clear();
        for (auto it = s.rbegin(); it != s.rend(); ++it) {
            s_tree += *it + "\n";
        }
        return s_tree;
    }

    std::vector<int> range_query(Point x, Point y)
    {
        std::vector<int> res;
        std::queue<node *> nodes;
        if (_root == nullptr)
            return res;
        nodes.push(_root);
        while (!nodes.empty()) {
            node *cur = nodes.front();
            // enqueue and check each leaf
            for (std::size_t i = 0; i < capacity; ++i) {
                // cannot in this node
                if (!cur[i].overlap(x, y))
                    continue;
                // leaf need to record
                if (cur[i].is_leaf()) {
                    res.push_back(cur[i].get_id());
                    continue;
                }
                node *next = cur[i].get_leaf();
                if (next != nullptr)
                    nodes.push(next);
            }
            nodes.pop();
        }
        return res;
    }

    std::vector<int> knn_query(Point p, int k)
    {
        struct GridSearch {
            double _distance;
            node *_node;
            GridSearch() : _distance(0), _node(nullptr) {}
            GridSearch(const GridSearch &g) : _distance(g._distance), _node(g._node) {}
            GridSearch(node *__node, const Point &__p) : _node(__node)
            {
                _distance = __node->get_distance(__p);
            }

            bool operator<(const GridSearch &other) const
            {
                return _distance > other._distance;
            }
        };
        std::vector<int> res;
        std::priority_queue<GridSearch> search_q;
        if (_root == nullptr)
            return res;
        // enqueue root
        for (std::size_t i = 0; i < capacity; ++i)
            if (_root[i].is_valid())
                search_q.push(GridSearch(_root + i, p));

        GridSearch first;
        while (k && !search_q.empty()) {
            first = search_q.top();
            search_q.pop();
            // point
            if (first._node->is_leaf()) {
                // get one
                k--;
                res.push_back(first._node->get_id());
                continue;
            }
            // insert node
            node *next = first._node->get_leaf();
            for (std::size_t i = 0; i < capacity; ++i) {
                // not leaf and valid
                if (next[i].is_valid())
                    search_q.push(GridSearch(next + i, p));
            }
        }
        // K is too big
        if (k)
            res.clear();

        return res;
    }

    node *_root;
};

#endif
