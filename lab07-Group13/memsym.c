#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

// Output file
FILE* output_file;


// Global variables
int* memory;
int mem_size;

int r1, r2;
int current_process = 0;

TLBEntry* TLB;
int TLB_size;

PageTableEntry* page_tables[4];
int page_table_size;

int define_called = FALSE;


// Struct for Page Table Entries
typedef struct PageTableEntry {
    int isValid;
    int pfn;
    int pid;
} PageTableEntry;

// Struct for TLB Entry
typedef struct TLBEntry {
    int isValid;
    int vpn;
    int pfn;
} TLBEntry;


// TLB replacement strategy (FIFO or LRU)
char* strategy;

char** tokenize_input(char* input) {
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL) {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}


void define(int offset, int pfn, int vpn) {
    if (define_called) {
        fprintf(stderr, "Error: multiple calls to define in the same trace\n");
        exit(-1);
    }

    define_called = TRUE;

    // Initialize memory
    mem_size = 1 << (pfn * vpn);
    memory = (int*)malloc(mem_size * sizeof(int));
    memset(memory, 0, mem_size * sizeof(int));

    // Initialize Page Tables for 4 processes
    page_table_size = 1 << vpn;
    for (int i = 0; i < 4; i++) {
        page_tables[i] = (PageTableEntry*)malloc(page_table_size * sizeof(PageTableEntry));
        for (int j = 0; j < page_table_size; j++) {
            page_tables[i][j].valid = FALSE;
        }
    }

    // Initialize TLB
    TLB_size = 8; // Assuming TLB size is 8
    TLB = (TLBEntry*)malloc(TLB_size * sizeof(TLBEntry));
    for (int i = 0; i < TLB_size; i++) {
        TLB[i].valid = FALSE;
    }

    // Output initialization information
    printf("Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n", offset, pfn, vpn);
}

void ctxswitch(int pid) {
    if(pid < 0 || pid > 3) {
        fprintf(stderr, "Invalid context switch to proccess %d\n", pid);
        exit(-1);
    }

    // Save existing state
    int saved_r1 = r1;
    int saved_r2 = r2;

    // Set current process
    current_process = pid;

    // Restore saved state
    r1 = saved_r1;
    r2 = saved_r2;

        printf("Switched execution context to process %d\n", pid);

}

void map(int vpn, int pfn) {

}

void unmap(int vpn) {

}

PageTableEntry pinspect(int vpn) {

}

TLBEntry tinspect(int tlbN) {

}

int linspect(int physical_location) {

}

int rinspect(int register) {

}

void load(int dest, int src) {

}

void store(int dest, int src) {

}

void add() {

}




int main(int argc, char* argv[]) {
    const char usage[] = "Usage: memsym.out <strategy> <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 4) {
        printf("%s", usage);
        return 1;
    }
    strategy = argv[1];
    input_trace = argv[2];
    output_trace = argv[3];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    output_file = fopen(output_trace, "w");

    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez ) {
            fprintf(stderr, "Reached end of trace. Exiting...\n");
            return -1;
        } else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);

        // TODO: Implement your memory simulator

        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }

    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;
}
