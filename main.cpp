#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <node.h>
#include <rtree.h>
#include <sstream>
#include <string>
#include <vector>

using Point = std::array<double, 2>;

int indexing(const std::string &coords_name, const std::string &offsets_name,
             const std::string &rtree_name);
int range_search(const std::string &rtree_name, const std::string &rquery_name,
                 const std::string &result_name);
int knn_search(std::string rtree_name, std::string knnrquery_name,
               int k, std::string result_name);

std::vector<std::string> parseCSVLine(const std::string &line)
{
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string value;

    while (std::getline(ss, value, ',')) {
        result.push_back(value);
    }

    return result;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Too few Args, you need identify types, like \"" << argv[0]
                  << " <index|range|knn> <Other Args>\"" << std::endl;
        return 1;
    }

    rtree tree;

    // part 1
    if (std::strstr(argv[1], "index")) {
        if (argc < 5) {
            std::cerr << "Index needs at least 4 Args, which like \"" << argv[0]
                      << " index <coords> <offsets> <output>\"" << std::endl;
            return 1;
        }

        // std::string coords_name = "../Provided_files/coords.txt";
        // std::string offsets_name = "../Provided_files/offsets.txt";
        // std::string rtree_name = "../Provided_files/Rtree.txt";

        std::string coords_name = argv[2];
        std::string offsets_name = argv[3];
        std::string rtree_name = argv[4];

        return indexing(coords_name, offsets_name, rtree_name);
    }

    // part 2
    if (std::strstr(argv[1], "range")) {
        if (argc < 4) {
            std::cerr << "Range queries needs at least 3 Args, which like \"" << argv[0]
                      << " range <rtree> <rqueries> [<output>]\"" << std::endl;
            return 1;
        }

        // std::string rtree_name = "../Provided_files/Rtree.txt";
        // std::string rquery_name = "../Provided_files/Rqueries.txt";
        std::string rtree_name = argv[2];
        std::string rquery_name = argv[3];
        std::string result_name;
        if (argc > 4)
            result_name = argv[4];

        return range_search(rtree_name, rquery_name, result_name);
    }

    // part 3
    if (std::strstr(argv[1], "knn")) {
        if (argc < 5) {
            std::cerr << "Knn queries needs at least 4 Args, which like \"" << argv[0]
                      << " knn <rtree> <NNqueries> <k> [<output>]\"" << std::endl;
            return 1;
        }

        // std::string rtree_name = "../Provided_files/Rtree.txt";
        // std::string knnrquery_name = "../Provided_files/NNqueries.txt";
        std::string rtree_name = argv[2];
        std::string knnrquery_name = argv[3];
        int k = atoi(argv[4]);
        std::string result_name;
        if (argc > 5)
            result_name = argv[5];

        return knn_search(rtree_name, knnrquery_name, k, result_name);
    }
    return 0;
}

int indexing(const std::string &coords_name, const std::string &offsets_name,
             const std::string &rtree_name)
{
    rtree tree;
    // read coords.txt
    std::ifstream coords(coords_name);
    if (!coords) {
        std::cerr << "Failed to open file: " << coords_name << std::endl;
        return 1;
    }

    std::vector<Point> xyValues;

    std::string line;
    while (std::getline(coords, line)) {
        std::vector<std::string> values = parseCSVLine(line);
        if (values.size() != 2) {
            std::cerr << "Invalid line: " << line << std::endl;
            continue;
        }

        Point p;
        try {
            p[0] = std::stof(values[0]);
            p[1] = std::stof(values[1]);
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse values: " << line << std::endl;
            continue;
        }

        xyValues.emplace_back(p);
    }

    coords.close();

    // read offsets.txt
    std::ifstream offsets(offsets_name);
    if (!offsets) {
        std::cerr << "Failed to open file: " << offsets_name << std::endl;
        return 1;
    }

    std::vector<std::tuple<int, int, int>> idOffsets;

    while (std::getline(offsets, line)) {
        std::vector<std::string> values = parseCSVLine(line);
        if (values.size() != 3) {
            std::cerr << "Invalid line: " << line << std::endl;
            continue;
        }

        int id, startOffset, endOffset;
        try {
            id = std::stoi(values[0]);
            startOffset = std::stoi(values[1]);
            endOffset = std::stoi(values[2]);
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse values: " << line << std::endl;
            continue;
        }

        idOffsets.emplace_back(id, startOffset, endOffset);
    }

    offsets.close();

    // build node
    std::vector<node> leaf_node(idOffsets.size());
    for (int i = 0; i < idOffsets.size(); ++i) {
        auto off = idOffsets[i];
        std::vector<Point> ps;
        for (int j = std::get<1>(off); j <= std::get<2>(off); ++j) {
            ps.push_back(xyValues[j]);
        }
        leaf_node[i] = node(ps, std::get<0>(off));
    }

    tree.build_tree(leaf_node);

    // output
    std::ofstream rtrees(rtree_name);
    if (!rtrees) {
        std::cerr << "Failed to open file: " << rtree_name << std::endl;
        return 1;
    }
    std::string print_tree = tree.print_tree();
    rtrees.write(print_tree.c_str(), print_tree.size());

    rtrees.close();

    return 0;
}

int range_search(const std::string &rtree_name, const std::string &rquery_name,
                 const std::string &result_name)
{
    rtree tree;
    // build tree from rtree.txt
    std::ifstream rtrees(rtree_name);
    if (!rtrees) {
        std::cerr << "Failed to open file: " << rtree_name << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> nodes;
    while (std::getline(rtrees, line)) {
        nodes.push_back(line);
    }
    rtrees.close();

    tree.build_tree(nodes);

    // begin range query
    std::ifstream rqueries(rquery_name);
    if (!rqueries) {
        std::cerr << "Failed to open file: " << rquery_name << std::endl;
        return 1;
    }

    struct Rectangle {
        double x_low;
        double y_low;
        double x_high;
        double y_high;
    };

    std::vector<Rectangle> rectangles;

    while (std::getline(rqueries, line)) {
        std::stringstream ss(line);
        Rectangle rect;

        if (ss >> rect.x_low >> rect.y_low >> rect.x_high >> rect.y_high) {
            rectangles.push_back(rect);
        }
    }
    rqueries.close();

    std::string rqueries_res;
    rqueries_res.clear();
    for (auto i = 0; i < rectangles.size(); ++i) {
        std::vector<int> res;
        auto rect = rectangles[i];
        res = tree.range_query({rect.x_low, rect.x_high}, {rect.y_low, rect.y_high});
        rqueries_res += std::to_string(i) + "(" + std::to_string(res.size()) + "):";
        for (auto j = 0; j < res.size(); ++j) {
            rqueries_res += std::to_string(res[j]);
            if (j != res.size() - 1)
                rqueries_res += ",";
            else
                rqueries_res += "\n";
        }
        // if no results, need add "\n"
        if (!res.size())
            rqueries_res += "\n";
    }

    if (!result_name.empty()) {
        std::ofstream results(result_name);
        if (!results) {
            std::cerr << "Failed to open file: " << result_name << std::endl;
            return 1;
        }
        results.write(rqueries_res.c_str(), rqueries_res.size());

        results.close();
    } else
        std::cout << rqueries_res;

    return 0;
}

int knn_search(std::string rtree_name, std::string knnrquery_name,
               int k, std::string result_name)
{
    rtree tree;
    // build tree from rtree.txt
    std::ifstream rtrees(rtree_name);
    if (!rtrees) {
        std::cerr << "Failed to open file: " << rtree_name << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> nodes;
    while (std::getline(rtrees, line)) {
        nodes.push_back(line);
    }
    rtrees.close();

    tree.build_tree(nodes);

    std::ifstream knnqueries(knnrquery_name);
    if (!knnqueries) {
        std::cerr << "Failed to open file: " << knnrquery_name << std::endl;
        return 1;
    }

    std::vector<Point> points;
    while (std::getline(knnqueries, line)) {
        std::stringstream ss(line);
        Point point;

        if (ss >> point[0] >> point[1]) {
            points.push_back(point);
        }
    }
    knnqueries.close();

    std::string knnqueries_res;
    knnqueries_res.clear();
    for (auto i = 0; i < points.size(); ++i) {
        std::vector<int> res;
        res = tree.knn_query(points[i], k);
        knnqueries_res += std::to_string(i) + "(" + std::to_string(res.size()) + "):";
        for (auto j = 0; j < res.size(); ++j) {
            knnqueries_res += std::to_string(res[j]);
            if (j != res.size() - 1)
                knnqueries_res += ",";
            else
                knnqueries_res += "\n";
        }
        // if no results, need add "\n"
        if (!res.size())
            knnqueries_res += "\n";
    }

    if (!result_name.empty()) {
        std::ofstream results(result_name);
        if (!results) {
            std::cerr << "Failed to open file: " << result_name << std::endl;
            return 1;
        }
        results.write(knnqueries_res.c_str(), knnqueries_res.size());

        results.close();
    } else
        std::cout << knnqueries_res;

    return 0;
}