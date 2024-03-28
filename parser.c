//parses the input file into Process and event
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define NEW 1
#define READY 2
#define READY_SUSPEND 3
#define BLOCKED 4
#define BLOCKED_SUSPEND 5
#define RUNNING 6
#define EXIT 7

#define MAX_PROCESSES 20

struct process {
	int name;
	int state;
	bool justChanged;
	char* queue;
};

int getProcessNum(char* processName) {
	char processNumStr[2] = "";
	processNumStr[0] = processName[1];
	processNumStr[1] = processName[2];
	int processNum = atoi(processNumStr);
	return processNum;
}

char* getStateName(int stateID) {
	char* stateName = "";
	switch (stateID) {
	case 1:
		stateName = "New";
		break;
	case 2:
		stateName = "Ready";
		break;
	case 3:
		stateName = "Ready/Suspend";
		break;
	case 4:
		stateName = "Blocked";
		break;
	case 5:
		stateName = "Blocked/Suspend";
		break;
	case 6:
		stateName = "Running";
		break;
	case 7:
		stateName = "Exit";
		break;
	default:
		printf("Error in getStateName: Input %d out of range", stateID);
	}
	return stateName;
}

// picks a random process from a set of processes in a given state
int getRandomProcessByState(struct process* processes, int state) {
	int processesInState[MAX_PROCESSES];
	int numProcessesInState = 0;
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (processes[i].state == state) {
			// set the value of the current index equal to the process name
			processesInState[numProcessesInState] = processes[i].name;
			// iterate the index
			numProcessesInState++;
		}
	}

	// no processes in the given state, return -1
	if (numProcessesInState == 0) {
		return -1;
	}

	// seed rand()
	srand(time(0));

	// select the **INDEX** of the process selected. this index is out of the set of processes in that state, NOT the set of all processes.
	// so in a set of [1, 3, 7, 12], the selectedProcessIndex '3' refers to process #12, not process #3.
	// bounded between (0, number of processes in the state - 1) AKA (0, last filled index in the processesInState array)
	int selectedProcessIndex = rand() % (numProcessesInState - 1);

	// return the randomly selected process
	return processesInState[selectedProcessIndex];
}

// returns the percentage of Blocked processes, out of all active (Ready, Running, Blocked) processes
double getBlockedPercentage(struct process* processes) {
	int activeProcesses = 0;
	int activeProcessesBlocked = 0;
	for (int i = 0; i < MAX_PROCESSES; i++) {
		// count all active processes
		if (processes[i].state == READY || processes[i].state == RUNNING || processes[i].state == BLOCKED) {
			activeProcesses++;
		}
		if (processes[i].state == BLOCKED) {
			activeProcessesBlocked++;
		}
	}

	return (double)activeProcessesBlocked / activeProcesses;
}

int main() {
	int i;
	char* rch;
	char str[500];
	char LineInFile[60][500];
	int lineP, lineQ;
	char* sch;
	char tokenizedLine[40][40];
	struct process processes[MAX_PROCESSES];
	int diskQueue[MAX_PROCESSES];
	int printerQueue[MAX_PROCESSES];
	int keyboardQueue[MAX_PROCESSES];
	
	// define all processes as empty initially
	for (int i = 0; i < MAX_PROCESSES; i++) {
		processes[i].name = 0;
		processes[i].state = 0;
		processes[i].justChanged = false;
		processes[i].queue = "";
		diskQueue[i] = 0;
		printerQueue[i] = 0;
		keyboardQueue[i] = 0;
	}

	double swapThreshold = 0.8;
	int numSwapped = 1;

	// have the user select a percentage at which to swap out processes from main memory
	printf("Select the threshold percentage at which processes will be swapped out from main memory.\n");
	printf("1 - 80%%\n2 - 90%%\n3 - 100%%\n");
	int userInput = 0;
	while (userInput < 1 || userInput > 3) {
		printf("Enter your selection (1, 2, or 3): ");
		if (scanf("%i", &userInput) <= 0) {
			printf("No number input, using default value of 80%%.\n");
			userInput = 1;
			// remove the endline character from stdin after an invalid input
			getchar();
		}
	}

	printf("Using treshold percentage: ");
	if (userInput == 1) {
		swapThreshold = 0.8;
		printf("80%%.\n");
	} else if (userInput == 2) {
		swapThreshold = 0.9;
		printf("90%%.\n");
	} else if (userInput == 3) {
		swapThreshold = 1;
		printf("100%%.\n");
	}

	// have the user select # of processes to swap out once threshold percentage reached
	printf("Select the number of processes to swap out once the threshold percentage has been reached (1 or 2): ");
	userInput = 1;
	if ((scanf("%i", &userInput) <= 0) || (userInput < 1 || userInput > 2)) {
		printf("Invalid input, using default value of 1.\n");
	}

	printf("Using number of processes swapped at each threshold: ");
	if (userInput == 1) {
		numSwapped = 1;
		printf("1.\n\n");
	} else if (userInput == 2) {
		numSwapped = 2;
		printf("2.\n\n");
	}

	FILE* fp1;
	FILE* fp2;
	fp1 = fopen("inp3.txt", "r");			//open the original input file
	fp2 = fopen("inp3_parsed.txt", "w");	//output the Process ID and event to another file. 

	lineP = 0;
	i = 0;

	printf("Started parsing...\n");

	char processName[2];
	int processNum;
	if (fgets(str, sizeof(str), fp1) != NULL) {

		lineP = 0;
		rch = strtok(str, " ");
		while (rch != NULL) {
			strcpy(LineInFile[lineP], rch);
			lineP++;
			rch = strtok(NULL, " ");
		}

		//create array that stores each processNum and state
		for (int i = 0; i < lineP; i++) {
			if (LineInFile[i][0] == 'P') {
				processNum = getProcessNum(LineInFile[i]);
				processes[processNum - 1].name = processNum;
			} else if (strcmp(LineInFile[i], "New") == 0) {
				processes[processNum - 1].state = NEW;
			} else if (strcmp(LineInFile[i], "Ready") == 0) {
				processes[processNum - 1].state = READY;
			} else if (strcmp(LineInFile[i], "Ready/Suspend") == 0) {
				processes[processNum - 1].state = READY_SUSPEND;
			} else if (strcmp(LineInFile[i], "Blocked") == 0) {
				processes[processNum - 1].state = BLOCKED;
			} else if (strcmp(LineInFile[i], "Blocked/Suspend") == 0) {
				processes[processNum - 1].state = BLOCKED_SUSPEND;
			} else if (strcmp(LineInFile[i], "Running") == 0) {
				processes[processNum - 1].state = RUNNING;
			} else if (strcmp(LineInFile[i], "Exit") == 0) {
				processes[processNum - 1].state = EXIT;
			}
		}

		// print initial states to file
		for (int i = 0; i < MAX_PROCESSES; i++) {
			if (processes[i].name == 0) {
				continue;
			}
			fprintf(fp2, "P%d %s ", (i+1), getStateName(processes[i].state));
		}
		fprintf(fp2, "\n\n");
	}

	//parse each remaining line into Process event
	//while loop with fgets reads each line
	while (fgets(str, sizeof(str), fp1) != NULL) {


		fprintf(fp2, "%s", str);

		// add additional newline if string does not contain one (matters for end of inp2)
		if (strchr(str, '\n') == NULL) {
			fprintf(fp2, "\n");
		}

		lineP = 0;
		rch = strtok(str, ":;.");					// use strtok to break up the line by : or . or ; This would separate each line into the different events
		while (rch != NULL) {
			strcpy(LineInFile[lineP], rch);			//copy the events into an array of strings
			lineP++;								//keep track of how many events are in that line
			rch = strtok(NULL, ":;.");				//needed for strtok to continue in the while loop
		}

		// decide if any processes should be suspended
		if (getBlockedPercentage(processes) >= swapThreshold) {
			for (int i = 0; i < numSwapped; i++) {
				// select a random blocked process and move it to blocked/suspend
				processNum = getRandomProcessByState(processes, BLOCKED);
				if (processNum != -1) {				
					processes[processNum - 1].state = BLOCKED_SUSPEND;
					processes[processNum - 1].justChanged = true;
				}
			}
		}

		//for each event (e.g. Time slice for P7 expires) pull out process number and event
		for (i = 1; i < lineP - 1; i++) {
			lineQ = 0;
			sch = strtok(LineInFile[i], " ");
			while (sch != NULL) {
				strcpy(tokenizedLine[lineQ], sch);		//use strtok to break up each line into separate words and put the words in the array of strings
				lineQ++;								//count number of valid elements
				sch = strtok(NULL, " ");
			}

			//tokenizedLine has the event separated by spaces (e.g. Time slice for P7 expires)

			// requests I/O, moved to blocked
			if (strcmp(tokenizedLine[1], "requests") == 0) {
				processNum = getProcessNum(tokenizedLine[0]);
				processes[processNum - 1].state = BLOCKED;
				processes[processNum - 1].justChanged = true;
				processes[processNum - 1].queue = tokenizedLine[3];
				if (strcmp(tokenizedLine[3], "disk") == 0) {
					diskQueue[processNum - 1] = processNum;
				} else if (strcmp(tokenizedLine[3], "keyboard") == 0) {
					keyboardQueue[processNum - 1] = processNum;
				} else if (strcmp(tokenizedLine[3], "printer") == 0) {
					printerQueue[processNum - 1] = processNum;
				}

			// dispatched to processor, moved to running
			} else if ((strcmp(tokenizedLine[2], "dispatched") == 0)) {
				processNum = getProcessNum(tokenizedLine[0]);

				// if process is currently suspend, first move it into main memory before dispatching it
				if (processes[processNum - 1].state == READY_SUSPEND) {
					processes[processNum - 1].state = READY;
				}
				processes[processNum - 1].state = RUNNING;
				processes[processNum - 1].justChanged = true;

			// timeout, moved to ready
			} else if (strcmp(tokenizedLine[0], "Time") == 0) {
				processNum = getProcessNum(tokenizedLine[3]);
				processes[processNum - 1].state = READY;

			// interrupt occurs, moved from blocked to ready or from blocked/suspend to ready/suspend
			} else if (strcmp(tokenizedLine[1], "interrupt") == 0) {
				processNum = getProcessNum(tokenizedLine[4]);
				if (processes[processNum - 1].state == BLOCKED) {
					processes[processNum - 1].state = READY;
				} else if (processes[processNum - 1].state == BLOCKED_SUSPEND) {
					processes[processNum - 1].state = READY_SUSPEND;
				}
				processes[processNum - 1].queue = "";
				diskQueue[processNum - 1] = 0;
				keyboardQueue[processNum - 1] = 0;
				printerQueue[processNum - 1] = 0;

			// process finishes, moved to exit
			} else {
				processNum = getProcessNum(tokenizedLine[0]);
				processes[processNum - 1].state = EXIT;
			}
			processes[processNum - 1].justChanged = true;

			// move a Ready/Suspend process to main memory if a process just terminated
			if (processes[processNum - 1].state == EXIT) {
				processNum = getRandomProcessByState(processes, READY_SUSPEND);
				if (processNum != -1) {
					processes[processNum - 1].state = READY;
					processes[processNum - 1].justChanged = true;

				}

			}
		}
		// print all states
		for (int i = 0; i < MAX_PROCESSES; i++) {
			if (processes[i].name == 0) {
				continue;
			}
			fprintf(fp2, "P%d %s", (i+1), getStateName(processes[i].state));
			if (processes[i].justChanged == true) {
				fprintf(fp2, "*");
			}
			fprintf(fp2, " ");
			processes[i].justChanged = false; // reset justChanged at end of each cycle
		}
		// print disk queue
		fprintf(fp2, "\ndisk queue:");
		for (int i = 0; i < MAX_PROCESSES; i++) {
			if (diskQueue[i] == 0) {
				continue;
			}
			fprintf(fp2, " P%d", diskQueue[i]);
		}
		// print printer queue
		fprintf(fp2, "\nprinter queue:");
		for (int i = 0; i < MAX_PROCESSES; i++) {
			if (printerQueue[i] == 0) {
				continue;
			}
			fprintf(fp2, " P%d", printerQueue[i]);
		}
		// print keyboard queue
		fprintf(fp2, "\nkeyboard queue:");
		for (int i = 0; i < MAX_PROCESSES; i++) {
			if (keyboardQueue[i] == 0) {
				continue;
			}
			fprintf(fp2, " P%d", keyboardQueue[i]);
		}
		fprintf(fp2, "\n\n");

	}

	printf("Parsing complete\n\n");

	fclose(fp1);
	fclose(fp2);

	return 0;
}