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

    StepCounter(double threshold, double min_delta) 
        : threshold(threshold)
        , min_delta(min_delta)
    {}


    template<size_t AxisColumn, typename... Args>
    size_t Count(std::vector<std::tuple<Args...>> data) {
        std::decay_t<decltype(data)> peaks;

        for (size_t row = 2; row < data.size() - 1; row++) {
            double time = std::get<Time>(data[row]);
            double acceleration = std::get<AxisColumn>(data[row]);

            if     (   acceleration  > std::get<AxisColumn>(data[row - 1])
                    && acceleration  < std::get<AxisColumn>(data[row + 1])
                    && acceleration  > threshold) {

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

};

int main(int argc, char** argv) {

    ArgumentParser::ArgParser arg_parser("Step Counter Argument Parser");
    arg_parser.AddArgument<std::string>('c', "csv", "Path to .csv file with raw data. Positional argument").Positional();
    arg_parser.AddArgument<char>('d', "delimeter", "delimeter in .csv file. WORK IN PROGRESS!!! DON'T TOUCH IT").Default('\t');
    arg_parser.AddArgument<double>('t', "threshold", "Minimum acceleration need to count a step").Default(11);
    arg_parser.AddArgument<double>("min-delta", "Minimum time delta beetwen steps. (need to filter low peeks)").Default(0.5);
    arg_parser.AddArgument<int>('a', "axis", "Num of axis to parse. WORK IN PROGRESS!!!");

    arg_parser.AddHelp('h', "help", "Programm to parse your accelerometr in phone to count you steps.\n\nWarning!\nCSV File with tabulation needed!\nParses only Y axis. Either recompile program, either change threshold and min-delta params.");

    arg_parser.Parse(argc, argv);

    if (arg_parser.Help()) {
        std::cout << arg_parser.HelpDescription() << std::endl;

        return 0;
    }

    auto data = ParseCSV<double, double, double, double, double>(arg_parser.Get<std::string>("csv"), arg_parser.Get<char>("delimeter"));
    StepCounter counter(arg_parser.Get<double>("threshold"), arg_parser.Get<double>("min-delta"));

    std::cout << counter.Count<Acceleration_Y>(data);

    return 0;
}