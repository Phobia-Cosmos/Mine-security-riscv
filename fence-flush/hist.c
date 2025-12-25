#include "../rlibsc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// 超过 200 cycle 的直接丢弃（不关心 tail）
#define HISTOGRAM_ENTRIES 200
#define HISTOGRAM_SCALE 1
#define MEASUREMENTS 1000

#define LOGFILE "histogram.csv"

// 没有函数序言 / 尾声
// 没有：
//   addi sp, sp, -xx
//   sd ra, ...
//   ld ra, ...
// 一个ret占一个Byte？为什么要这么多的ret？
// ret（RISC-V 为 jalr x0, x1, 0）不是 1 字节，是 4 字节定长指令
// 生成一定规模的代码 footprint，用来填满 I-cache
// 每条 ret 会被 CPU 取指、预测返回、执行
// 代码区域占用对齐后的 4KB，使其精准映射一个 I-cache 页。
void __attribute__((aligned(4096), naked)) victim()
{
  REP4K(asm volatile("ret");)
}

// typedef function pointer？这是什么语法？
// victim_t 表示：void f(void);
// 一个无参无返回的函数指针类型
typedef void (*victim_t)(void);

// TODO：为什么这里的测量又是要执行v这个函数才可以测量？
// 因为你测的是 指令访问时间（I-Cache） 或 BTB/RSB 预测时间，而不是数据访问。
size_t measure_access_time(void *address)
{
  // 地址强转为函数指针
  victim_t v = (victim_t)address;
  uint64_t x = rdcycle();
  v();
  uint64_t y = rdcycle();
  return y - x;
}

void measure_hits(void *address, size_t *histogram,
                  size_t number_of_measurements)
{
  for (size_t i = 0; i < number_of_measurements; i++)
  {
    size_t hit = measure_access_time(address);
    if (hit < HISTOGRAM_ENTRIES)
      histogram[hit]++;
  }
}

void measure_misses(void *address, size_t *histogram,
                    size_t number_of_measurements)
{
  for (size_t i = 0; i < number_of_measurements; i++)
  {
    // fencei(address);
    fencei();
    size_t miss = measure_access_time(address);
    if (miss < HISTOGRAM_ENTRIES)
      histogram[miss]++;
  }
}

size_t hit_histogram[HISTOGRAM_ENTRIES], miss_histogram[HISTOGRAM_ENTRIES];

int main(int argc, char *argv[])
{
  FILE *logfile = fopen(LOGFILE, "w+");
  if (logfile == NULL)
  {
    fprintf(stderr, "Error: Could not open logfile: %s\n", LOGFILE);
    return -1;
  }

  fprintf(logfile, "Time,Hit,Miss\n");

  victim();

  measure_hits(victim, hit_histogram, MEASUREMENTS);
  measure_misses(victim, miss_histogram, MEASUREMENTS);

  for (size_t i = 0; i < HISTOGRAM_ENTRIES; i += HISTOGRAM_SCALE)
  {
    size_t hit = 0, miss = 0;
    for (size_t scale = 0; scale < HISTOGRAM_SCALE; scale++)
    {
      hit += hit_histogram[i + scale];
      miss += miss_histogram[i + scale];
    }
    fprintf(stdout, "%4zu: %10zu %10zu\n", i, hit, miss);
    if (logfile != NULL)
    {
      fprintf(logfile, "%zu,%zu,%zu\n", i, hit, miss);
    }
  }

  fclose(logfile);
  return 0;
}
