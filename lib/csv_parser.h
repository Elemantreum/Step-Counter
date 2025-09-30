#pragma once
#include <string.h>
#include <vector>
#include <tuple>
#include <fstream>
#include <sstream>
#include <exception>

template <typename T>
T convert_to (const std::string &str)
{
    std::istringstream ss(str);
    T num;
    ss >> num;
    return num;
}

template <typename... Args>
std::vector<std::tuple<Args...>> ParseCSV(std::string file_path, char delimeter) {
    std::ifstream csv_file(file_path);

    if (!csv_file.is_open()) {
        throw std::runtime_error("Can't open CSV file!");
    }

    std::vector<std::tuple<Args...>> result;

    std::string line;

    while(std::getline(csv_file, line)) {

        std::istringstream stream_line(line);

        std::vector<std::string> values;
        std::string value;

        while (std::getline(stream_line, value, delimeter)) {
            values.push_back(value);
        }

        size_t index = sizeof...(Args);
        result.push_back(std::make_tuple( convert_to<Args>(values[--index])...) );

    }

    return result;

}