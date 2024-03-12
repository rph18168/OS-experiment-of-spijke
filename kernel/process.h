#ifndef _PROC_H_
#define _PROC_H_

#include "riscv.h"

typedef struct trapframe_t {
  // space to store context (all common registers)
  /* offset:0   */ riscv_regs regs;

  // process's "user kernel" stack
  /* offset:248 */ uint64 kernel_sp;
  // pointer to smode_trap_handler
  /* offset:256 */ uint64 kernel_trap;
  // saved user process counter
  /* offset:264 */ uint64 epc;

  // kernel page table. added @lab2_1
  /* offset:272 */ uint64 kernel_satp;
}trapframe;

// added @lab2_challenge2
typedef struct h_blk_t
{
  uint64 addr;
  uint64 size;
  uint8 valid;
}h_blk;

typedef struct heap_management_t
{
  // 内存块个数
  uint64 num;
  // 堆已分配内存末尾
  uint64 h_end;
  // 堆管理数组
  h_blk blk_array[MAX_HEAP_SIZE];
}heap_management;

// the extremely simple definition of process, used for begining labs of PKE
typedef struct process_t {
  // pointing to the stack used in trap handling.
  uint64 kstack;
  // user page table
  pagetable_t pagetable;
  // trapframe storing the context of a (User mode) process.
  trapframe* trapframe;
  // 堆管理结构
  heap_management* heap_management;
}process;

// switch to run user app
void switch_to(process*);

// current running process
extern process* current;

// address of the first free page in our simple heap. added @lab2_2
extern uint64 g_ufree_page;

#endif
