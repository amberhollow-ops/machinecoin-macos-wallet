#include <mach-o/dyld.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char **argv) {
 char exe[PATH_MAX], binary[PATH_MAX]; uint32_t size=sizeof(exe);
 if (_NSGetExecutablePath(exe,&size)) return 1;
 snprintf(binary,sizeof(binary),"%s/machinecoin-qt",dirname(exe));
 char **args=calloc(argc+2,sizeof(char*)); if(!args) return 1;
 args[0]=binary; args[1]="-lang=en";
 for(int i=1;i<argc;i++) args[i+1]=argv[i];
 execv(binary,args); perror("Machinecoin Wallet"); return 1;
}
