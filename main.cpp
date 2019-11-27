#include "shell.h"

void handle_signal(int signo) {
}

int main(int argc, char **argv) {
    signal(SIGINT, handle_signal);
    shell microsha;

    return 0;
}