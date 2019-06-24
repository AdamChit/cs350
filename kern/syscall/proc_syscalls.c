#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include <synch.h>
#include "opt-A2.h"
#include <machine/trapframe.h>
#include <array.h>

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

int *PID_counter;

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);

  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  // lock_acquire(curproc->proc_lock);
  // // set the process as dead
  // curproc->dead = true;
  // // save the return code + status
  // curproc->exit_code = _MKWAIT_EXIT(exitcode);
  // // wake up any process waiting for the current process to exit
  // cv_signal(curproc->proc_cv,curproc->proc_lock);
  // // destroy any of the current process's dead childern
  // for( unsigned i=0; i < array_num(curproc->childern); i++){
  //   struct proc *child_process = array_get(curproc->childern, i);
  //   if(child_process->dead){
  //     proc_destroy(child_process);
  //   }
  // }
  // // destroy the current process if parent is dead
  // if(curproc->parent == NULL){
  //   lock_release(curproc->proc_lock);
  //   proc_destroy(curproc);
  // }

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  /* for now, this is just a stub that always returns a PID of 1 */
  /* you need to fix this to make it work properly */
  *retval = curproc->PID;
  return(0);
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
  struct proc *found_child_process;
  lock_acquire(curproc->proc_lock);
  for( unsigned i=0; i < array_num(curproc->childern) ; i++){
    struct proc *child_process = array_get(curproc->childern, i);
    lock_acquire(child_process->proc_lock);
    if(child_process->PID == pid){
      found_child_process = child_process;
    }
    lock_release(child_process->proc_lock);
  }
  lock_release(curproc->proc_lock);
  lock_acquire(found_child_process->proc_lock);
  if (found_child_process == NULL){
    return ENOMEM;
  }
  if (found_child_process->dead){
    exitstatus = found_child_process->exit_code;
    lock_release(found_child_process->proc_lock);
    proc_destroy(found_child_process);
  }
  else {
    cv_wait(found_child_process->proc_cv,found_child_process->proc_lock);
    exitstatus = found_child_process->exit_code;
    lock_release(found_child_process->proc_lock);
    proc_destroy(found_child_process);
  }
  
  /* for now, just pretend the exitstatus is 0 */
 // exitstatus = 0;
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  
  return(0);
}

#if OPT_A2

int sys_fork(struct trapframe *tf,pid_t *retval){
  struct trapframe *tf_heap_copy = kmalloc(sizeof(*tf));
  *tf_heap_copy = *tf;


  const char *name  = "child_proc";
  const char *thread_name = "child_thread";



  // create the new child process
  struct proc *new_child_proc = proc_create_runprogram(name);
  if (!new_child_proc){
    return ENOMEM;
  }

  // create address space for new child process
  struct addrspace **new_address_space;
  int ret = as_copy(curproc_getas(), new_address_space);
  if (ret == ENOMEM){
    return ENOMEM;
  }

  // set the new address space to child processs
  spinlock_acquire(&new_child_proc->p_lock);
	new_child_proc->p_addrspace = *new_address_space;
	spinlock_release(&new_child_proc->p_lock);

  // add the child proccess to partens list of childern
  lock_acquire(curthread->t_proc->proc_lock);
  array_add(curthread->t_proc->childern, new_child_proc , NULL);
  lock_release(curthread->t_proc->proc_lock);
  
  int result = thread_fork(thread_name,new_child_proc,enter_forked_process,tf_heap_copy,0);
  
  //failled thread_fork
  if (result) {
    return ENOMEM;
  }

  *retval = new_child_proc->PID;

  return 0;
}


#else
#endif
