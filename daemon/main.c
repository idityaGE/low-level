#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[]) {
  pid_t pid = fork();
  
  if (pid < 0) exit(1); // failed
  if (pid > 0) exit(0); // success child process has been created

  if (setsid() < 0) exit(1); // failed to create a session for child process

  // session is the collection of one or more processes. 
  // By calling pid_t setsid(void) calling process becomes the session leader.
  // the session id set to the calling process pid
  // calling fuction also becomes the leader of the process group and PGID = calling process PID

  // The process is disassociated from its Controlling Terminal. 
  // This is crucial for creating daemons (background processes) 
  // that should continue running even after the user logs out or the terminal is closed.
  
  pid = fork(); // second fork to prevent the re-acquiring a terminal
  if (pid < 0) exit(1);
  if (pid > 0) exit(0);

  umask(0); // file creation mode 000

  chdir("/");
  
  FILE *pidfile = fopen("/var/run/mydaemon.pid", "w");
  if (pidfile) {
      fprintf(pidfile, "%d\n", getpid()); // write my daemon PID
      fclose(pidfile);
  } else {
    printf("Daemon PID: %d\n", getpid());
    perror("failed to write the daemon PID");
  }

  close(STDIN_FILENO);   // 0
  close(STDOUT_FILENO);  // 1
  close(STDERR_FILENO);  // 2

  while(true) {
    int fd = open("/tmp/mydaemon.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    
    if (fd != -1) {
      time_t now = time(NULL);
      char *timestamp = ctime(&now);
      write(fd, timestamp, strlen(timestamp));
      close(fd);
    }

    sleep(2);
  }

  return 1;
}
