#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <functional>
#include <iomanip> // Pour std::fixed et std::setprecision

class Benchmark {
public:
    template<typename Func, typename... Args>
    static void measure(const std::string& name, Func&& func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::forward<Func>(func)(std::forward<Args>(args)...);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        double milliseconds = microseconds / 1000.0;
        
        std::cout << name << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds" << std::endl;
    }
};

#define BENCHMARK_FUNCTION(func, ...) \
    Benchmark::measure(#func, func, __VA_ARGS__)

class BenchmarkGuard {
public:
    BenchmarkGuard(const std::string& name) 
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
    
    ~BenchmarkGuard() {
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        double milliseconds = microseconds / 1000.0;
        
        std::cout << name_ << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds" << std::endl;
    }
private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

#define BENCHMARK_BLOCK(name) \
    BenchmarkGuard benchmark_guard##__LINE__(name)
