#include <fcntl.h>

extern char **environ;
extern void exit(int code);
extern int main(int argc, char **argv);
 
void _start() {
  int ex = main(0, 0);
  exit(ex);
}
