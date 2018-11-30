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

  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));

  for (i = 0; i < 64; i++) { 
    if (shm_table.shm_pages[i].id == id) {
      uint pa = (uint)&shm_table.shm_pages[i].frame;
      uint va = PGROUNDUP(myproc()->sz);

      if (mappages(myproc()->pgdir, (void *)va, PGSIZE, pa, PTE_W|PTE_U) == 0) {
        goto bad;
      }

      shm_table.shm_pages[i].refcnt += 1;
      *pointer = (char *)va;
      myproc()->sz += PGSIZE;
    }
    else { // shared memory segment does not exist
      if (!shm_table.shm_pages[i].frame) { // if we find an empty entry
        shm_table.shm_pages[i].id = id;
        char * ka = kmalloc(PGSIZE, GFP_KERNEL);

        if (!ka) {
          goto bad;
        }
        shm_table.shm_pages[i].frame = ka;
        shm_table.shm_pages[i].refcnt = 1;

        uint pa = (uint)&shm_table.shm_pages[i].frame;
        uint va = PGROUNDUP(myproc()->sz);

        if (mappages(myproc()->pgdir, (void *)va, PGSIZE, pa, PTE_W|PTE_U) == 0) {
          goto bad;
        }

      }
    }
  }

  release(&(shm_table.lock));
  
  bad:
    exit();
  
  return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!




return 0; //added to remove compiler warning -- you should decide what to return
}
