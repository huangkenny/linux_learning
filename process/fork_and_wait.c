#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int global; /* In BSS segement, will automatically be assigned '0'*/

int main ()
{
   pid_t child_pid;
   int status;
   int local = 0;

   // creat a new process
   child_pid = fork();
   
   // fork failed
   if (child_pid < 0) {
      perror("fork failed");
      exit(1);
   }

   // Child process
   if (child_pid == 0) {
      local = 2;
      global = 2;

      printf("child process pid = %d, ppid = %d, local = %d global = %d\n", 
          getpid(), getppid(), local, global); 

      char* argv[] = {"bash"};
      return execv("/usr/bin/bash", argv); 
   } else {
     // parent process
      local = 1;
      global = 1;
      printf("parent process pid = %d, child_pid = %d, local = %d global = %d\n", 
          getpid(), child_pid, local, global); 
      pid_t exit_pid = waitpid(-1, &status, 0);
      printf("process %d exited: status = %d\n", exit_pid, WEXITSTATUS(status));
      printf("parent process pid = %d, child_pid = %d, local = %d global = %d\n", 
          getpid(), child_pid, local, global); 
      exit(0); 
   }

}
