#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}
/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	/*
	맨 위에 인라인 어셈블리로 된 syscall() 함수
	번호를 rax, 인자들을 rdi, rsi, rdx, r10, r8, r9에 넣는 코드가 그대로 보임
	*/
	printf ("system call!\n");

	switch(f->R.rax){
		case SYS_HALT:{
			halt();
			break;
		}
		case SYS_EXIT:{
			int status = f->R.rax;
			exit(status); // int status
			break;
		}
		case SYS_FORK:
			const char *thread_name = f->R.rdi;
			fork(thread_name);
			break;
			
		case SYS_WAIT:{
			pid_t pid = f->R.rdi;
			wait(pid); //  pid_t pid
			break;
		}
		case SYS_CREATE:{
			char * file = f->R.rdi;
			unsigned initial_size = f->R.rsi;
			create(file, initial_size); // char *file, initial_size
			break;
		}
		case SYS_REMOVE:{
			const char * file = f->R.rdi;
			remove(file); // const char * file
			break;
		}
		case SYS_OPEN:{
			char *file = f->R.rdi;
			open(file); //char *file
			break;
		}
		case SYS_FILESIZE:{
			int fd = f->R.rdi;
			filesize(fd); // int fd
			break;
		}
		case SYS_READ: {
			int fd = f->R.rdi;
			void *buffer = f->R.rsi;
			unsigned size = f->R.rdx;
			read(fd, buffer, size); // int fd, void *buffer, unsigned size
			break;
		}
		case SYS_WRITE:{
			int fd = f->R.rdi;
			void *buffer = f->R.rsi;
			unsigned size = f->R.rdx;
			write(fd, buffer, size); // int fd, const void *buffer, unsigned size
			break;
		}
		case SYS_SEEK:{
			int fd = f->R.rdi;
			unsigned position = f->R.rsi;
			seek(fd, position); // int fd, unsinged position
			break;
		}
		case SYS_TELL:{
			int fd = f->R.rdi;
			tell(fd); // int fd
			break;
		}
		case SYS_CLOSE:{
			int fd = f->R.rdi;
			close(fd); // int fd
			break;
		}
	}


	thread_exit ();
}

void
halt (void){
	/*
	pintos 종료 시, power_off() (에 선언된 src/include/threads/init.h)를 호출 해야 함
	이 방법은 교착 상태 등에 대한 일부 정보를 잃게 됨
	거의 사용하지 않는 것이 좋음
	*/
}

void
exit (int status){
	/*
	현재 사용자 프로그램 종료
	status 커널로 돌아감
	프로세스의 부모 wait 프로세스가 해당 프로세스에 대해 종료를 요청한 경우
	이 상태가 반환
	일반적으로 0은 status 성공
	0이 아닌 값은 오류를 나타냄
	*/
}

pid_t
fork (const char *thread_name){
	/*
	현재 프로세스의 복제본인 새 프로세스를 THREAD_NAME 이라는 이름으로 생성
	%RBX, %RSP, %R12-%R15를 제외하고 나머지 레지스터는 복제 안해도 됨
	해당 레지스터들은 호출 프로세스에서 저장되는 레지스터
	자식 프로세스의 PID 반환해야 함
	그렇지 않으면 유효 PID가 아님

	자식 프로세스는 파일 디스크립터와 가상 메모리 공간을 포함한 모든 리소스 복제해야함
	부모 프로세스는 자식 프로세스가 리소스를 성공적으로 복제 했는지 확인 할 때까지 fork() 호출을 종료해선 안됨
	자식 프로세스가 복제에 실패하면 부모 프로세스의 fork() 호출은 TID_ERROR를 반환

	template는 pml4_for_each() in을 사용
	해당 페이지 테이블 구조를 포함한 전체 사용자 메모리 공간을 복사하지만 전달된 값을 누락 부분을 채워야 함
	*/
}

int
exec (const char *cmd_line){
	/*
	프로세스를 지정된 이름의 실행 파일로 변경
	지정된 인수를 전달할 수 있음
	이 함수는 성공 반환 X
	프로그램이 로드 되거나 실행 되지 않으면 프로세스 종료 : -1
	함수는 호출한 스레드의 이름은 변경하지 않음
	파일 디스크립터는 호출 후에도 계속 열려 있음
 	*/
}

int
wait (pid_t pid){
	/*
	자식 프로세스를 기다리고 pid자식 프로세스의 종료 상태를 가져옵니다.
	자식 프로세스가 여전히 살아있으면 종료될 때까지 기다립니다.
	그런 다음, 종료 시 전달된 pid상태를 반환합니다 .
	자식 프로세스 가 `wait`를 호출하지 않았지만 커널에 의해 종료된 경우(예: 예외로 인해 강제 종료된 경우)에는 `false` 를 반환해야 합니다 .
	부모 프로세스가 `wait`를 호출할 때 이미 종료된 자식 프로세스를 기다리는 것은 완전히 합법적이지만,
	커널은 부모 프로세스가 자식 프로세스의 종료 상태를 가져오거나 자식 프로세스가 커널에 의해 종료되었음을 알 수 있도록 해야 합니다.pidpidexit()wait(pid)-1

	wait-1다음 조건 중 하나라도 충족되면 즉시 실패하고 반환해야 합니다 .

	pid`calling`은 호출 프로세스의 직접적인 자식 프로세스를 가리키는 것이 아닙니다.
	`calling`은 호출 프로세스 가 `calling` 호출에 성공하여 반환 값으로 `calling` pid을 받은 경우에만 호출 프로세스의 직접적인 자식 프로세스가 됩니다 .
	자식 프로세스는 상속되지 않습니다.
	예를 들어, A가 자식 프로세스 B를 생성하고 B가 자식 프로세스 C를 생성하는 경우, B가 종료되었더라도 A는 C를 기다릴 수 없습니다.
	프로세스 A의 `calling` 호출은 반드시 실패해야 합니다.
	마찬가지로, 부모 프로세스가 고아 프로세스보다 먼저 종료되면 고아 프로세스는 새로운 부모 프로세스에 할당되지 않습니다.pidforkwait(C)
	호출하는 프로세스는 wait이미 호출을 완료했습니다 wait. pid즉, 프로세스는 wait특정 자식 프로세스에 대해 최대 한 번만 호출할 수 있습니다.
	*/

}	

bool
create (const char *file, unsigned initial_size){
	
}

bool
remove (const char *file){
	
}

int
open (const char *file){
	
}

int
filesize (int fd){
	
}

int
read (int fd, void *buffer, unsigned size){
	/*
	fd로 열려 있는 파일에서 size 바이트를 읽어 buffer에 저장
	읽은 바이트 수를 반환하며(파일 끝이면 0),
	파일 끝이 아닌 다른 이유로 파일을 읽을 수 없는 경우에는 -1을 반환
	fd 0은 input_getc()를 사용해 키보드로부터 읽습니다.
	*/
	if(buffer == NULL){ // NULL비교
		exit(-1);
	}
	uint64_t *pml4 = thread_current()->pml4;

	if (!pml4_get_page(pml4, buffer)){ // 매핑 여부 확인
		exit(-1);
	}
	uint64_t *pte = pml4e_walk(pml4, buffer, 0);
	if(!is_user_vaddr(pte)){ // 유저 영역인지 확인
		exit(-1);
	}

	if (fd == 0){
		input_getc();
	}
	if (fd == 1){
		return -1;
	}
	if (fd >= 2){
		file_read()
	}

}

int
write (int fd, const void *buffer, unsigned size){
	/*
	체크 해야 하는 것 3가지
	NULL인가?
	유저 영역인가?
	실제 매핑되어 있는가
	-> 셋 중 하나라도 걸리면 exit(-1)
	*/
	if(buffer == NULL){ // NULL비교
		exit(-1);
	}
	uint64_t *pml4 = thread_current() -> pml4;

	if (!pml4_get_page(pml4, buffer)){ // 매핑 여부 확인
		exit(-1);
	}
	uint64_t *pte = pml4e_walk(pml4, buffer, 0);
	if(!is_user_vaddr(pte)){ // 유저 영역인지 확인
		exit(-1);
	} 

	putbuf(buffer, size); // buffer에서 n개의 문자를 콘솔에 출력하는 함수
}

void
seek (int fd, unsigned position){
	
}

unsigned
tell (int fd){
	
}

void
close (int fd){
	
}

