#include "../rlibsc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HISTOGRAM_ENTRIES 150
#define HISTOGRAM_SCALE 1
#define MEASUREMENTS 1000

#define LOGFILE "histogram.csv"

// eviction pool 足够大；不会意外复用同一 VPN
char __attribute__((aligned(4096))) buffer[4096 * 32];

// TODO：为什么重复10次就可以清除TLB了？
// 这是经验性 TLB eviction，不是 flush。TLB 是有限容量 + 组相联
// | 层级      | 容量               | 结构        |
// | ------- | ---------------- | --------- |
// | L1 DTLB | 32–64 entries    | 4–8 way   |
// | L2 STLB | 256–1536 entries | set-assoc |
// TLB 按 VPN → set 映射
static inline void evict_tlb(void *addr) {
  int i = 1;
  REP10(maccess(buffer + i++ * 4096);)
}

size_t measure_access_time(void *address) {
  uint64_t x = rdcycle();
  maccess(address);
  uint64_t y = rdcycle();
  return y - x;
}

void measure_hits(void *address, size_t *histogram,
                  size_t number_of_measurements) {
  for (size_t i = 0; i < number_of_measurements; i++) {
    size_t hit = measure_access_time(address);
    if (hit < HISTOGRAM_ENTRIES)
      histogram[hit]++;
  }
}

void measure_misses(void *address, size_t *histogram,
                    size_t number_of_measurements) {
  for (size_t i = 0; i < number_of_measurements; i++) {
    evict_tlb(address);
    size_t miss = measure_access_time(address);
    if (miss < HISTOGRAM_ENTRIES)
      histogram[miss]++;
  }
}

size_t hit_histogram[HISTOGRAM_ENTRIES], miss_histogram[HISTOGRAM_ENTRIES];
char __attribute__((aligned(4096))) address[4096];

int main(int argc, char *argv[]) {
  FILE *logfile = fopen(LOGFILE, "w+");
  if (logfile == NULL) {
    fprintf(stderr, "Error: Could not open logfile: %s\n", LOGFILE);
    return -1;
  }

  fprintf(logfile, "Time,Hit,Miss\n");

  memset(address, 1, 4096);
  memset(buffer, 2, sizeof(buffer));

  measure_hits(address, hit_histogram, MEASUREMENTS);
  measure_misses(address, miss_histogram, MEASUREMENTS);

  for (size_t i = 0; i < HISTOGRAM_ENTRIES; i += HISTOGRAM_SCALE) {
    size_t hit = 0, miss = 0;
    for (size_t scale = 0; scale < HISTOGRAM_SCALE; scale++) {
      hit += hit_histogram[i + scale];
      miss += miss_histogram[i + scale];
    }
    fprintf(stdout, "%4zu: %10zu %10zu\n", i, hit, miss);
    if (logfile != NULL) {
      fprintf(logfile, "%zu,%zu,%zu\n", i, hit, miss);
    }
  }

  fclose(logfile);

  return 0;
}
