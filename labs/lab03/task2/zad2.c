#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {
    if (argc != 2){
        fprintf(stderr, "wrong number of arguments. 1 expected, got %d\n", argc - 1);
        return 1;
    }

    setbuf(stdout, NULL);

    printf("%s", argv[0]);
    execl("/bin/ls", "ls", argv[1], NULL);

    return 0;
}
