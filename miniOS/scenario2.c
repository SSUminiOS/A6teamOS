#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


typedef struct task_struct {
    void *stack;
    unsigned long state;  // 0: runnable, 1: running, 2: terminated
    struct task_struct *next;
} task_struct;

task_struct *head = NULL;

// 프로세스를 실행하는 함수
int execute_process(task_struct *task) {
    if (task->state != 0 && task->state != 1) {
        fprintf(stderr, "Task cannot be run\n");
        return -1;
    }
    task->state = 1; // running
    /*
	// 스택 내용이 문자열로 가정하고 출력
	if (task->stack != NULL) {
        printf("Stack content as string: '%s'\n", (char *)task->stack);
    } else {
        printf("Error: Stack is NULL\n");
    }*/

    //int (*func)() = (int (*)())task->stack;  // 함수 포인터로 변환
	int (*func)() = (int (*)())(task->stack+0x1129);
    int result = func();  // 함수 실행
    task->state = 2; // terminated

    return result;
}

// 새로운 프로세스 생성
task_struct *create_process(char *binary_code, size_t code_size) {
	
    long pagesize = sysconf(_SC_PAGESIZE)*250;
    void *stack = mmap(NULL, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    memcpy(stack, binary_code, code_size);

    task_struct *new_task = malloc(sizeof(task_struct));
    if (new_task == NULL) {
        munmap(stack, pagesize);
        return NULL;
    }

    new_task->stack = stack;
    new_task->state = 0;  // runnable
    new_task->next = NULL;

    if (head == NULL) {
        head = new_task;
    } else {
        task_struct *temp = head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_task;
    }

    return new_task;
}

// 파일로부터 바이너리 코드 읽기 및 프로세스 생성
void load_and_create_process(const char *filename) {
	
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }
	
    char buffer[1000000];
	
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
	
    if (bytes_read <= 0) {
        perror("Failed to read from file");
        close(fd);
        return;
    }
	
    //create_process(buffer, bytes_read);  // bytes_read에 널 문자를 포함하여 전달
	create_process(buffer, bytes_read);
    close(fd);
}

/*
	buffer[bytes_read] = '\0';  // 읽은 데이터 끝에 널 문자 추가
    // buffer를 사용하여 프로세스 생성
	char buffer1[] = "\x48\xC7\xC0\x7B\x00\x00\x00\xC3";
	char buffer2[1024];
	memcpy(buffer2, buffer, sizeof(buffer1)); // 널 문자를 포함하여 복사
*/

// Round-robin 스케줄러 실행
void run_scheduler() {
    task_struct *current = head;
    while (current != NULL) {
        if (current->state == 0 || current->state == 1) {
            int result = execute_process(current);
            printf("Process returned: %d\n", result);
        }
        current = current->next;
    }
}

int main() {
    const char *dir_path = "to_do_list";
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Failed to open directory");
        return 1;
    }

    struct dirent *entry;
	
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // 일반 파일인 경우
            // 파일 확장자가 .bin인지 확인
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && !strcmp(ext, ".bin")) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
				
                load_and_create_process(full_path);
            }
        }
    }
    closedir(dir);

    run_scheduler(); // 스케줄러 실행

    return 0;
}