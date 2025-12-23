#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CT_REP 10

#define LOGFILE "timings.csv"

#define REP16(x) x x x x x x x x x x x x x x x x
#define REP256(x) \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)        \
  REP16(x)
#define REP1K(x) REP256(x) REP256(x) REP256(x) REP256(x)

#define MERGE_(a, b) a##b
#define LABEL_(a) SCT_MERGE_(measure_, a)
#define UNIQUE_NAME LABEL_(__LINE__)

uint64_t overhead = 0;

#if defined(__x86_64__)
static inline uint64_t rdcycle()
{
  uint64_t a, d;
  asm volatile("mfence");
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  a = (d << 32) | a;
  asm volatile("mfence");
  return a;
}
#else
static inline uint64_t rdcycle()
{
  uint64_t val;
  asm volatile("rdcycle %0" : "=r"(val));
  return val;
}
#endif

// 这个是将什么定义成什么？
// measure_func_t类型是一个函数指针类型，返回值是uint64_t
typedef uint64_t (*measure_func_t)(void);

typedef struct
{
  measure_func_t fnc; // 测量函数
  const char *name;   // 指令名
  int cycles;         // 测出来的 cycles / instr
  int ct;             // 是否常数时间（1 = 是）
} function_info_t;

function_info_t functions[1000];
int function_count = 0;

// 所有 MEASURE(...) 自动执行
// 为什么红定义里使用instr需要加上#？__attribute__((constructor))是函数构造时自动执行吗？
// 这里会连续定义两个函数，一个measure_#instr，紧接着一个自动注册函数
// #instr 使用的是 C 预处理器的“字符串化（stringify）”操作符。
#define MEASURE(instr, init, code, flags)                          \
  uint64_t MERGE_(measure_, instr)()                               \
  {                                                                \
    uint64_t start = rdcycle();                                    \
    asm volatile(init "; fence; " REP1K(code) "; fence;" flags);   \
    return rdcycle() - start;                                      \
  }                                                                \
  void __attribute__((constructor)) MERGE_(ctor_measure_, instr)() \
  {                                                                \
    functions[function_count].fnc = MERGE_(measure_, instr);       \
    functions[function_count].name = #instr;                       \
    functions[function_count].cycles = -1;                         \
    functions[function_count].ct = 1;                              \
    function_count++;                                              \
  }

// 这是什么意思？为什么里面会有MERGE_(measure_, fnc)()，这是一个空函数吗？
// 不是一个空函数，会生成一个base_measurement函数，用来测量测量框架的时间
#define BASE(fnc)                                      \
  void __attribute__((constructor)) base_measurement() \
  {                                                    \
    MERGE_(measure_, fnc)();                           \
    overhead = MERGE_(measure_, fnc)();                \
  }

// MEASURE(warmup, "", "", "")
// base是什么？
// 整个测量框架需要执行的时间，里面不含任何指令，nop也不含的时候的耗时
MEASURE(base, "", "", "")

MEASURE(nop, "", "nop;", "")
MEASURE(mv, "", "mv a0, a1;", :: : "a0")
MEASURE(add, "", "add a0, a1, a2;", :: : "a0")
MEASURE(add_s, "", "add a1, a1, a1;", :: : "a1")
// MEASURE(sub, "", "sub a0, a1, a2;", ::: "a0")
// MEASURE(subw, "", "subw a0, a1, a2;", ::: "a0")
// MEASURE(addi, "", "addi a0, a1, 1337;", ::: "a0")
// MEASURE(addiw, "", "addiw a0, a1, 1337;", ::: "a0")
// MEASURE(addw, "", "addw a0, a1, 1337;", ::: "a0")
// MEASURE(slt, "", "slt a0, a1, a2;", ::: "a0")
// MEASURE(slti, "", "slti a0, a1, 1337;", ::: "a0")
// MEASURE(sltu, "", "sltu a0, a1, a2;", ::: "a0")
// MEASURE(sltiu, "", "sltiu a0, a1, 1337;", ::: "a0")
// MEASURE(lui, "", "lui a0, 1337;", ::: "a0")
// // MEASURE(auip, "", "auip a0, 1337;", ::: "a0")

// MEASURE(and, "", "and a0, a1, a2;", ::: "a0")
// MEASURE(xor, "", "xor a0, a1, a2;", ::: "a0")
// MEASURE(or, "", "or a0, a1, a2;", ::: "a0")
// MEASURE(andi, "", "andi a0, a1, 1337;", ::: "a0")
// MEASURE(xori, "", "xori a0, a1, 1337;", ::: "a0")
// MEASURE(ori, "", "ori a0, a1, 1337;", ::: "a0")
// MEASURE(sll, "", "sll a0, a1, a2;", ::: "a0")
// MEASURE(srl, "", "srl a0, a1, a2;", ::: "a0")
// MEASURE(slli, "", "slli a0, a1, 15;", ::: "a0")
// MEASURE(slliw, "", "slliw a0, a1, 15;", ::: "a0")
// MEASURE(sllw, "", "sllw a0, a1, 15;", ::: "a0")
// MEASURE(srli, "", "srli a0, a1, 15;", ::: "a0")
// MEASURE(srliw, "", "srliw a0, a1, 15;", ::: "a0")
// MEASURE(srlw, "", "srlw a0, a1, a2;", ::: "a0")
// MEASURE(srai, "", "srai a0, a1, 15;", ::: "a0")
// MEASURE(sraiw, "", "sraiw a0, a1, 15;", ::: "a0")
// MEASURE(sra, "", "sra a0, a1, a2;", ::: "a0")
// MEASURE(sraw, "", "sraw a0, a1, a2;", ::: "a0")

// // MEASURE(csrrci, "", "csrrci a0, 0, 0;", ::: "a0")
// MEASURE(fence, "", "fence iorw,iorw;", ::: "a0")
// MEASURE(fencei, "", "fence.i;", ::: "a0")

// // MEASURE(wfi, "", "wfi;", ::: "a0")

// MEASURE(div, "", "div a0, a1, a2;", ::: "a0")
// MEASURE(divu, "", "divu a0, a1, a2;", ::: "a0")
// MEASURE(divuw, "", "divuw a0, a1, a2;", ::: "a0")
// MEASURE(divw, "", "divw a0, a1, a2;", ::: "a0")

// MEASURE(mul, "", "mul a0, a1, a2;", ::: "a0")
// MEASURE(mulh, "", "mulh a0, a1, a2;", ::: "a0")
// MEASURE(mulhsu, "", "mulhsu a0, a1, a2;", ::: "a0")
// MEASURE(mulhu, "", "mulhu a0, a1, a2;", ::: "a0")
// MEASURE(mulw, "", "mulw a0, a1, a2;", ::: "a0")

// MEASURE(rem, "", "rem a0, a1, a2;", ::: "a0")
// MEASURE(remu, "", "remu a0, a1, a2;", ::: "a0")
// MEASURE(remuw, "", "remuw a0, a1, a2;", ::: "a0")
// MEASURE(remw, "", "remw a0, a1, a2;", ::: "a0")

// MEASURE(fadds, "", "fadd.s f0, f1, f2;", ::: "f0")
// MEASURE(fclasss, "", "fclass.s a0, f0;", ::: "f0")
// MEASURE(fdivs, "", "fdiv.s f0, f1, f2;", ::: "f0")
// MEASURE(feqs, "", "feq.s a0, f1, f2;", ::: "a0")

// MEASURE(cadd, "", "c.add a0, a1;", ::: "a0")
// MEASURE(caddi, "", "c.addi a0, 15;", ::: "a0")
// MEASURE(caddiw, "", "c.addiw a0, 15;", ::: "a0")
// MEASURE(caddw, "", "c.addw a0, a1;", ::: "a0")
// MEASURE(cand, "", "c.and a0, a1;", ::: "a0")
// MEASURE(candi, "", "c.andi a0, 15;", ::: "a0")
// MEASURE(cli, "", "c.li a0, 15;", ::: "a0")
// MEASURE(clui, "", "c.lui a0, 15;", ::: "a0")
// MEASURE(cmv, "", "c.mv a0, a1;", ::: "a0")
// MEASURE(cnop, "", "c.nop;", "")
// MEASURE(cor, "", "c.or a0, a1;", ::: "a0")
// MEASURE(cslli, "", "c.slli a0, 3;", ::: "a0")
// MEASURE(csrai, "", "c.srai a0, 3;", ::: "a0")
// MEASURE(csrli, "", "c.srli a0, 3;", ::: "a0")
// MEASURE(csub, "", "c.sub a0, a1;", ::: "a0")
// MEASURE(csubw, "", "c.subw a0, a1;", ::: "a0")
// MEASURE(cxor, "", "c.xor a0, a1;", ::: "a0")

BASE(base)

int main(int argc, char *argv[])
{
  FILE *logfile = fopen(LOGFILE, "w+");
  // NULL被展开成(void *)0
  if (logfile == NULL)
  {
    fprintf(stderr, "Error: Could not open logfile: %s\n", LOGFILE);
    return -1;
  }

  fprintf(logfile, "Instr,Time\n");
  // 为什么要srand？为什么一定要播种随机数？
  // 在你这段代码里，rand() 不是用来产生“随机值”，而是用来：打破指令执行时的“输入相关性”和“微架构状态相关性”
  // 保证每次程序运行时，后续注入的随机扰动不同
  srand(time(NULL));

  for (int rep = 0; rep < CT_REP; rep++)
  {
    for (int i = 0; i < function_count; i++)
    {
      functions[i].fnc();
      // 为什么需要加上这两行？
      // 在每一次测量之间，强制污染（clobber）参数寄存器
      // a0–a7	参数 / 返回值寄存器
      // a1, a2	常用作函数参数
      // a0 通常用作返回值（measure_*() 的返回）
      // a1 / a2：
      //   常见参数寄存器
      //   影响执行资源
      //   又不破坏返回值
      asm volatile("mv a1, %0" : : "r"(rand()));
      asm volatile("mv a2, %0" : : "r"(rand()));

      uint64_t cycles = ((functions[i].fnc() - overhead) + 1023) / 1024;
      if (functions[i].cycles != -1 && functions[i].cycles != cycles)
      {
        functions[i].ct = 0;
        printf("%s: %d != %zd\n", functions[i].name,
               functions[i].cycles, cycles);
      }
      functions[i].cycles = cycles;
    }
  }

  for (int i = 0; i < function_count; i++)
  {
    printf("%-8s: %3zd %s\n", functions[i].name, (long)functions[i].cycles,
           functions[i].ct ? "" : "! not CT !");
    fprintf(logfile, "%s,%zd\n", functions[i].name, (long)functions[i].cycles);
  }

  fclose(logfile);

  return 0;
}
