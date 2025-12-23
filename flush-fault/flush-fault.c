#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <sched.h>

size_t start, end;
//size_t start_instr, end_instr, delta_instr;
size_t delta, idx = 0;

// ! 这个受害者函数为什么会是受害者？哪个指令导致他的危险？
// unimp 是 非法指令
// 执行它会触发 SIGILL（非法指令异常）
// 这是可控 fault 触发点
void victim() {
    // printf("victim\n");
    asm volatile ("j l1\n\t"
                  "nop\n\t"
                  "nop\n\t"
                  "nop\n\t"
                  "nop\n\t"
                  "nop\n\t"
                  "nop\n\t"
                  "unimp\n\t"
                  "unimp\n\t"
                  "unimp\n\t"
                  "nop\n\t"
                  "l1:nop\n\t");
    // printf("%d\n", idx++);
}

static jmp_buf trycatch_buf;
// ! 这个函数的作用是什么？
// 当一个信号正在被处理时，它会被自动阻塞;内核会 屏蔽 SIGILL;防止 handler 递归触发
void unblock_signal(int signum __attribute__((__unused__))) {
  sigset_t sigs;
  sigemptyset(&sigs);     // 创建一个空的信号集合 
  sigaddset(&sigs, signum); // 将指定的信号加入集合
  // 手动解除 SIGILL / SIGSEGV 的屏蔽；允许 下一次 fault 继续被 handler 捕获
  sigprocmask(SIG_UNBLOCK, &sigs, NULL);  // 解除该信号的屏蔽
}

static inline uint64_t rdtsc() {
  uint64_t val;
  asm volatile ("rdcycle %0" : "=r"(val));
  //asm volatile("nop");
  return val;
}

static inline size_t rdinstret() {
  size_t val;
  asm volatile("rdinstret %0" : "=r"(val));
  return val;
}

void trycatch_segfault_handler(int signum) {
  // printf("trycatch_segfault_handler_start\n");
  (void)signum;
  end = rdtsc();
  //end_instr = rdinstret();
  if (start) {
    delta = end - start;
    //delta_instr = end_instr - start_instr;
  }
  // ! 为什么这里要使用signal以及longjmp？longjmp是什么？
  // SIGSEGV 是“段错误”，通常是由于访问非法内存引起的。通过解除屏蔽，可以让信号处理函数处理这个信号。
  unblock_signal(SIGSEGV);
  // 如果不注册 handler → 程序直接退出
  unblock_signal(SIGILL);
  // longjmp：从异常中“跳回去继续执行”;直接回到之前的 setjmp(buf)
  // 异常发生在 汇编跳转后的任意位置
  // 没有正常 return 路径
  // 唯一办法是 非局部跳转
  longjmp(trycatch_buf, 1);
  // printf("trycatch_segfault_handler_end\n");
}

int flush_reload_t(void *ptr) {
  start = 0;
  end = 0;

  // ! setjmp是什么？为什么要sched_yield？下面的汇编代码是在做什么？
  // 第一次返回 0;longjmp 回来返回 1
  if (!setjmp(trycatch_buf)) {
    // 让调度器：清空 pipeline;减少前一次执行对 cache 的影响
    sched_yield();
    //start_instr = rdinstret();
    start = rdtsc();
    // 清空寄存器（防止依赖）;jr ptr：直接跳到 victim+offset;执行 unimp → SIGILL
    // memory：内存状态不可信（防止重排）
    asm volatile ("xor a5, a5, a5\n\t"
                  "xor a0, a0, a0\n\t"
                  "jr %0 \n\t" :: "r"(ptr) : "memory", "a0", "a5");
    printf("WE SHOULD NEVER SEE THIS\n");
  }
  return (int)(end - start);
}

static inline void flush(void* addr) {
  // ! 这样就可以flush？这里flush的是icache吗？
  // 保证后续取指重新从内存获取;使 I-cache 失效或至少失去一致性
  // 防止编译器优化;确保 fence.i 与该地址存在依赖
  asm volatile("xor a7, a7, a7\n"
               "add a7, a7, %0\n"
               "fence.i\n\t"
  : : "r"(addr) : "a7","memory");
}


void dummy() {
  asm volatile("nop");
}

typedef void (*fnc)();
int main(int argc, char* argv[]) {
  start = 0;
  end = 0;
  // victim 内部的 unimp
  char* ptr = (char*)((size_t)victim + 14);

  // ! 这里是什么也是?捕获非法指令异常
  signal(SIGILL, trycatch_segfault_handler);

  FILE* fd = fopen("./flush-fault.csv", "w");
  for (size_t j = 0; j < 1000; j++) {
    for (size_t i = 0; i < 100; i++) {
      int cached = i % 2 == 0;
      // ! 这里是用函数指针来表示两个函数中的一个函数会执行吧？
      // cached=1 → 执行 victim() → I-cache 热;cached=0 → 执行 dummy() → victim 冷
      fnc target = (fnc) (cached * (size_t) victim + (1 - cached) * (size_t) dummy);
      // printf("%s\n", cached?"victim":"dummy");
      target();

      size_t value = flush_reload_t(ptr);
      fprintf(fd, "%zu,%d,%zu\n", i, cached, value);
      if (j > 20) {
        fprintf(fd, "%zu,%d,%zu\n", i, cached, value);
      }
      flush(ptr);
    }
    // if(j == 1) break;
  }

  fclose(fd);
  return 0;
}
