#include "ain_config.h"
#include "ain_globals.h"

#include <signal.h>
#include <sys/wait.h>

#include "os/ain_process.h"

struct _ain_process_t {
  pid_t pid;
};

void ain_set_signal(ain_sighandler_t handler) {
  signal(SIGINT, (sig_t)handler);
}

ain_process_t *ain_process_fork() {
  pid_t pid = fork();
  AIN_ASSERT_NULL(pid != -1);
  if (pid == 0) return NULL;

  ain_process_t *ps = (ain_process_t *)malloc(sizeof(ain_process_t));
  AIN_ASSERT_NULL(ps);
  ps->pid = pid;

  return ps;
}

int ain_process_is_alive(ain_process_t *p) {
  int status;
  int ret = waitpid(p->pid, &status, WNOHANG | WUNTRACED);
  return (ret == p->pid) && (!WIFEXITED(status) && !WIFSIGNALED(status));
}

void ain_process_stop(ain_process_t *child) { kill(child->pid, SIGINT); }

void ain_process_close(ain_process_t *child) {
  if (ain_process_is_alive(child)) kill(child->pid, SIGKILL);
  free(child);
}