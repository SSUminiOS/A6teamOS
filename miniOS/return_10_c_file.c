#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    for (int i = 1; i < 10; i++) {
        char data[] = {0x48, 0xC7, 0xC0, i, 0x00, 0x00, 0x00, 0xC3};
        char file_name[20]; // 충분한 크기의 배열을 선언
        sprintf(file_name, "execute_%d.bin", i); // 파일 이름 형식화
        
        int fd = open(file_name, O_CREAT | O_WRONLY, 0644);
        if (fd < 0) {
            perror("Failed to create file");
            return 1;
        }
        write(fd, data, sizeof(data));
        close(fd);
    }
    return 0;
}