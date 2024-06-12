// OS assignment#3

#include "types.h"
#include "stat.h"
#include "user.h"

void get_proc_priority_test(void);
void fork_test(void);
void fork_test2(void);
void starvation_test(void);

// pid의 priority를 출력하는 테스트
void get_proc_priority_test(void)
{
    printf(1, "Test1\n");
    for(int i=0; i<10; i++){
        int priority = get_proc_priority(i);
        if(priority != -1){
            printf(1, "[*] pid : %d\n", i);
            printf(1, "[*] priority : %d\n", priority);
        }
    }
}

void fork_test(void)
{
    printf(1, "Test2\n");
    int pid = fork();
    if(pid == 0){
        int priority = get_proc_priority(getpid());
        printf(1, "[*] c_pid : %d\n", getpid());
        printf(1, "[*] c_priority : %d\n", priority);
        exit();
    }
    else if(pid > 0){
        wait();
		int priority = get_proc_priority(getpid());
        printf(1, "[*] p_pid : %d\n", getpid());
        printf(1, "[*] p_priority : %d\n", priority);
    }
}

void fork_test2(void)
{
    printf(1, "Test3\n");
    set_proc_priority(getpid(), 3);
    int pid = fork();
    if(pid == 0){
        int priority = get_proc_priority(getpid());
        printf(1, "[*] pid : %d\n", getpid());
        printf(1, "[*] priority : %d\n", priority);
        exit();
    }
    else if(pid > 0){
        wait();
		int priority = get_proc_priority(getpid());
        printf(1, "[*] p_pid : %d\n", getpid());
        printf(1, "[*] p_priority : %d\n", priority);
    }
}

void starvation_test(void) {
    printf(1, "Test4\n");
    int pid_high = fork();
    if(pid_high > 0){
        set_proc_priority(pid_high, 2);
    }
    else if(pid_high == 0){
        char *args_high[] = { "loop", "2", 0 }; 
        exec("loop", args_high);
        printf(1, "Exec failed for high priority process\n");
        exit();
    }
    sleep(100);
    int pid_med = fork();
    if(pid_med > 0){
        set_proc_priority(pid_med, 4);
    }
    else if(pid_med == 0){
        char *args_low[] = { "loop", "4", 0 }; 
        exec("loop", args_low);
        printf(1, "Exec failed for low priority process\n");
        exit();
    }
    sleep(100);
    int pid_low = fork();
    if(pid_low > 0){
        set_proc_priority(pid_low, 3);
    }
    else if(pid_low == 0){
        char *args_low[] = { "loop", "3", 0 }; 
        exec("loop", args_low);
        printf(1, "Exec failed for medium priority process\n");
        exit();
    }
    sleep(10);
    sleep(10);
    sleep(10);
    wait();
    wait();
    wait();
}


int main(int argc, char *argv[])
{
    get_proc_priority_test();
    fork_test();
    fork_test2();
    starvation_test();
    exit();
}