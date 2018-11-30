#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
  int i = 0;
  int tempPageNum = -1;

  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  
  // Loop through the table and find page table that matches
  for (i = 0; i < 64; i++) {
    if (shm_table.shm_pages[i].id == id) {
      tempPageNum = i;
      break;
    }
  }

  if (tempPageNum >= 0) {
    char * pa = shm_table.shm_pages[tempPageNum].frame;
    uint va = PGROUNDUP(myproc()->sz);

    if (mappages(myproc()->pgdir, (void *)va, PGSIZE, V2P(pa), PTE_W|PTE_U) < 0) {
      goto bad;
    }

    shm_table.shm_pages[tempPageNum].refcnt += 1;
    *pointer = (char *)va;
    myproc()->sz += PGSIZE;
  }
  else {
    for (i = 0; i < 64; i++) {
      if (shm_table.shm_pages[i].id == 0) { // if we find an empty entry
          shm_table.shm_pages[i].id = id;

          char * mem = kalloc();
          if (!mem) {
            goto bad;
          }
          memset(mem, 0, PGSIZE);

          shm_table.shm_pages[i].frame = mem;
          shm_table.shm_pages[i].refcnt = 1;

          char * pa = shm_table.shm_pages[i].frame;
          uint va = PGROUNDUP(myproc()->sz);

          if (mappages(myproc()->pgdir, (void *)va, PGSIZE, V2P(pa), PTE_W|PTE_U) < 0) {
            goto bad;
          }

        *pointer = (char *)va;
        myproc()->sz += PGSIZE;
        break;
      }
    }    
  }
  
  bad:
    release(&(shm_table.lock));
 
  return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
  cprintf("start close\n");
  
  int i = 0;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));

  for (i = 0; i < 64; i++){
    if (shm_table.shm_pages[i].id == id){
      shm_table.shm_pages[i].refcnt -= 1;
      if (shm_table.shm_pages[i].refcnt == 0){
        shm_table.shm_pages[i].id = 0;
	shm_table.shm_pages[i].frame = 0;	
      }
    }
  }

 release(&(shm_table.lock));

 cprintf("end close\n");

return 0; //added to remove compiler warning -- you should decide what to return
}
