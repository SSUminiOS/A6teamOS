# xv6_team6
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