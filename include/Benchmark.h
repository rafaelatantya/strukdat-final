#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <string>

// fungsi utama buat nge-run benchmark pq vs queue biasa
// scale: jumlah pasien
// outputFilePath: path file buat nyimpen hasil json
void runFullBenchmark(int scale, const std::string& outputFilePath);

#endif // BENCHMARK_H
