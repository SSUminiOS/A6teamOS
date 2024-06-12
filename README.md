# OS_team6
- - -
## 0. Document  
- - -
- 운영체제 6팀 최종 결과 보고서.pdf  
- 운영체제 6팀 최종 발표.pptx  
- - -  
## 1. miniOS
- - -
### Implement Stack Pointer
miniOS  
├── test_02_conclusion.txt # 자세한 과정 및 결론 포함  
├── c_file.c # hex to bin  
├── elf_to_hex.py # elf to hex python  
├── hello.c # example code with library  
├── input_hex.txt # result of xxd  
├── output_array.txt # result of "elf_to_hex.py"  
├── withoutLibrary.c # example code without library  
├── return_10_c_file.c # c code for genarate 10 excutable bin  
├── scenario.c # bin을 바로 실행  
├── scenario2.c # bin에서 main entry sp 이동후 실행, task_struct, simple scheduler  
└── to_do_list/  
    ├── execute_1-10.bin # scenario2.c에서 sp를 0x0으로, 0.bin 삭제 해야 작동  
    └── execute_0.bin # scenario2.c에서 sp를 0x1129으로, 1~10.bin 삭제 해야 작동  
- - -
## 2. KosmOS
- - -
### Boot Loader + initial Kernel
KosmOS  
├── kernel  
│   ├── kernel.elf  
│   ├── main.cpp # kernel code  
│   └── etc: for kernel  
└── KosmosLoaderPkg  
    ├── KosmosLoaderPkg.dec  
    ├── KosmosLoaderPkg.dsc  
    ├── Loader.inf  
    └── Main.c # Boot Loader Code  
- - -
# 3. xv6_team6
- - -
### How to Test
1. git clone official empty xv6
   - git clone https://github.com/mit-pdos/xv6-public.git xv6
2. git clone our repository
   - git clone https://github.com/AltairKosmoTale/xv6 xv6_team6
3. copy & paste code from "xv6_team6" to "xv6"
4. make clean
5. make qemu-nox QEMU=$QEMU
- - -
### 6~8th week: Copy-On-Write   
- defs.h : get_refcount, inc_refcount, dec_refcount, GetNumFreePages, pagefault 함수 선언  
- kalloc.c : num_free_pages, pgrefcount, get_refcount, inc_refcount, dec_refcount, GetNumFreePages, freerange, kfree  
- mmu.h : PGSHIFT 선언  
- syscall.c : sys_getNumFreePages systemcall 추가  
- syscall.h : SYS_getNumFreePages systemcall 번호 선언  
- trap.c : pagefault handler 추가  
- user.h : getNumFreePages 추가  
- usys.S : getNumFreePages SYSCALL 추가  
- vm.c : copyuvm PTE_W 비트 수정, 부모 페이지 테이블 공유 하도록 수정, lcr3 추가, pagefault 구현  
- cow.c : Copy-On-Write 테스트 코드 작성  
- - -  
### 4~5th week: Round Robin based Priority Queue Scheduler  
- defs.h : set_proc_priority, get_proc_priority  
- syscall.c  : sys_set_proc_priority, sys_get_proc_priority 추가  
- syscall.h : set_proc_priority, get_proc_priority에 대한 시스템콜 번호 선언  
- sysproc.c : sys_set_proc_priority, sys_get_proc_priority에 대한 wrapper 구현  
- user.h : set_proc_priority, get_proc_priority 선언 추가  
- usys.S : set_proc_priority, get_proc_priority SYSCALL 선언  
- proc.c : procdump 함수 수정, scheduler 함수 aging logic 추가, set_proc_priority, get_proc_priority 함수 구현  
- proc.h : proc구조체에 priority, count 멤버변수 추가 
- proctest.c : "Round Robin based Priority Queue Scheduler" Test 함수  
- - -
### 3rd week: test user program  
- helloworld.c : test user program  
- Makefile : helloworld.c 추가  
- - -