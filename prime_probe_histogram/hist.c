#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../rlibsc.h" 

#define HISTOGRAM_ENTRIES 1500
#define HISTOGRAM_SCALE 10
#define MEASUREMENTS 1000

#define LOGFILE "histogram.csv"

char __attribute__((aligned(4096))) buffer[2 * 1024 * 1024];

// Cache line size：64B；Offset：[5:0]（64B）
#ifdef C906
static inline void prime(void *addr) {
  // TODO：这里是获取哪几位？prime的程序逻辑是什么？
  // & 63：取 6 位 → 64 个 set；pset = cache set index（假设 64 sets）
  size_t pset = (((size_t)addr) >> 6) & 63;
  // 取 bit 12（页内偏移之外的最低一位页号信息）
  int pn = ((size_t)addr) & 4096;
  int bufpn = ((size_t)buffer) & 4096;
  int i = 1;
  // 稳定地构造“不同虚拟页，但相同 cache set”的 eviction set
  int j = (pn == bufpn) ? 0 : 1;
  // (pset << 6)-固定 cache set；offset 到同一个 set，不同 line
  // i++ * 2 * 4096；每次跳 2 个 page；保证不同 cache line / way
  // 控制 page parity / bank / slice
  REP4(maccess(buffer + i++ * 2 * 4096 + j * 4096 + (pset << 6));)
}
#endif

// prime中访问的是buffer
size_t measure_access_time(void *address) {
  uint64_t x = rdcycle();
  prime(address);
  uint64_t y = rdcycle();
  return y - x;
}

// prime函数
void measure_hits(void *address, size_t *histogram,
                  size_t number_of_measurements) {
  for (size_t i = 0; i < number_of_measurements; i++) {
    size_t hit = measure_access_time(address);
    if (hit < HISTOGRAM_ENTRIES)
      histogram[hit]++;
  }
}

// probe时先访问address数组，会驱逐cache中buffer的位置
void measure_misses(void *address, size_t *histogram,
                    size_t number_of_measurements) {
  for (size_t i = 0; i < number_of_measurements; i++) {
    maccess(address);
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

  // 会把这一页里的 每一个字节都设为 1；确保页被真实分配（避免 COW / lazy allocation）
  // 防止零页优化；确保访问不会触发 page fault
  memset(address, 1, 4096);
  memset(buffer, 2, sizeof(buffer));

  measure_hits(address, hit_histogram, MEASUREMENTS);
  measure_misses(address, miss_histogram, MEASUREMENTS);

  // 每10间隔测量一次
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
