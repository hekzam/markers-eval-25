#include "../include/benchmark.hpp"
#include <vector>
#include <algorithm>
#include <random>
#include <memory>
#include <cstdlib>

// Exemple 1: Fonction de tri
std::vector<int> bubble_sort(std::vector<int> vec) {
    for(size_t i = 0; i < vec.size(); i++) {
        for(size_t j = 0; j < vec.size() - 1; j++) {
            if(vec[j] > vec[j + 1]) {
                std::swap(vec[j], vec[j + 1]);
            }
        }
    }
    return vec;
}

// Exemple 2: Fonction avec paramètres
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

int main(void) {  // Définition explicite de main
    // Préparation des données
    std::vector<int> data(1000);
    std::srand(static_cast<unsigned int>(std::time(nullptr)));  // Initialisation du générateur
    for(int i = 0; i < 1000; i++) {
        data[i] = std::rand() % 1000;
    }

    // Exemple 1: Benchmark d'une fonction de tri
    std::cout << "===== Comparaison des algorithmes de tri =====" << std::endl;
    
    // Test du bubble sort
    auto data_copy = data;
    BENCHMARK_FUNCTION(bubble_sort, data_copy);
    
    // Test du std::sort
    data_copy = data;
    Benchmark::measure("std::sort", [&data_copy]() {
        std::sort(data_copy.begin(), data_copy.end());
    });

    // Exemple 2: Benchmark avec bloc de code
    std::cout << "\n===== Test avec bloc de code =====" << std::endl;
    {
        BENCHMARK_BLOCK("Fibonacci(30)");
        fibonacci(30);
    }

    // Exemple 3: Benchmark avec différents paramètres
    std::cout << "\n===== Test avec différents paramètres =====" << std::endl;
    BENCHMARK_FUNCTION(fibonacci, 25);
    BENCHMARK_FUNCTION(fibonacci, 30);
    BENCHMARK_FUNCTION(fibonacci, 35);

    return 0;
}
