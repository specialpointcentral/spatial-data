#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <node.h>
#include <rtree.h>

std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string value;

    while (std::getline(ss, value, ',')) {
        result.push_back(value);
    }

    return result;
}

int main(int argc, char **argv) {
    using Point = std::array<double, 2>;

    std::string coords_name = "../Provided_files/coords.txt";
    std::string offsets_name = "../Provided_files/offsets.txt";

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
        } catch (const std::exception& e) {
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
        } catch (const std::exception& e) {
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

    // part 1
    rtree tree(leaf_node);

    std::string rtree_name = "../Provided_files/Rtree.txt";
    std::ofstream rtrees(rtree_name);
    if (!rtrees) {
        std::cerr << "Failed to open file: " << rtree_name << std::endl;
        return 1;
    }
    std::string print_tree = tree.print_tree();
    rtrees.write(print_tree.c_str(), print_tree.size());

    rtrees.close();

    // part 2
    std::string rquery_name = "../Provided_files/Rqueries.txt";
    std::ifstream rqueries(rquery_name);
    if (!rqueries) {
        std::cerr << "Failed to open file: " << rquery_name << std::endl;
        return 1;
    }

    struct Rectangle
    {
        double x_low;
        double y_low;
        double x_high;
        double y_high;
    };

    std::vector<Rectangle> rectangles;

    while (std::getline(rqueries, line))
    {
        std::stringstream ss(line);
        Rectangle rect;

        if (ss >> rect.x_low >> rect.y_low >> rect.x_high >> rect.y_high)
        {
            rectangles.push_back(rect);
        }
    }
    rqueries.close();

    std::string rqueries_res;
    rqueries_res.clear();
    for (auto i = 0; i < rectangles.size(); ++i) {
        std::vector<int> res;
        auto rect = rectangles[i];
        res = tree.range_query({rect.x_low, rect.x_high},{rect.y_low, rect.y_high});
        rqueries_res += std::to_string(i) + "(" + std::to_string(res.size()) + "):";
        for (auto j = 0; j < res.size(); ++j)
        {
            rqueries_res += std::to_string(res[j]);
            if (j != res.size() - 1)
                rqueries_res += ",";
            else
                rqueries_res += "\n";
        }
    }

    std::cout << rqueries_res;

    // part 3
    std::string knnrquery_name = "../Provided_files/NNqueries.txt";
    std::ifstream knnqueries(knnrquery_name);
    if (!knnqueries) {
        std::cerr << "Failed to open file: " << knnrquery_name << std::endl;
        return 1;
    }

    int k = 10;
    std::vector<Point> points;
    while (std::getline(knnqueries, line))
    {
        std::stringstream ss(line);
        Point point;

        if (ss >> point[0] >> point[1])
        {
            points.push_back(point);
        }
    }
    knnqueries.close();

    std::string knnqueries_res;
    knnqueries_res.clear();
    for (auto i = 0; i < points.size(); ++i)
    {
        std::vector<int> res;
        res = tree.knn_query(points[i], k);
        knnqueries_res += std::to_string(i) + "(" + std::to_string(res.size()) + "):";
        for (auto j = 0; j < res.size(); ++j)
        {
            knnqueries_res += std::to_string(res[j]);
            if (j != res.size() - 1)
                knnqueries_res += ",";
            else
                knnqueries_res += "\n";
        }
    }

    std::cout << knnqueries_res;

    return 0;
}