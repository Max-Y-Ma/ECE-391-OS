#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "page.h"
#include "proc/PCB.h"
// #include "drivers/file.h"
#include "drivers/RTC.h"
#include "drivers/terminal.h"
#include "loader.h"
#include "syscalls.h"
#include "spinlock.h"
#include "alloc.h"

#define PASS 1
#define FAIL 0

#define CP1_TESTS 0
#define CP2_TESTS 0
#define CP3_TESTS 0
#define CP4_TESTS 0
#define CP5_TESTS 0
#define EXTRA_TESTS 1

#define ASSERT_PAGE_FAULT(mem_address)				\
do {												\
	char* xptr = (char*)mem_address;				\
	char x = *xptr; x++;							\
} while(0)

#define ASSERT_PAGE_ACCESS(mem_address)				\
do {												\
	char* xptr = (char*)mem_address;				\
	char x = *xptr; x++;							\
} while(0)

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

extern int32_t system_halt_wrapper(uint8_t status);
extern int32_t system_execute_wrapper(const uint8_t* command);
extern int32_t system_read_wrapper(int32_t fd, void* buf, int32_t nbytes);
extern int32_t system_write_wrapper(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t system_open_wrapper(const uint8_t* filename);
extern int32_t system_close_wrapper(int32_t fd);
extern int32_t system_getargs_wrapper(uint8_t* buf, int32_t nbytes);
extern int32_t system_vidmap_wrapper(uint8_t** screen_start);
extern int32_t system_set_handler_wrapper(int32_t signum, void* handler_address);
extern int32_t system_sigreturn_wrapper(void);
extern int32_t system_malloc_wrapper(uint32_t nbytes);
extern int32_t system_free_wrapper(void* ptr);
extern int32_t system_ioctl_wrapper(int32_t fd, uint32_t command, uint32_t args);

/* Checkpoint 1 tests */
#if CP1_TESTS
/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	/* Check IDT */
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	/* Check Multiple/Nested Exceptions */
	asm volatile("int $1");

	/* Check System Call */
	asm volatile("int $0x80");

	return result;
}

/**
 * @brief Paging Test : Checks memory protection of paging
 * 
 * @details Expected Behavior:
 * 				- 1.) [0x00000000 - 0x000B8000) -> Page Fault
 * 				- 2.) [0x000B8000] -> Valid Memory Access
 * 				- 3.) (0x000B8000, 0x003FFFFF] -> Page Fault
 * 				- 4.) [0x00400000, 0x007FFFFF] -> Valid Memory Access
 * 				- 5.) [0x00800000, 0xFFFFFFFF] -> Page Fault
 * 
 * @details Checks that the memory access behavior specified above
 *          is true. This is a valid test because paging should
 *          enforce memory R/W protection. 
*/
int paging_test() {
	TEST_HEADER;

	/* Test behavior #1 */
	// ASSERT_PAGE_FAULT(NULL);
	// ASSERT_PAGE_FAULT(0x000A0000);

	/* Test behavior #2 */
	ASSERT_PAGE_ACCESS(VIDEO_MEM_START);
	ASSERT_PAGE_ACCESS(VIDEO_MEM_START + 0x100);

	/* Test behavior #3 */
	// ASSERT_PAGE_FAULT(VIDEO_MEM_START + PAGE_4KB_SIZE_B);
	// ASSERT_PAGE_FAULT(0x003FFFFF);

	/* Test behavior #4 */
	ASSERT_PAGE_ACCESS(0x00400000);
	ASSERT_PAGE_ACCESS(0x007FFFFF);

	/* Test behavior #5 */
	// ASSERT_PAGE_FAULT(0x00800000);
	// ASSERT_PAGE_FAULT(0xFFFFFFFF);

	return PASS;
}

int keyboard_raw_test() {
	TEST_HEADER;

	asm volatile("int $0x21");
	while(1);
}
#endif

/* Checkpoint 2 tests */
#if CP2_TESTS
int file_read_cp2_test() {
	TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);

	const uint8_t name[] = "frame0.txt";
	int32_t fd = pcb_open(&pcb, name);
	if (-1 == fd) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	uint8_t buf[187];
	if (-1 == pcb_read(&pcb, fd, buf, 187)) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	KDEBUG("%s\n", buf);

	if (-1 == pcb_close(&pcb, fd)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}

	return PASS;
}

int file_read_dentry_name_test(){

	TEST_HEADER;
	// /* Test behavior one (file that exists and is txt)*/
	int ret;

	// const uint8_t name = "frame0.txt";
	// dentry_t dentry;
	// ret = read_dentry_by_name(name, &dentry );
	// uint8_t buf[file_system.inode_base[dentry.inode_num].length];

	// ret = read_data(dentry.inode_num, 0, buf, file_system.inode_base[dentry.inode_num].length); /* Does the last argument work?*/
	// printf("string read is \n\n%s\n", buf);

	/* Test behavior two (file that exists and is not txt)*/

	// const uint8_t name2 = "ls";
	// dentry_t dentry2;
	// ret = read_dentry_by_name(name2, &dentry2 );
	// uint8_t buf[file_system.inode_base[dentry2.inode_num].length];
	// ret = read_data(dentry2.inode_num, 0, buf, file_system.inode_base[dentry2.inode_num].length); /* Does the last argument work?*/
	// printf("string read is \n%s\n", buf);
	
	/* Test behavior three (File that does not exist)*/
	const uint8_t name3[] = "filenotthere";
	dentry_t dentry3;
	ret = read_dentry_by_name(name3, &dentry3 );
	if (ret < 0) {
		 printf("Couldn't find file \n");
	} else {
		uint8_t buf[file_system.inode_base[dentry3.inode_num].length];
		ret = read_data(dentry3.inode_num, 0, buf, file_system.inode_base[dentry3.inode_num].length);
		printf("string read is \n%s\n", buf);
	}
	return PASS;
}

int file_read_dentry_index_test() {
	TEST_HEADER;

	int ret;

	int index = 15; /* corresponds to frame1.txt*/
	dentry_t dentry;
	ret = read_dentry_by_index(index, &dentry );
	if (ret >= 0) {
		uint8_t buf[file_system.inode_base[dentry.inode_num].length];
		ret = read_data(dentry.inode_num, 0, buf, file_system.inode_base[dentry.inode_num].length);
		printf("string read is \n%s\n", buf);
	} else {
		printf("Couldn't find file \n");
	}

	return PASS;
}

int list_files_test() {
	TEST_HEADER;

	boot_block_t * bblock =	file_system.boot_block;
	int i;
	for (i = 0; i < bblock->dir_count; i ++) {
		printf("file_name: ");
		int8_t file_name[32];
		strncpy(file_name, (int8_t*)bblock->direntries[i].filename,32);
		
		int file_type = bblock->direntries[i].filetype;
		int file_length  = file_system.inode_base[bblock->direntries[i].inode_num].length;
		printf("%s file_type: %d file_length: %d \n", file_name, file_type, file_length);

	}


	return PASS;
}

int read_directory_test() {
	TEST_HEADER;

	boot_block_t * bblock = file_system.boot_block;
	int i; 
	pcb_t process;
	pcb_init(&process);
	
	int fd2 = pcb_open(&process,  bblock->direntries[0].filename);
	for (i = 0; i < bblock->dir_count; i++) {
		printf("file name: ");
		uint8_t file_name[32];

		pcb_read(&process, fd2, file_name, 1);
		printf("%s file_type: ",file_name); 
		
		printf("%d file_length: %d ",bblock->direntries[i].filetype , file_system.inode_base[bblock->direntries[i].inode_num].length);
		printf("\n");
	}

	if (-1 == pcb_close(&process, fd2)) {
		KDEBUG("Failed to close file\n");
		return FAIL;
	}
	return PASS;
}

/*test behavior 1 (file exists and is txt)*/
int test_file_exists_txt(){
TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);
	uint8_t name[] = "frame0.txt";

	int32_t fd = pcb_open(&pcb, name);
	if (-1 == fd) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	uint8_t buf[4096];
	if (-1 == pcb_read(&pcb, fd, buf, 4096)) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	printf("%s\n", buf);
	if (-1 == pcb_close(&pcb, fd)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}
		return PASS;

}

int test_file_exists_txt2(){
TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);
	uint8_t name[] = "frame1.txt";

	int32_t fd = pcb_open(&pcb, name);
	if (-1 == fd) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	uint8_t buf[4096];
	if (-1 == pcb_read(&pcb, fd, buf, 4096)) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	printf("%s\n", buf);
	if (-1 == pcb_close(&pcb, fd)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}
		return PASS;

}

int test_file_exists_not_txt() {
TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);
	/* test behavior 2 (file that exists and is not txt)*/
	uint8_t name[] = "ls";
	int32_t fd2 = pcb_open(&pcb, name);
	if (-1 == fd2) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	uint8_t buf[8192];
	int data;
	int i;
	if (-1 == (data = pcb_read(&pcb, fd2, buf, 8192))) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	for (i = 0 ; i < data; i ++) {
		printf("%c", buf[i]);
	}
	printf("\n");
if (-1 == pcb_close(&pcb, fd2)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}
		return PASS;

}

int test_file_exists_not_txt2() {
TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);
	/* test behavior 2 (file that exists and is not txt)*/
	uint8_t name[] = "grep";
	int32_t fd2 = pcb_open(&pcb, name);
	if (-1 == fd2) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	uint8_t buf[8192];
	int data;
	int i;
	if (-1 == (data = pcb_read(&pcb, fd2, buf, 8192))) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	for (i = 0 ; i < data; i ++) {
		if (buf[i] == '\b') {continue;}
		printf("%c", buf[i]);
	}
	printf("\n");
if (-1 == pcb_close(&pcb, fd2)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}
		return PASS;

}

int test_file_not_exist() {
TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);
	/* test behavior 3 (file that does not exist)*/

	uint8_t name[] = "non-existent";
	uint8_t buf[4096];
	int32_t fd3 = pcb_open(&pcb, name);
	if (-1 == fd3) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	if (-1 == pcb_read(&pcb, fd3, buf, 4096)) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	printf("%s\n", buf);
	if (-1 == pcb_close(&pcb, fd3)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}
		return PASS;

}
int test_long_file() {
	TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);
	/* test behavior 4 (long file)*/
	const uint8_t name[] = "fish";

	int32_t fd4 = pcb_open(&pcb, name);
	if (-1 == fd4) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}

	uint8_t buf[8192];
	int data;
	int i;
	if (-1 == (data= pcb_read(&pcb, fd4, buf, 8192)) ){
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	for (i = 0; i < data; i ++) {
		printf("%c", buf[i]);
	}

	if (-1 == pcb_close(&pcb, fd4)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}
		return PASS;

}
int test_long_file_name() {
TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);

	/* test behavior 5 (file with very long name)*/
	uint8_t buf2[8192];
	const uint8_t name[] = "verylargetextwithverylongname.txt";
	int32_t fd5 = pcb_open(&pcb,name);
	if (-1 == fd5) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	if (-1 == pcb_read(&pcb, fd5, buf2, 8192)) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	KDEBUG("Successfully read from file: %s\n\n", name);
	printf("%s\n", buf2);


	

	if (-1 == pcb_close(&pcb, fd5)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}
		return PASS;

}

int test_part_file() {
TEST_HEADER;

	pcb_t pcb;
	pcb_init(&pcb);
	/* test behavior 6 ( reading part of a file, then resuming reading )*/
	const uint8_t name[] = "frame0.txt";
	int fd = pcb_open(&pcb, name);
	if (-1 == fd) {
		KDEBUG("Failed to open file: %s\n", name);
		return FAIL;
	}
	uint8_t buf [187];
	if (-1 == pcb_read(&pcb, fd, buf, 118)) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	// KDEBUG("Successfully read from file: %s\n", name);
	printf("%s\n Filler Text\n", buf);
	uint8_t buf2[100];
	if (-1 == pcb_read(&pcb, fd, buf2, 68)) {
		KDEBUG("Failed to read file: %s\n", name);
		return FAIL;
	}
	// KDEBUG("Successfully read from file: %s\n", name);
	printf("%s\n", buf2);
	if (-1 == pcb_close(&pcb, fd)) {
		KDEBUG("Failed to close file: %s\n", name);
		return FAIL;
	}

	return PASS;
}

int rtc_freq_mod(){
	int j,k;
	clear();
	uint32_t freq = 2;

	unsigned char filename[] = "file";
	RTC_open(filename);
	while(1){
		k = RTC_write(&freq);
		for(j=0; j<20; j++){
			//Write single character to screen here
			printf("i");
			//Wait for interrupt
			k = RTC_read();
		}
		// printf(freq);
		printf("\n");
		// clear();
		freq = freq*2;
		if(freq > 1024)
			freq = 2;
	}
	return PASS;	//never reach here
}

int invalid_frequency_check(){
	unsigned char filename[] = "file";
	RTC_open(filename);
	uint32_t rate = 5;

	/*Test invalid rates*/
	if(RTC_write(&rate) == 0)
		return FAIL;

	rate = 1337;
	if(RTC_write(&rate) == 0)
		return FAIL;

	rate = 175;
	if(RTC_write(&rate) == 0)
		return FAIL;

	rate = 2048;
	if(RTC_write(&rate) == 0)
		return FAIL;

	/*Test valid rates*/
	rate = 512;
	if(RTC_write(&rate) == -1)
		return FAIL;
	
	rate = 4;
	if(RTC_write(&rate) == -1)
		return FAIL;

	return PASS;
}
#endif

/* Checkpoint 3 tests */
#if CP3_TESTS
int terminal_test() {
	TEST_HEADER;

	/* Clear Screen */
	clear();
	putc(' ');

	uint8_t filename[] = "file";
	terminal_open(filename);

	/* Test printing a message */
	int32_t fd = 0;
	int8_t buffer[MAX_CHAR_BUF_SIZE] = "Hello World\nHello Bro!\n\n";
	int8_t nbytes = strlen(buffer);
	terminal_write(fd, buffer, nbytes);

	/* Should Print Hello World*/
	terminal_write(fd, buffer, strlen("Hello World\n"));

	/* Should Print Original Message Still*/
	terminal_write(fd, buffer, nbytes * 2);

	/* Test printing a long message */
	int8_t buffer2[] = "\nHello World!Hello World!Hello World!Hello World!Hello World!Hello World! \
	Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!\n";
	if (strlen(buffer2) != terminal_write(fd, buffer2, strlen(buffer2))) {assertion_failure();}

	/* Test terminal_read arguments */
	if (terminal_read(fd, NULL, 10) != -1 || terminal_write(fd, NULL, 10) != -1) {assertion_failure();}
	if (terminal_read(-1, buffer2, 10) != -1 || terminal_write(-1, buffer2, 10) != -1) {assertion_failure();}
	if (terminal_read(fd, buffer2, -1) != -1 || terminal_write(fd, buffer2, -1) != -1) {assertion_failure();}

	/* Test echoing messages & read overflow/underflow */
	int i = 0;
	if (terminal_read(fd, buffer2, 0) != 0) {assertion_failure();}
	for (i = 0; i < 2; i++) {
		if (terminal_read(fd, buffer2, 1) > 1) {assertion_failure();}

		if (strlen(buffer2) != terminal_write(fd, buffer2, strlen(buffer2))) {assertion_failure();}
	}
	for (i = 0; i < 2; i++) {
		if (terminal_read(fd, buffer2, 2) > 2) {assertion_failure();}

		if (strlen(buffer2) != terminal_write(fd, buffer2, strlen(buffer2))) {assertion_failure();}
	}
	for (i = 0; i < 2; i++) {
		if (terminal_read(fd, buffer2, MAX_CHAR_BUF_SIZE/2) > MAX_CHAR_BUF_SIZE/2) {assertion_failure();}

		if (strlen(buffer2) != terminal_write(fd, buffer2, strlen(buffer2))) {assertion_failure();}
	}

	/* Infinite terminal echoing */
	while(1) {
		if (terminal_read(fd, buffer2, MAX_CHAR_BUF_SIZE) > MAX_CHAR_BUF_SIZE) {assertion_failure();}

		if (strlen(buffer2) != terminal_write(fd, buffer2, strlen(buffer2))) {assertion_failure();}
	}

	while(1) { asm volatile ("hlt"); }
}
int pcb_function_tests() {
	TEST_HEADER;
	int i;

	/* We start with bad parameters */
	KDEBUG("Testing bad files, we should see ERRORS\n");
	uint8_t name[] = "BADFILEBADFILE";
	if (-1 != pcb_open(name)) {
		return FAIL;
	}
	if (-1 != pcb_check_valid_fd(FILE_ARRAY_SIZE + 1)) {
		return FAIL;
	}
	if (-1 != pcb_check_valid_fd(2)) {
		return FAIL;
	}

	KDEBUG("\nTesting random file read\n");

	/* Generic tests */
	pcb_t* pcb = get_curr_pcb();
	if (!pcb) return FAIL;
	if (-1 == pcb_init(pcb)) return FAIL;
	uint8_t name1[] = "shell";
	int32_t fd = pcb_open(name1);
	if (-1 == fd) return FAIL;
	KDEBUG("Reading first 30 chars from shell\n");
	uint8_t buf[30];
	if (-1 == pcb_read(fd, buf, 30));
	KDEBUG("Successfully read from file: shell\n");
	KDEBUG("%s\n\n", buf);
	if (-1 == pcb_close(fd)) {
		KDEBUG("Failed to close file: shell\n");
		return FAIL;
	}

	KDEBUG("Testing syscall write/read through RTC\n");

	/* RTC tests, and correct file array cleanup tests */
	fd = pcb_open((uint8_t*)"rtc");
	if (-1 == fd) return FAIL;
	uint32_t freq = 8;
	if (-1 == pcb_write(fd, &freq, 0)) return FAIL;
	int j, k;
	for(j=0; j<20; j++){
		//Write single character to screen here
		printf("i");
		//Wait for interrupt
		k = pcb_read(fd, 0, 0);
		if (-1 == k) return FAIL;
	}
	printf("\n\n");

	KDEBUG("Trying to open too many files test\n");

	/* Just for fun, let's open a bunch of files */
	if (-1 == pcb_open((uint8_t*)"testprint")) return FAIL;
	if (-1 == pcb_open((uint8_t*)"testprint")) return FAIL;
	if (-1 == pcb_open((uint8_t*)"testprint")) return FAIL;
	if (-1 == pcb_open((uint8_t*)"testprint")) return FAIL;
	if (-1 == pcb_open((uint8_t*)"testprint")) return FAIL;

	/* This last one should fail, because we tried to open too many files */
	if (-1 == pcb_open((uint8_t*)"testprint")) {
		KDEBUG("Too many files test passed\n\n");
	} else {
		return FAIL;
	}

	KDEBUG("Changing the frequency test\n");
	freq = 128;
	pcb_write(fd, &freq, 0);
	for(j=0; j<20; j++){
		//Write single character to screen here
		printf("i");
		//Wait for interrupt
		k = pcb_read(fd, 0, 0);
	}
	printf("\n\n");
	if (-1 == pcb_close(2)) return FAIL;
	if (-1 == pcb_close(3)) return FAIL;
	if (-1 == pcb_close(4)) return FAIL;
	if (-1 == pcb_close(5)) return FAIL;
	if (-1 == pcb_close(6)) return FAIL;
	if (-1 == pcb_close(7)) return FAIL;

	KDEBUG("Inspect the PCB: only stdin and stdout should be open\n");
	KDEBUG("PCB active? %d     PCB id? %d\n", pcb->active, pcb->id);
	for (i=0;i<FILE_ARRAY_SIZE;i++) {
		fd_t fdd = pcb->file_array[i];
		KDEBUG("fd: %d {op_table = %d, inode = %d, file_pos = %d, flags = %d}\n", 
						i, fdd.op_table, fdd.inode, fdd.file_pos, fdd.flags);
	}

	if (-1 == pcb_destroy(pcb)) return FAIL;


	return PASS;
}

int basic_process_test() {
	TEST_HEADER;

	system_execute((uint8_t*)"shell");
	//system_execute("shell");
	system_halt(1);

	return PASS;
}

/*NEEDS TO BE EXPANDED!!*/
int syscall_test(){
	TEST_HEADER;

	int32_t fd = 0;
	uint8_t buf[] = "test"; 
	uint8_t* buf_ptr = buf;
	int32_t nbytes = 1;
	uint8_t command = 0;

	system_halt_wrapper(command);

	system_execute_wrapper(&command);

	system_read_wrapper(fd, buf, nbytes);

	system_write_wrapper(fd, buf, nbytes);

	system_open_wrapper(&command);

	system_close_wrapper(fd);

	system_getargs_wrapper(buf, nbytes);

	system_vidmap_wrapper(&buf_ptr);

	system_set_handler_wrapper(fd, buf);

	system_sigreturn_wrapper();

	return PASS;
}

int rtc_freq_mod(){
	int j,k;
	clear();
	uint32_t freq = 2;

	unsigned char filename[] = "file";
	RTC_open(filename);
	while(1){
		k = RTC_write(0, &freq, 0);
		for(j=0; j<20; j++){
			//Write single character to screen here
			printf("i");
			//Wait for interrupt
			k = RTC_read(0, 0, 0);
		}
		// printf(freq);
		printf("\n");
		// clear();
		freq = freq*2;
		if(freq > 1024)
			freq = 2;
	}
	return PASS;	//never reach here
}
#endif 

/* Checkpoint 4 tests */
#if CP4_TESTS
#endif

/* Checkpoint 5 tests */
#if CP5_TESTS
int pit_test() {
	TEST_HEADER;

	

	return PASS;
}
#endif

#if EXTRA_TESTS
int spinlock_test() {

	/* Check spinlock API calls */
	spinlock_t lock = SPIN_LOCK_UNLOCKED;
	if (spin_lock(&lock) != 0) {
		assertion_failure();
	}

	if (spin_unlock(&lock) != 0) {
		assertion_failure();
	}

	uint32_t flags;
	if (spin_lock_irqsave(&lock, &flags) != 0) {
		assertion_failure();
	}

	if (spin_lock_irqrestore(&lock, &flags) != 0) {
		assertion_failure();
	}

	/* Check blocking */
	spin_lock(&lock);
	spin_lock(&lock);

	return 0;
}

uint32_t slab_object_size(uint32_t index)
{
	if (index == 0) {
		return SLAB_OBJECT_SIZE_0;
	}
	else if (index == 1) {
		return SLAB_OBJECT_SIZE_1;
	}
	else if (index == 2) {
		return SLAB_OBJECT_SIZE_2;
	}
	else if (index == 3) {
		return SLAB_OBJECT_SIZE_3;
	}
	else if (index == 4) {
		return SLAB_OBJECT_SIZE_4;
	}
	else if (index == 5) {
		return SLAB_OBJECT_SIZE_5;
	}
	else if (index == 6) {
		return SLAB_OBJECT_SIZE_6;
	}
	else if (index == 7) {
		return SLAB_OBJECT_SIZE_7;
	}
	else {
		return -1;
	}
}

int slab_cache_basic_test() {
	/* Test kernel allocations */
	uint8_t* knewchar = (uint8_t*)kmalloc(sizeof(uint8_t), KMEM_KERNEL);
	*knewchar = 'a';
	kfree(knewchar, KMEM_KERNEL);
	
	/* Test malloc user call */
	uint8_t* newchar = (uint8_t*)system_malloc_wrapper(sizeof(uint8_t));
	*newchar = 'b';
	system_free_wrapper(newchar);

	/* Test all different sizes of allocations */
	uint16_t i;
	for (i = 0; i < NUM_SLAB_OBJECTS; i++) {
		uint32_t size = slab_object_size(i);
		uint8_t* knew1 = (uint8_t*)kmalloc(size * sizeof(uint8_t), KMEM_KERNEL);
		uint8_t* knew2 = (uint8_t*)kmalloc(size * sizeof(uint8_t), KMEM_KERNEL);

		uint16_t j;
		for (j = 0; j < size; j++) {
			knew1[j] = j;
			knew2[j] = j;
		}

		kfree(knew1, KMEM_KERNEL);
		kfree(knew2, KMEM_KERNEL);
	}

	/* Test error conditions */
	uint8_t* reaccess_freeptr = (uint8_t*)kmalloc(sizeof(uint8_t), KMEM_KERNEL);
	*reaccess_freeptr = 'a';
	kfree(reaccess_freeptr, KMEM_KERNEL);
	*reaccess_freeptr = 'b'; // This should error since already freed

	return 0;
}

int ioctl_test() {
	/* Call terminal output mode ioctl */
	uint32_t stdin = 0;
	uint32_t output_mode_off = 0;
	system_ioctl_wrapper(stdin, TERMINAL_IOCTL_SET_OUTPUT_MODE, output_mode_off);

	return 0;
}
#endif

/* Test suite entry point */
void launch_tests(){
	/* Clear Screen */
	clear();

	/* Initialize terminal */
	// terminal_open((uint8_t*)"None");

#if CP1_TESTS
	/* CP1 Tests */
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("paging_tests", paging_test());
#endif

#if CP2_TESTS
	/* CP2 Tests */

	// TEST_OUTPUT("CP2 File Read Test", file_read_cp2_test());
	// TEST_OUTPUT("test_file_exists", test_file_exists_txt());
	// TEST_OUTPUT("test_file_exists2", test_file_exists_txt2());

	// TEST_OUTPUT("test_file_exists_not_txt", test_file_exists_not_txt());
	// TEST_OUTPUT("test_file_exists_not_txt2", test_file_exists_not_txt2());

	// TEST_OUTPUT("test_file_not_exist", test_file_not_exist());

	// TEST_OUTPUT("test_long_file", test_long_file());
	// TEST_OUTPUT("test_long_file_name", test_long_file_name());
	// TEST_OUTPUT("test_part_file", test_part_file());

	// TEST_OUTPUT("read_directory_test", read_directory_test());
	// TEST_OUTPUT("rtc frequency modulation", rtc_freq_mod());
	// TEST_OUTPUT("rtc write allowable", invalid_frequency_check());
#endif

#if CP3_TESTS
	/* CP3 Tests */
	// TEST_OUTPUT("terminal_test", terminal_test());
	// TEST_OUTPUT("Syscall ret val test", syscall_test());
	// TEST_OUTPUT("PCB Tests", pcb_function_tests());
	// TEST_OUTPUT("Basic Process Test", basic_process_test());
	// TEST_OUTPUT("RTC test cp3", rtc_freq_mod());
#endif

#if CP4_TESTS
#endif

#if CP5_TESTS
#endif

#if EXTRA_TESTS
	// TEST_OUTPUT("spinlock test", spinlock_test());
	// TEST_OUTPUT("dynamic allocation test", slab_cache_basic_test());
	TEST_OUTPUT("ioctl base test", ioctl_test());
#endif

}
