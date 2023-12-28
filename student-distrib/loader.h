#include "drivers/file.h"
#include "proc/PCB.h"

#define FIRST_BYTE 0x7f
#define SECOND_BYTE 'E'
#define THIRD_BYTE 'L'
#define FOURTH_BYTE 'F'
#define ELF_HEADER_SIZE 40

/* Program images are linked to execute at virtual address 0x08048000 */
#define PROGRAM_START_MEM 0x8048000
#define PROGRAM_END_MEM 0x8400000

typedef struct {
    unsigned char ei_magic [4] ; /* Needs to be 0x7f E L F , otherwise won't work. */
    unsigned char ei_class; /* 1 for 32-bit, 2 for 64-bit (WE ARE WORKING WITH 32-BIT !!!!)*/
    unsigned char ei_data; /* 1 for little-indian, 2 for big-indian */
    unsigned char ei_version; /* Always 1*/
    unsigned char ei_os_abi; /* ABI for the target OS (should be 3 for linux)*/
    unsigned char ei_abiversion; /* abi version*/
    unsigned char ei_pad[7]; /* padding bytes (should be filled with zeros)*/
    unsigned char e_type[2];  /* object file type*/
    unsigned char e_machine[2]; /* target ISA*/
    unsigned char e_version[4]; /* same as ei_version */
    unsigned char e_entry[4]; /* memory address of the entry point where the process starts executing (IMPORTANT!!!!)*/
    unsigned char e_phoff[4]; /* pointer to start of program header table*/
    unsigned char  e_shoff[4]; /* pointer to start of section header table*/
    unsigned char e_flags[4]; /* Probably for the Eflags register (not sure...)*/
    unsigned char e_ehsize[2]; /* contains the size of this header (52) for 32-bit format*/
    unsigned char e_phentsize[2]; /* size of program header table entry*/
    unsigned char e_phnum [2]; /* number of entries in program header table*/
    unsigned char e_shentsize[2]; /* size of section header table entry*/
    unsigned char e_shnum[2]; /* nmber of entrieson section header table*/
    unsigned char e_shstrndx[2]; /* index of the section header table entry that containsthe section names*/

} elf_header_info_t;

int read_header(const uint8_t *filename);
int load_program(const uint8_t *filename, uint8_t *prog_mem);
