#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <functional>
#include <iomanip> // Pour std::fixed et std::setprecision
#include <fstream>

class Benchmark {
  public:
    template <typename Func, typename... Args>
    static void measure(const std::string& name, Func&& func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();

        std::forward<Func>(func)(std::forward<Args>(args)...);

        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        double milliseconds = microseconds / 1000.0;

        std::cout << name << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds" << std::endl;
        return;
    }
};

#define BENCHMARK_FUNCTION(func, ...) Benchmark::measure(#func, func, __VA_ARGS__)

class BenchmarkGuard {
  public:
    BenchmarkGuard(const std::string& name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {
    }

    ~BenchmarkGuard() {
        auto end = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        double milliseconds = microseconds / 1000.0;

        std::cout << name_ << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds"
                  << std::endl;
    }

  private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

#define BENCHMARK_BLOCK(name) BenchmarkGuard benchmark_guard##__LINE__(name)
class BenchmarkGuardCSV {
    public:
      BenchmarkGuardCSV(const std::string& name, std::ofstream* csv)
        : name_(name), csv_(csv),
          start_(std::chrono::high_resolution_clock::now()) {
      }
  
      ~BenchmarkGuardCSV() {
          auto end = std::chrono::high_resolution_clock::now();
          auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
          double milliseconds = microseconds / 1000.0;
  
          std::cout << name_ << ": " << std::fixed << std::setprecision(3) << milliseconds << " milliseconds" << std::endl;
          if(csv_) {
              *csv_ << name_ << "," << std::fixed << std::setprecision(3) << milliseconds << std::endl;
          }
      }
  
    private:
      std::string name_;
      std::chrono::time_point<std::chrono::high_resolution_clock> start_;
      std::ofstream* csv_;
};
  
#define BENCHMARK_BLOCK_CSV(name, csv_ptr) BenchmarkGuardCSV benchmark_guard_csv##__LINE__(name, csv_ptr)
