#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct task_struct {
    void *stack;
    unsigned long state;  // 0: runnable, 1: running, 2: terminated
    int (*start_routine)(void *);
    void *arg;
} task_struct;

const char hello_world_code[] = "\x48\xC7\xC0\x7B\x00\x00\x00\xC3"; // return 123;

// 프로세스를 실행하는 함수
int execute_process(task_struct *task) {
    memcpy(task->stack, hello_world_code, sizeof(hello_world_code));
    int (*func)() = (int (*)())task->stack;
    return func();
}

// 새로운 프로세스 생성
task_struct *create_process(int (*start_routine)(void *), void *arg) {
    long pagesize = sysconf(_SC_PAGESIZE);
    void *stack = mmap(NULL, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    task_struct *new_task = malloc(sizeof(task_struct));
    if (new_task == NULL) {
        munmap(stack, pagesize);
        return NULL;
    }

    new_task->stack = stack;
    new_task->state = 0;  // runnable
    new_task->start_routine = start_routine;
    new_task->arg = arg;

    return new_task;
}

// 프로세스 종료
void terminate_process(task_struct *task) {
    long pagesize = sysconf(_SC_PAGESIZE);
    munmap(task->stack, pagesize);
    free(task);
}

int main() {
    task_struct *task = create_process((int (*)(void *))execute_process, NULL);
    if (!task) {
        fprintf(stderr, "Failed to create process\n");
        return 1;
    }

    task->state = 1;  // running
    int result = execute_process(task);
    printf("Process returned: %d\n", result);

    task->state = 2;  // terminated
    terminate_process(task);

    return 0;
}