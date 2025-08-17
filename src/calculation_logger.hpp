// Mostly written by Claude AI
#ifndef CALCULATION_LOGGER_HPP
#define CALCULATION_LOGGER_HPP

#include "ring_buffer_queue.hpp"
#include <atomic>
#include <fstream>
#include <string>

class calculation_logger {
private:
    ring_buffer_calculation_queue& log_queue_;
    std::atomic<bool>& calculation_done_;
    std::string log_file_name_;
    std::ofstream log_file_;
    
    void write_to_console(const std::string& message);
    void write_to_file(const std::string& message);

public:
    calculation_logger(ring_buffer_calculation_queue& log_queue, std::atomic<bool>& calculation_done, 
           const std::string& log_file_name = "");
    ~calculation_logger();
    
    void progress_calculation_logger();
};

#endif