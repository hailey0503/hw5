#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void
syscall_exit (int status)
{
  printf ("%s: exit(%d)\n", thread_current ()->name, status);
  thread_exit ();
}

/*
 * This does not check that the buffer consists of only mapped pages; it merely
 * checks the buffer exists entirely below PHYS_BASE.
 */
static void
validate_buffer_in_user_region (const void* buffer, size_t length)
{
  uintptr_t delta = PHYS_BASE - buffer;
  if (!is_user_vaddr (buffer) || length > delta)
    syscall_exit (-1);
}

/*
 * This does not check that the string consists of only mapped pages; it merely
 * checks the string exists entirely below PHYS_BASE.
 */
static void
validate_string_in_user_region (const char* string)
{
  uintptr_t delta = PHYS_BASE - (const void*) string;
  if (!is_user_vaddr (string) || strnlen (string, delta) == delta)
    syscall_exit (-1);
}


static int
syscall_open (const char* filename)
{
  struct thread* t = thread_current ();
  if (t->open_file != NULL)
    return -1;

  t->open_file = filesys_open (filename);
  if (t->open_file == NULL)
    return -1;

  return 2;
}

static int
syscall_write (int fd, void* buffer, unsigned size)
{
  struct thread* t = thread_current ();
  if (fd == STDOUT_FILENO)
    {
      putbuf (buffer, size);
      return size;
    }
  else if (fd != 2 || t->open_file == NULL)
    return -1;

  return (int) file_write (t->open_file, buffer, size);
}

static int
syscall_read (int fd, void* buffer, unsigned size)
{
  struct thread* t = thread_current ();
  if (fd != 2 || t->open_file == NULL)
    return -1;

  return (int) file_read (t->open_file, buffer, size);
}

static void
syscall_close (int fd)
{
  struct thread* t = thread_current ();
  if (fd == 2 && t->open_file != NULL)
    {
      file_close (t->open_file);
      t->open_file = NULL;
    }
}

static void*
syscall_sbrk(intptr_t increment, struct intr_frame *f) 
{
  struct thread* t = thread_current ();
  if (increment == 0) {
    return t->sbrk;
  }
  
  int count = 0;
  
  void *pre_sbrk = t->sbrk;
  void *kpage_start = NULL;
  void *upage_start = NULL;
  //printf("t->sbrk first: %p\n",t->sbrk);
  //printf("heap first: %p\n",t->heap_start_address);
  //printf("increment: %d\n", increment);
  if (is_user_vaddr(t->sbrk) && (f->esp > t->sbrk))
  {
    //printf("if: increment:%d\n", increment);
    while(count * PGSIZE < increment) {
      
        //printf("while: increment:%d\n", increment);
        void* upage = pg_round_down((uint8_t *) t->sbrk );
        if (upage_start == NULL) {
          upage_start = upage;
        }
        //printf("upage %p\n", upage);
        void* kpage = (void*)palloc_get_page(PAL_USER | PAL_ZERO);
        if (kpage_start == NULL) {
          kpage_start = kpage;
        }
        bool writable = true;
        
        if (kpage == NULL) {
            //printf("count: %d\n", count);
            //printf("upage: %p\n", upage);
            //printf("kpage is null 1: %p\n", t->sbrk);
            palloc_free_multiple(kpage_start, count);
            for (int i = 0; i < count; i++) {
              pagedir_clear_page(t->pagedir, upage_start + PGSIZE * i);
            }
            
            t->sbrk = pre_sbrk;
            //printf("kpage is null 2: %p\n", t->sbrk);
            return (void*) -1;
        } 
        //printf("upage11: %p\n", upage);
        //printf("kpage11: %p\n", kpage);
        bool success = pagedir_set_page(t->pagedir, upage, kpage, writable); 

        if (success) {
          
            count++;
            //printf("prev: %p\n", pre_sbrk);
            if (count * PGSIZE - increment > 0) {
              //printf("count * PGS: %d\n", count * PGSIZE -  increment);
              t->sbrk = t->sbrk + ( increment - (count - 1) * PGSIZE);
            } else {
              t->sbrk = t->sbrk + PGSIZE;
            }
        }
        else {
            //printf("set page failed\n");
            palloc_free_multiple(kpage_start, count);
            syscall_exit(-1);
        }
    }
  }
  return pre_sbrk;

  
}

static void
syscall_handler (struct intr_frame *f)
{
  uint32_t* args = (uint32_t*) f->esp;
  struct thread* t = thread_current ();
  t->in_syscall = true;

  validate_buffer_in_user_region (args, sizeof(uint32_t));
  switch (args[0])
    {
    case SYS_EXIT:
      validate_buffer_in_user_region (&args[1], sizeof(uint32_t));
      syscall_exit ((int) args[1]);
      break;

    case SYS_OPEN:
      validate_buffer_in_user_region (&args[1], sizeof(uint32_t));
      validate_string_in_user_region ((char*) args[1]);
      f->eax = (uint32_t) syscall_open ((char*) args[1]);
      break;

    case SYS_WRITE:
      validate_buffer_in_user_region (&args[1], 3 * sizeof(uint32_t));
      validate_buffer_in_user_region ((void*) args[2], (unsigned) args[3]);
      f->eax = (uint32_t) syscall_write ((int) args[1], (void*) args[2], (unsigned) args[3]);
      break;

    case SYS_READ:
      validate_buffer_in_user_region (&args[1], 3 * sizeof(uint32_t));
      validate_buffer_in_user_region ((void*) args[2], (unsigned) args[3]);
      f->eax = (uint32_t) syscall_read ((int) args[1], (void*) args[2], (unsigned) args[3]);
      break;

    case SYS_CLOSE:
      validate_buffer_in_user_region (&args[1], sizeof(uint32_t));
      syscall_close ((int) args[1]);
      break;
    
    case SYS_SBRK:
      validate_buffer_in_user_region (&args[1], sizeof(intptr_t));

      f->eax = (uint32_t) syscall_sbrk((intptr_t) args[1], f);
      break;


    default:
      printf ("Unimplemented system call: %d\n", (int) args[0]);
      break;
    }

  t->in_syscall = false;
}
