#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>



// TODO: Add more fields to this struct
struct job {
    int id;
    int start;
    int arrival;
    int completion;
    int length;
    int tickets;
    int remaining;
    struct job *next;
    int has_run;
    int wait;
    int turnaround;
    int response;
};

/*** Globals ***/ 
int seed = 100;

//This is the start of our linked list of jobs, i.e., the job list
struct job *head = NULL;

/*** Globals End ***/

/*Function to append a new job to the list*/
void append(int id, int arrival, int length, int tickets){
  // create a new struct and initialize it with the input data
  struct job *tmp = (struct job*) malloc(sizeof(struct job));

  //tmp->id = numofjobs++;
  tmp->id = id;
  tmp->length = length;
  tmp->arrival = arrival;
  tmp->tickets = tickets;
  tmp -> remaining = length;

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
  int tickets = 0;

  struct job **head_ptr = malloc(sizeof(struct job*));

  if( (fp = fopen(filename, "r")) == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) > 1) {
    arrival = strtok(line, ",\n");
    length = strtok(NULL, ",\n");
    tickets += 100;
       
    // Make sure neither arrival nor length are null. 
    assert(arrival != NULL && length != NULL);
        
    append(id++, atoi(arrival), atoi(length), tickets);
  }

  fclose(fp);

  // Make sure we read in at least one job
  assert(id > 0);

  return;
}




void policy_STCF(struct job *head, int slice) {
  printf("Execution trace with STCF:\n");
  int time = 0;
  int time_left = 0;
  struct job *current = head;

  //find time_left
  while (current != NULL) {
    time_left += current->length;
    current = current->next;
  }

  //execute until all jobs are done
  while (time_left > 0) {
    current = head;
    struct job *shortestJob = NULL;

    struct job *previous = NULL;
    //make shortestJobTime equal to max value so current->length can overwrite it
    int shortestJobTime = INT_MAX;

    while (current != NULL) {
      //check that arrival time is valid, and that job still has time remaining
      if (current->arrival <= time && current->remaining > 0) {
        if (current->remaining < shortestJobTime) {
          shortestJobTime = current->remaining;
          shortestJob = current;
        }
      }
      previous = current;
      current = current->next;
    }

    struct job *waitingJob = head;
    while (waitingJob != NULL) {
      if ((waitingJob -> remaining > 0) && (waitingJob -> id != shortestJob -> id) && (waitingJob -> arrival <= time)) {
        if(shortestJob -> remaining >= slice) {
          waitingJob -> wait += slice;
        }
        else {
          waitingJob -> wait += shortestJob -> remaining;
        }
      }
      waitingJob = waitingJob -> next;
    }

    if (shortestJob->remaining > slice) {
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", time, shortestJob->id, shortestJob->arrival, slice);
      if(shortestJob->has_run != 1){
        shortestJob->start = time;
      }
  
      time += slice;
      shortestJob->remaining -= slice;
      time_left -= slice;
      shortestJob->has_run = 1;
    } else {
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", time, shortestJob->id, shortestJob->arrival, shortestJob->remaining);
      if(shortestJob->has_run != 1){
        shortestJob->start = time;
      }

      time += shortestJob->remaining;
      time_left -= shortestJob->remaining;
      shortestJob->remaining = 0;
      shortestJob->completion = time;
      shortestJob->has_run = 1;
    }

  }
  printf("End of execution with STCF.\n");
  return;
}


void analyze_STCF(struct job *head) {
  // TODO: Fill this in
  int currenttime = 0;
  int response;
  float totalresponse = 0;
  int turnaround;
  float totalturnaround = 0;
  int wait;
  float totalwait = 0;
  int jobnum = 0;

  struct job * current = head;

  while(current != NULL){
    response = current->start - current->arrival;
    turnaround = current->completion - current->arrival;
    wait = current->wait;
    totalresponse += response;
    totalturnaround += turnaround;
    totalwait += wait;

    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", current->id, response, turnaround, wait);
    jobnum++;
    current = current->next;
  }

  printf("Average -- Response: %0.2f  Turnaround: %0.2f  Wait: %0.2f\n", totalresponse / jobnum, totalturnaround / jobnum, totalwait / jobnum);


  return;
}




void policy_RR(struct job *head, int slice) {
  printf("Execution trace with RR:\n");
  struct job *current = head;
  struct job *running_job = NULL;

  int remaining_work = 0;
  struct job *temp = head;

  while (temp->next != NULL) {
      remaining_work += temp->length;
      temp = temp->next;
  }
  remaining_work += temp->length;
  temp->next = head; // create circular linked list

  int time = 0;
  while (remaining_work > 0) {
    if (current->arrival <= time) {
      if (current->remaining <= 0) {
        current = current->next;
      }


      int remaining_time = current -> remaining;
      // if remaining >= time slice, run for entire time, otherwise only run for remaining time
      int run_time;
      if (remaining_time >= slice) {
        run_time = slice;
      } 
      else {
        run_time = remaining_time;
      }
      if (current->length == current->remaining) {
        current ->response = time - current->arrival;
      }

      current->remaining -= run_time;
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", time, current->id, current->arrival, run_time);

      // Calculate wait time for other jobs during the current job's execution
      struct job *waiting_job = current->next;
      while (waiting_job != current && waiting_job->arrival <= time) {
      if (waiting_job->remaining > 0) {
        waiting_job->wait += run_time;
      }
      waiting_job = waiting_job->next;
      }

      time += run_time;
      current->turnaround = time - current->arrival;
      remaining_work -= run_time;
      current = current->next;
    }
    else {
      current = current->next;
      time ++;
    }
  }
  printf("End of execution with RR.\n");
  return;
}

void analyze_RR(struct job *head) {
  struct job *current = head;
  int total_response = 0;
  int total_turnaround = 0;
  int total_wait = 0;
  float jobnum = 0;

  while (current->next != head) {
    jobnum++;
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", current->id, current->response, current->turnaround, current->wait);
    total_response += current->response;
    total_turnaround += current->turnaround;
    total_wait += current->wait;
    current = current->next;
  }
  jobnum++;
  printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", current->id, current->response, current->turnaround, current->wait);
  total_response += current->response;
  total_turnaround += current->turnaround;
  total_wait += current->wait;

  printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", total_response/jobnum, total_turnaround/jobnum, total_wait/jobnum);  
  return;
}



void policy_LT(struct job *head, int slice) {
  printf("Execution trace with LT:\n");

  // Set variables
  int totalTime = 0;
  struct job *itr = head;
  int remainingWork = 0;
  int numTickets = 0;
  int numNodes = 0;

  // Get total number of tickets and time needed
  while(itr != NULL){
    remainingWork += itr -> length;
    numTickets += itr -> tickets;
    numNodes++;
    itr = itr -> next;
  }

  // While there is still jobs that need to be run
  while(remainingWork > 0) {
    // Get a random number
    int randomNum = rand() % numTickets;
    // printf("%d\n", randomNum);

    int ticketTotal = 0;
    struct job *toRun = head;

    // Get the job to run that wins the lottery
    do {
      if (toRun -> remaining != 0) {
        ticketTotal += toRun -> tickets;
        if (ticketTotal >= randomNum) {
          break;
        }
      }
      toRun = toRun -> next;
    } while((ticketTotal < randomNum) && (toRun != NULL));

    // If this is the first time the job has run, set it's response time
    if (toRun -> length == toRun -> remaining) {
      toRun -> response = totalTime - toRun -> arrival;
    }

    // If remaining time in node < slice, run for only the remaining time
    // Otherwise run for the entire slice
    int remainingTime = toRun -> remaining;
    int runTime;
    if (remainingTime >= slice) {
      runTime = slice;
    } 
    else {
      runTime = remainingTime;
    }

    // Calculate wait time for other jobs during the current job's execution
    struct job *waitingJob = head;
    while (waitingJob != NULL) {
      if ((waitingJob -> remaining > 0) && (waitingJob -> id != toRun -> id)) {
        waitingJob -> wait += runTime;
      }
      waitingJob = waitingJob -> next;
    }

    // Print job that needs to run
    printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
      totalTime, toRun -> id, toRun -> arrival, runTime);


    // Increment time by job run time and decrement remaining work by the same value
    totalTime += runTime;
    remainingWork -= runTime;
    toRun -> turnaround = totalTime - toRun -> arrival;

    // Decrement node's remaining time by the time it ran for
    toRun -> remaining -= runTime;
    // If job has finished running, take it out of the lottery
    if(toRun -> remaining == 0) {
      numTickets -= toRun -> tickets;
    }
  }
  printf("End of execution with LT.\n");
  return;
}




void analyze_LT(struct job *head) {
  // TODO: Fill this in
  struct job *current = head;
  int totalResponse = 0;
  int totalTurnaround = 0;
  int totalWait = 0;
  float jobNum = 0;

  while (current != NULL) {
    jobNum++;
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", 
      current -> id, current -> response, current -> turnaround, current -> wait);
    totalResponse += current -> response;
    totalTurnaround += current -> turnaround;
    totalWait += current -> wait;
    current = current -> next;
  }

  printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n", 
    totalResponse/jobNum, totalTurnaround/jobNum, totalWait/jobNum);  
  return;
}




int main(int argc, char **argv) {

 if (argc < 5) {
    fprintf(stderr, "missing variables\n");
    fprintf(stderr, "usage: %s analysis-flag policy workload-file slice-length\n", argv[0]);
		exit(EXIT_FAILURE);
  }
  
  srand(time(0));

  int analysis = atoi(argv[1]);
  char *policy = argv[2],
       *workload = argv[3];
  int slice = atoi(argv[4]);

  // Note: we use a global variable to point to 
  // the start of a linked-list of jobs, i.e., the job list 
  read_workload_file(workload);

  if (strcmp(policy, "STCF") == 0 ) {
    policy_STCF(head, slice);
    if (analysis) {
      printf("Begin analyzing STCF:\n");
      analyze_STCF(head);
      printf("End analyzing STCF.\n");
    }

    exit(EXIT_SUCCESS);
  }

  if (strcmp(policy, "RR") == 0 ) {
    policy_RR(head, slice);
    if (analysis) {
      printf("Begin analyzing RR:\n");
      analyze_RR(head);
      printf("End analyzing RR.\n");
    }

    exit(EXIT_SUCCESS);
  }

  if (strcmp(policy, "LT") == 0 ) {
    policy_LT(head, slice);
    if (analysis) {
      printf("Begin analyzing LT:\n");
      analyze_LT(head);
      printf("End analyzing LT.\n");
    }

  }

  // TODO: Add other policies 

	exit(EXIT_SUCCESS);
}
