#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/fcntl.h>
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
#include <vfs.h>
#include <vm.h>
#include "opt-A2.h"
#include <test.h>
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

  lock_acquire(p->proc_lock);
  // set the process as dead
  p->dead = true;
  // save the return code + status
  p->exit_code = _MKWAIT_EXIT(exitcode);
  // wake up any process waiting for the current process to exit
  cv_broadcast(p->proc_cv,p->proc_lock);
  // destroy any of the current process's dead childern
  for( unsigned i=array_num(p->childern); i >0; i--){
    struct proc *child_process = array_get(p->childern, i - 1);
    if(child_process->dead){
      proc_destroy(child_process);
      array_remove(p->childern,i  - 1);
      //child_process = NULL;
    }
  }
  // destroy the current process if parent is dead
  if(p->parent == NULL){
    lock_release(p->proc_lock);
    proc_destroy(p);
  }else{
    lock_release(p->proc_lock);
  }

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */

  //proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}

void sys__exit_readonly(int exitcode) {

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

  lock_acquire(p->proc_lock);
  // set the process as dead
  p->dead = true;
  // save the return code + status
  p->exit_code = exitcode;
  // wake up any process waiting for the current process to exit
  cv_broadcast(p->proc_cv,p->proc_lock);
  // destroy any of the current process's dead childern
  for( unsigned i=array_num(p->childern); i >0; i--){
    struct proc *child_process = array_get(p->childern, i - 1);
    if(child_process->dead){
      proc_destroy(child_process);
      array_remove(p->childern,i  - 1);
      //child_process = NULL;
    }
  }
  // destroy the current process if parent is dead
  if(p->parent == NULL){
    lock_release(p->proc_lock);
    proc_destroy(p);
  }else{
    lock_release(p->proc_lock);
  }

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */

  //proc_destroy(p);
  
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

  lock_acquire(curproc->proc_lock);
  
  for( unsigned i=0; i < array_num(curproc->childern) ; i++){
    struct proc *child_process = array_get(curproc->childern, i);
    
    lock_acquire(child_process->proc_lock);
    if(child_process->PID == pid){
      //lock_acquire(child_process->proc_lock);
      while(!child_process->dead){
        cv_wait(child_process->proc_cv,child_process->proc_lock);
      }
      exitstatus = child_process->exit_code;

      lock_release(child_process->proc_lock);
      break;
    }
    lock_release(child_process->proc_lock);
  }

  lock_release(curproc->proc_lock);
  
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

  spinlock_acquire(&new_child_proc->p_lock);
  // create address space for new child process
  //struct addrspace **new_address_space;
  int ret = as_copy(curproc_getas(), &(new_child_proc->p_addrspace));
  if (ret == ENOMEM){
    return ENOMEM;
  }
  // set the new address space to child processs
	//new_child_proc->p_addrspace = *new_address_space;
	spinlock_release(&new_child_proc->p_lock);

  // add the child proccess to partens list of childern
  lock_acquire(curthread->t_proc->proc_lock);
  new_child_proc->parent = curthread->t_proc;
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

int
sys_execv(userptr_t progname, userptr_t args)
{
  (void)args;
  //char *progname;
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
  char *kern_progname = (char*) kmalloc(256*sizeof(char));
  struct addrspace *current_as = curproc->p_addrspace;
  int num_args = 0;
  //struct array *copyed_args = array_create();
  char *copyed_args[num_args+1];
  // copy user program name into kernal
  strcpy(kern_progname, (char*)progname);
  
  // count number of args
  while(((userptr_t *)args)[num_args])
  {
      num_args++;
      // ps++;
  }

  // added null to show end of args
  copyed_args[num_args] = NULL;
  
  // copy args into kern
  for(int i = 0; i < num_args; i++){
    //char* ps = (char *)((userptr_t *)args)[i];
    size_t size_new_param = strlen(((char **)args)[i]) + 1;
    //char *new_param = (char*) kmalloc(size_new_param*sizeof(char));
    //alloc right to the element in the array
    copyed_args[i] = kmalloc(size_new_param* sizeof(char));
    copyinstr((const_userptr_t)((userptr_t *)args)[i], copyed_args[i], size_new_param, NULL);
  }

  /* Open the file. */
	result = vfs_open(kern_progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* Create a new address space. */
	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	curproc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack_with_alignment(as, &stackptr,copyed_args, num_args);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}
  //kprintf("the number of args is %d\n", num_args);
	/* Warp to user mode. */
  //kill addrespace
  as_destroy(current_as);
	enter_new_process(num_args/*argc*/, (userptr_t)stackptr /*userspace addr of argv*/,
			  stackptr, entrypoint);
	
	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}
#else
#endif

