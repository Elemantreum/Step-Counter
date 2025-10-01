#include <lib/csv_parser.h>
#include <lib/arg_parser.h>
#include <iostream>


enum DataColumns {
    Time,
    Acceleration_X,
    Acceleration_Y,
    Acceleration_Z,
    Absolute_Acceleration
};

template<typename... Args>
size_t CountSteps(const std::vector<std::tuple<Args...>>& data, double threshold, double min_delta) {
    std::decay_t<decltype(data)> peaks;

    for (size_t row = 2; row < data.size() - 1; row++) {
        double time = std::get<Time>(data[row]);
        double acceleration_y = std::get<Acceleration_Y>(data[row]);

        if     (   acceleration_y  > std::get<Acceleration_Y>(data[row - 1])
                && acceleration_y  < std::get<Acceleration_Y>(data[row + 1])
                && acceleration_y  > threshold) {

            if (peaks.empty()){
                peaks.push_back(data[row]);
                continue;
            }

            if (time - std::get<Time>(peaks.back()) >= min_delta) {
                peaks.push_back(data[row]);
            }

        }

        
    }

    return peaks.size();

}

struct StepCounter {

    double threshold;
    double min_delta;
    DataColumns axis;

    StepCounter(double threshold, double min_delta, int axis_column) 
        : threshold(threshold)
        , min_delta(min_delta)
        , axis(static_cast<DataColumns>(axis_column))
    {}

    template<typename... Args>
    size_t Count(std::vector<std::tuple<Args...>> data) {
        std::decay_t<decltype(data)> peaks;

        for (size_t row = 2; row < data.size() - 1; row++) {
            
            if(CheckPeak(data, row, axis)) {
                
                double curr_time = std::get<Time>(data[row]);

                if (peaks.empty()){
                    peaks.push_back(data[row]);
                } else if (curr_time - std::get<Time>(peaks.back()) >= min_delta) {
                    peaks.push_back(data[row]);
                }
                
            }
            
        }

        return peaks.size();
    }

private:

    template<size_t Column, typename... Args>
    bool CheckPeak(const std::vector<std::tuple<Args...>>& data, size_t row, size_t target_axis) {
        if (Column != target_axis) return false;
        
        double current = std::get<Column>(data[row]);
        double prev = std::get<Column>(data[row - 1]);
        double next = std::get<Column>(data[row + 1]);
        
        return (current > prev) && (current < next) && (current > threshold);
    }

    // Специализация для проверки всех колонок
    template<size_t... Is, typename... Args>
    bool CheckPeak(const std::vector<std::tuple<Args...>>& data, size_t row, size_t target_axis, std::index_sequence<Is...>) {
        return (CheckPeak<Is>(data, row, target_axis) || ...);
    }
    
    template<typename... Args>
    bool CheckPeak(const std::vector<std::tuple<Args...>>& data, size_t row, size_t target_axis) {
        return CheckPeak(data, row, target_axis, std::index_sequence_for<Args...>{});
    }

};

int main(int argc, char** argv) {

    ArgumentParser::ArgParser arg_parser("Step Counter Argument Parser");
    arg_parser.AddArgument<std::string>('c', "csv", "Path to .csv file with raw data. Positional argument").Positional();
    arg_parser.AddArgument<char>('d', "delimeter", "delimeter in .csv file. WORK IN PROGRESS!!! DON'T TOUCH IT");
    arg_parser.AddArgument<double>('t', "threshold", "Minimum acceleration need to count a step. Default = 11").Default(11);
    arg_parser.AddArgument<double>("min-delta", "Minimum time delta beetwen steps. (need to filter low peeks). Default = 0.5").Default(0.5);
    arg_parser.AddArgument<int>('a', "axis", "Num of axis to parse.");

    arg_parser.AddHelp('h', "help", "Programm to parse your accelerometr in phone to count you steps.\n\nWarning!\nCSV File with tabulation needed!\nParses only Y axis. Either recompile program, either change threshold and min-delta params.");

    if (!arg_parser.Parse(argc, argv)) {
        throw std::invalid_argument("Invalid arguments");
    };

    if (arg_parser.Help()) {
        std::cout << arg_parser.HelpDescription() << std::endl;

        return 0;
    }

    auto data = ParseCSV<double, double, double, double, double>(arg_parser.Get<std::string>("csv"), arg_parser.Get<char>("delimeter"));
    StepCounter counter(arg_parser.Get<double>("threshold"), arg_parser.Get<double>("min-delta"), arg_parser.Get<int>("axis"));

    std::cout << counter.Count(data);

    return 0;
}