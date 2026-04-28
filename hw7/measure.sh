g++ -O3 main.cpp -o matrix_sum
perf stat \
  -e task-clock,cycles,instructions,cache-references,cache-misses,branches,branch-misses \
  ./matrix_sum rows 10000

perf stat \
  -e task-clock,cycles,instructions,cache-references,cache-misses,branches,branch-misses \
  ./matrix_sum cols 10000


perf stat \
  -e task-clock,cycles,instructions,cache-references,cache-misses,branches,branch-misses \
  ./matrix_sum cols_opt 10000
#perf stat ./matrix_sum cols_opt 4096 30
