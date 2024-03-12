/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "pmm.h"
#include "vmm.h"
#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert( current );
  char* pa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)buf);
  sprint(pa);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page(uint64 size) {
  heap_management* heap_management = current->heap_management;
  for(uint64 i = 0; i < heap_management->num; ++i)
  {
    if(heap_management->blk_array[i].size >= size && heap_management->blk_array[i].valid == 1)
      return heap_management->blk_array[i].addr;
  }
  uint64 addr = heap_management->h_end;
  for(uint64 a = PGROUNDUP(addr); a < addr + size; a += PGSIZE)
  {
    uint64 pa = (uint64)alloc_page();
    if((void*)pa == NULL){
      panic("uvmalloc: out of memory\n");
    }
    memset((void*)pa, 0, PGSIZE);
    if(map_pages(current->pagetable, a, PGSIZE, (uint64)pa, prot_to_type(PROT_READ | PROT_WRITE, 1)) != 0){
      panic("map error\n");
    }
  }
  heap_management->h_end += size;
  heap_management->blk_array[heap_management->num].addr = addr;
  heap_management->blk_array[heap_management->num].size = size;
  heap_management->blk_array[heap_management->num].valid = 0; 
  heap_management->num++;
  // void* pa = alloc_page();
  // uint64 va = g_ufree_page;
  // g_ufree_page += PGSIZE;
  // user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
  //        prot_to_type(PROT_WRITE | PROT_READ, 1));

  return addr;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  for(int i = 0; i < current->heap_management->num; i++)
  {
    if(current->heap_management->blk_array[i].addr == (uint64)va)
    {
      // sprint("free addr: 0x%lx\n", current->heap[i]->addr);
      current->heap_management->blk_array[i].valid = 1;
      return 0;
    }
  }
  panic("free error\n"); 
  // user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
  // return 0;
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    // added @lab2_2
    case SYS_user_allocate_page:
      return sys_user_allocate_page(a1);
    case SYS_user_free_page:
      return sys_user_free_page(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
