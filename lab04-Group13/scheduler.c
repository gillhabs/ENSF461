#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>


// 0: no policy analysis

// TODO: Add more fields to this struct
struct job {
    int id;
    int arrival;
    int length;
    int hasRun;
    int startTime;
    int completionTime;
    // Other metadata
    struct job *next;
};

/*** Globals ***/ 
int seed = 100;

//This is the start of our linked list of jobs, i.e., the job list
struct job *head = NULL;
struct sorted_sjf *head_sjf = NULL;

/*** Globals End ***/

/*Function to append a new job to the list*/
void append(int id, int arrival, int length){
  // create a new struct and initialize it with the input data
  struct job *tmp = (struct job*) malloc(sizeof(struct job));

  //tmp->id = numofjobs++;
  tmp->id = id;
  tmp->length = length;
  tmp->arrival = arrival;

  // the new job is the last job
  tmp->next = NULL;

  // Case: job is first to be added, linked list is empty 
  if (head == NULL){
    head = tmp;
    return;
  }

  struct job *prev = head;

  //Find end of list 
  while (prev->next != NULL){
    prev = prev->next;
  }

  //Add job to end of list 
  prev->next = tmp;
  return;
}

/*Function to read in the workload file and create job list*/
void read_workload_file(char* filename) {
  int id = 0;
  FILE *fp;
  size_t len = 0;
  ssize_t read;
  char *line = NULL,
       *arrival = NULL, 
       *length = NULL;

  struct job **head_ptr = malloc(sizeof(struct job*));

  if( (fp = fopen(filename, "r")) == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) > 1) {
    arrival = strtok(line, ",\n");
    length = strtok(NULL, ",\n");
       
    // Make sure neither arrival nor length are null. 
    assert(arrival != NULL && length != NULL);
        
    append(id++, atoi(arrival), atoi(length));
  }

  fclose(fp);

  // Make sure we read in at least one job
  assert(id > 0);

  return;
}


void policy_FIFO(struct job *head) {
  printf("Execution trace with FIFO:\n");
  int currenttime = 0;
  struct job *current = head;
  while (current != NULL) {
    printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", currenttime, current->id, current->arrival, current->length);
    currenttime += current->length;
    current = current->next;
  }
  printf("End of execution with FIFO.\n");
  return;
}

void analyze_FIFO(struct job *head) {
  int currenttime = 0;
  int response;
  float totalresponse = 0;
  int turnaround;
  float totalturnaround = 0;
  int wait;
  float totalwait = 0;
  int jobnum = 0;
  struct job *current = head;
  while(current != NULL) {
    response = currenttime - current->arrival;
    wait = currenttime - current->arrival;
    currenttime += current->length;
    turnaround = currenttime - current->arrival;
    totalresponse += response;
    totalturnaround += turnaround;
    totalwait += wait;
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",current->id, response, turnaround, wait);
    jobnum++;
    current = current->next;
  }
  printf("Average -- Response: %0.2f  Turnaround %0.2f  Wait %0.2f\n", totalresponse / jobnum, totalturnaround / jobnum, totalwait / jobnum);
  return;
}


void policy_SJF(struct job *head) {
  printf("Execution trace with SJF:\n");
  //  Get the total number of nodes in the struct
  struct job *length = head;
  int numNodes = 0; 
  while(length != NULL){
    length = length -> next;
    numNodes++;
  }

  int time = 0;
  struct job *current = head;

  // Iterate through the list numNodes times to print node once
  for(int i = 0; i < numNodes; i++){
    struct job *shortestJob = head;
    // Iterate through the list
    while (current != NULL){
      // If the current node length is less than the current shortestJob length, hasn't already run, and has arrived
      if(((current -> length) < (shortestJob -> length)) && (current -> hasRun != 1) && ((current -> arrival) <= time)){
        // Make the shortestJob the current
        shortestJob = current;
      }
      // Get next node
      current = current->next;
    }

    // Print the job and mark it as having run
    printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
      time, shortestJob->id, shortestJob->arrival, shortestJob->length);
    shortestJob->hasRun = 1;

    // Put the start time and completion time in the node for analyzing purposes 
    shortestJob -> startTime = time;
    shortestJob -> completionTime = time + shortestJob -> length;

    // Update the time
    time += shortestJob -> length;

    // Reset current pointer back to head for the next iteration
    current = head;
  }
  printf("End of execution with SJF.\n");
  return;
}



void analyze_SJF(struct job *head) {
  int currenttime = 0;
  int response;
  float totalresponse = 0;
  int turnaround;
  float totalturnaround = 0;
  int wait;
  float totalwait = 0;
  int jobnum = 0;
  struct job *current = head;
  while(current != NULL) {
    response = current->startTime - current->arrival;
    wait = current->startTime - current->arrival;
    turnaround = current->completionTime - current->arrival;
    totalresponse += response;
    totalturnaround += turnaround;
    totalwait += wait;
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",
      current->id, response, turnaround, wait);
    jobnum++;
    current = current->next;
  }
  printf("Average -- Response: %0.2f  Turnaround %0.2f  Wait %0.2f\n", totalresponse / jobnum, totalturnaround / jobnum, totalwait / jobnum);


  return;
}


int main(int argc, char **argv) {

 if (argc < 4) {
    fprintf(stderr, "missing variables\n");
    fprintf(stderr, "usage: %s analysis-flag policy workload-file\n", argv[0]);
		exit(EXIT_FAILURE);
  }

  int analysis = atoi(argv[1]);
  char *policy = argv[2],
       *workload = argv[3];

  // Note: we use a global variable to point to 
  // the start of a linked-list of jobs, i.e., the job list 
  read_workload_file(workload);

  if (strcmp(policy, "FIFO") == 0 ) {
    policy_FIFO(head);
    if (analysis) {
      printf("Begin analyzing FIFO:\n");
      analyze_FIFO(head);
      printf("End analyzing FIFO.\n");
    }

    exit(EXIT_SUCCESS);
  }

  if (strcmp(policy, "SJF") == 0 ) {
    policy_SJF(head);
    if (analysis) {
      printf("Begin analyzing SJF:\n");
      analyze_SJF(head);
      printf("End analyzing SJF.\n");
    }

    exit(EXIT_SUCCESS);
  }

  // TODO: Add other policies 

	exit(EXIT_SUCCESS);
}
