// Mostly written by Claude AI
#include "calculation_logger.hpp"
#include "ring_buffer_queue.hpp"

#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

using std::to_string;
using std::endl;

//TODO maybe make 2 instantiations for excess(p) and q_set(p)
calculation_logger::calculation_logger(ring_buffer_calculation_queue& log_queue, std::atomic<bool>& calculation_done, 
               const std::string& log_file_name)
    : log_queue_(log_queue), calculation_done_(calculation_done), log_file_name_(log_file_name) {
    
    if (!log_file_name_.empty()) {
        log_file_.open(log_file_name_, std::ios::out | std::ios::app);
        if (!log_file_.is_open()) {
            std::cerr << "Warning: Could not open log file: " << log_file_name_ << endl;
        }
    }
}

calculation_logger::~calculation_logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void calculation_logger::write_to_console(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::cout << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S")
              << "." << std::setfill('0') << std::setw(3) << ms.count() 
              << "] " << message << std::endl;
}

void calculation_logger::write_to_file(const std::string& message) {
    if (!log_file_.is_open()) return;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    log_file_ << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
             << "] " << message << std::endl;
    log_file_.flush(); // Ensure immediate write
}

void calculation_logger::progress_calculation_logger() {
    data_quad result;
    std::time_t checkpoint_time = time(nullptr);
    while (!calculation_done_ || !log_queue_.empty()) {
        if (log_queue_.pop(result)) {
            if (result.index == UNSIGNED_MAX) {
                write_to_console("Logging completed.");
                if (log_file_.is_open()) {
                    write_to_file("Logging completed.");
                }
                break;
            }
            
            if (result.index == 0 || result.index == result.msbnp1 || time(nullptr) - checkpoint_time >= 60) {
                checkpoint_time = time(nullptr);
                std::string message = to_string(result.index) + "/" + to_string(result.msbnp1) + " bits complete ("
                + to_string(((float)result.index) / result.msbnp1) + "); curpow/result has "
                + to_string(result.cur_size) + "/" + to_string(result.res_size) + " terms";
                write_to_console(message);
                write_to_file(message);
            }
        } else {
            // Queue empty, small sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}