#include "vga.h"
#include "lib391/ece391syscall.h"

#define EOF -1

struct missile_node {
   uint8_t data;
   struct missile_node *next;
};

struct missile_node *head = NULL;

/* Display the list */
void printList() {
	/* Set terminal output mode*/
	ece391_ioctl(STDIN, TERMINAL_IOCTL_SET_OUTPUT_MODE, 1);

	struct missile_node *current = head;
	ece391_write(STDOUT, "\n[ ", 3);

	/* Start from the beginning */
	uint8_t seperator[] = " -> ";
	while(current != NULL) {
		ece391_write(STDOUT, &current->data, 1);
		ece391_write(STDOUT, seperator, ece391_strlen(seperator));
		current = current->next;
	}
	ece391_write(STDOUT, " ]\n", 3);

	/* Clear terminal output mode*/
	ece391_ioctl(STDIN, TERMINAL_IOCTL_SET_OUTPUT_MODE, 0);
}

void freeList() {
   	struct missile_node* current = head;
    struct missile_node* nextNode;

    while (current != NULL) {
        nextNode = current->next;  		// Save the next node before freeing the current one
        ece391_free(current);           // Free the current node
        current = nextNode;        		// Move to the next node
    }
}

/* Insertion at the end */
void insert(uint8_t data){

	/* Create a link */
	struct missile_node *new_missile = (struct missile_node*) ece391_malloc(sizeof(struct missile_node));
	new_missile->data = data;
	new_missile->next = NULL;

   	/* If the linked list is empty, make the new node the head */
    if (head == NULL) {
        head = new_missile;
        return;
    }

    /* Traverse the linked list to find the last node */
    struct missile_node *current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* Update the next pointer of the last node to point to the new node */
    current->next = new_missile;
}

/* This command_t enum encodes the input keys we care about */
typedef enum { NOKEY, QUIT, LEFT, RIGHT, UP, DOWN, FIRE } command_t;

command_t get_command(void) {
	char ch;
	int state = 0;
        while ((ch = ece391_getc()) != EOF) {
		switch(ch){
		    case '`': return QUIT;
		    case ' ': return FIRE;

		/* I am a vi-junkie, so you can control the crosshair with 
		 * the h,j,k,l vi-style cursor moving keys. */
		    case 'h': return LEFT;
		    case 'j': return DOWN;
		    case 'k': return UP;
		    case 'l': return RIGHT;
		}

		/* Arrow keys send the escape sequence "\033[?", where ? is one
		 * of A, B, C, or D . We use a small state-machine to track
		 * this character sequence */
		if (ch == '\033'){
		    state = 1; 
		}else if (ch == '[' && state == 1){
		    state = 2;
		}else {
			if (state == 2 && ch >= 'A' && ch <= 'D') {
				switch (ch) {
				    case 'A': return UP;
				    case 'B': return DOWN;
				    case 'C': return RIGHT;
				    case 'D': return LEFT;
				}
			}
			state = 0;
		}
	}
	return NOKEY;
}

int main(){
	/* Initialize Screen and Input */
	init_screen();

	/* Setup terminal */
	ece391_ioctl(STDIN, TERMINAL_IOCTL_SET_OUTPUT_MODE, 0);

	/* Start game */
	draw_starting_screen();
	while(FIRE != get_command());
	clear_screen();

	uint8_t cmd;
	while('`' != (cmd = ece391_getc())) {
		if (cmd == ' ') {
			printList();
		} else {
			insert(cmd);
		}
	}

	/* Free memory */
	freeList();

	/* Restore terminal */
	ece391_ioctl(STDIN, TERMINAL_IOCTL_SET_OUTPUT_MODE, 1);

	return 0;
}
