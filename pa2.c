#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "sstr.h"
#include "scounter.h"
#ifdef _OPENMP
#include <omp.h>
#endif


//Globals
struct scounter* scounter;
struct sstr* myString;
int i, M, N, L, extracaptured;
int failsafe = 1;
FILE* filePointer;


/* Input : Takes a char which will identify the thread and be the char it appends to myString
*  Postcondition: Does work on global variables, when thread is finished execution myString.done should be >0 and scounter should be set to the correct value
*/ 
void *myFunc (char id);

/* Input :Takes global variables from myString and scounter
*  Output:File with sstr and scounter output and information to the stdout
*/ 
void writeToFile();

/* Input :input parameters are i N M L c0 c1 c2
*  Postcondition: Created the string and verified the contents, and printed the information to stdout and a file
*/ 
int main (int argc, char *argv[]){
	//printf("Main called\n");
	srand((unsigned) time(NULL)); //This is to seed random with the clock time when called
  	if (argc != 8) { //check we have enougth parameters
		printf("Not enough parameters, expected i N M L c0 c1 c2 \n");
		return 0;
	}
	//grab the initial parameters
	i = atoi(argv[1]);
	N = atoi(argv[2]);
	L = atoi(argv[3]);
	M = atoi(argv[4]);

	//Now check that those parameters are well formed
	if ( (i>3 || i<0) || (N>8||N<3) ) { //checking the i and N parameters are in bounds
		printf("i or N parameter incorrectly initialized \n");
		return 0;}
	if (M < 0 || L < 0 ) { //checking the M and L parameter
		printf("M or L parameter incorrectly initalized \n");
		return 0;}

	char language[8] = "abcdefgh";
	char * check = memchr(language, *argv[5],N); //check c0 Exists in N
	if (check == NULL ) {
		printf("c0 not in language \n");
		return 0;
	}
	check = memchr(language, *argv[6], N); //check c1 Exists in N
	if (check == NULL) {
		printf("c1 not in language \n");
		return 0;
	}
	check = memchr(language, *argv[7], N); //check c2 Exists in N
	if (check == NULL) {
		printf("c2 not in language \n");
		return 0;
	}
	
	//This section will initalize some more variables
	
	//Do some work to create the file
	filePointer = fopen("write.txt", "w+");
    if (filePointer == NULL) {
        printf("Write.txt doesn't exist -- cannot show output");
        return 0;}

	extracaptured = M%N; //this will be the counter for the remainder of the division of M / N so that each thread does similar work
	myString = malloc(sizeof(struct sstr)); 
	scounter = malloc(sizeof(struct scounter));
	scounter_create(scounter);
	sstr_create(myString, M, L, *argv[5],*argv[6],*argv[7]); //as a remainder the argvs here are the c0, c1, c2

	//Now we will check for satisfiablity. If not we simply turn our failsafe on and allow all threads to print. 
	if (N == 3) {
		if (i == 0) {
			if (L % 2 == 1) {// F0 cannont be satisfied on odd inputs as the occurence needs to be divisible by too for the equality to hold
				printf("F0 cannot be satisfied\n");		failsafe = 0;
			}
		}
		else if (i == 1) { //F1 is satisfable once we have enough characters to place, if L is 1 there just is no way to balance the equality
			if (L == 1){
				printf("F1 cannot be satisfied\n");		failsafe = 0;
			}
		}
		else if (i == 2) {}	//Left this here as a remainder that i == 2 is always satisfiable on good inputs
		else if (i == 3) {
			if (L % 2 == 1) { //as with F0 this case is impossible on odd inputs as the occurences can not be modified to divide evenly
				printf("F3 cannot be satisfied\n");		failsafe = 0;
			}
		}
	}
	
	int thread;
	# pragma omp parallel for num_threads(N)
	for ( thread = 0; thread<N; thread++ ) {
		//printf("thread: %d", thread);
		myFunc(language[thread]);
	}

	//there is invisible pragma omp barrier here so when we get down to this point the for loop has completely finished execution

	//this will write out output since we're done
	writeToFile();

	//release variables
	free(myString);
	free(scounter);
	return 0;
}
void * myFunc(char id) {
	int myIndex;
	int sleeptime = 100000;
	int* numOfC; //[i] = {0,0,0};
    
	numOfC = (int*)malloc(sizeof(int)*3);
	numOfC[0] = 0; numOfC[1] = 0; numOfC[2] = 0;
    char myChar = id; //
	

	do {
		sleeptime = 100000 + (4000*(rand()%101)); //usleep is in microseconds so we generate a number betwen 100000 and 500000 
		usleep(sleeptime);
        if ((*myString).done) { break;	} // this is here in case the string finished while we were alseep
		//property checks
		if (N == 3) {
			if (failsafe) {
				sstr_count(myString, numOfC);
				//printf("\nThread %c numOfC is : %d %d %d ", myChar, numOfC[0],numOfC[1],numOfC[2]);
				if (i == 0) {
					if (myChar != (*myString).c2 && (numOfC[0]+numOfC[1]) <= numOfC[2]) {sstr_append(myString, myChar);} //if we aren't c2 and there are more c2s than c1+c0 write c1 or c0
					else if (myChar == (*myString).c2 && (numOfC[0]+numOfC[1]) >= numOfC[2]) {sstr_append(myString, myChar);} //if we are c2 and there are less c2s than c1+c0 write c2
				}
				else if (i == 1) {
					if (L%2 == 0) {		
						if (myChar ==(*myString).c0&& numOfC[0] <= numOfC[2]) {sstr_append(myString, myChar);} //if we aren't c2 and there are more c2s than c0 write c0
						else if (myChar == (*myString).c2 && numOfC[0] >= numOfC[2]) {sstr_append(myString, myChar);} //if we are c2 and there are less c2s than c0 write c2
					}
					else {
						if (myChar ==(*myString).c0&& numOfC[0] < ( ((L+1)/2) -2) ) {sstr_append(myString, myChar);} //For each segment there is a # of c0 that we need = to ( ((l+1)/2) -2) so write c0 until we have that many
						else if (myChar == (*myString).c2 && numOfC[2] < ((L+1)/2) ) {sstr_append(myString, myChar);} //For c2 the amount is 2 larger or ((l+1)/2)
						else if (myChar ==(*myString).c1&& numOfC[1] == 0) {sstr_append(myString, myChar);} //and we need 1 c1, also since these requirements are based on the length of a segment and N=3 they properties are enforced
					}
				}
				else if (i == 2) {
					//basically this condition allows the first one of c0 or c1 to write for that section
					if (myChar ==(*myString).c0 && numOfC[1] == 0) {sstr_append(myString, myChar);}
					else if (myChar ==(*myString).c1 && numOfC[0] == 0) {sstr_append(myString, myChar);}
				}
				if (i == 3) {
					//this case basically boils down to F0 again except we just don't allow c1 to be written and thus the property is enforced
					if (myChar ==(*myString).c0&& numOfC[0] <= numOfC[2]) {sstr_append(myString, myChar);}
					else if (myChar ==(*myString).c2&& numOfC[2] <= numOfC[0]) {sstr_append(myString, myChar);}
				}
			}else{
				sstr_append(myString, myChar);
			}
		}
		else { //in the case where N>3 we can trivially satisfy all conditions by never putting characters that would violate the condition as all conditions are valid on 0
			if (!(myChar ==(*myString).c0|| myChar ==(*myString).c1|| myChar == (*myString).c2)) {
				sstr_append(myString, myChar);
			}
		}
		//printf("help im stuck, %c \n", myChar);
	} while (!(*myString).done);

	//clear numOfC for the new counts

	int mySegCount = 0;	//the numvber of segments this thread should count
	if (extracaptured) { //if no extra remains to be captured this value will be 0 and evaluate false
		mySegCount ++; 
		extracaptured --;
	}	//the remainder is distributed over the threads with each thread picking up 1 of the remainder until none remain.

	mySegCount += M/N; //grab the regular chunk of segments to check
	int x; //x is the segment counter
	for (x = 0; x < mySegCount; x++) {
		sstr_count(myString, numOfC);
		//printf("\nThread %c numOfC is : %d %d %d ", myChar, numOfC[0],numOfC[1],numOfC[2]);
		
		//this section increments the counter if the condition is met
		if (i == 0) {
			if (numOfC[0] + numOfC[1] == numOfC[2]) {
				scounter_increment(scounter); }
		}
		else if (i == 1) {
			if (numOfC[0] + (2*numOfC[1]) == numOfC[2]) {
				scounter_increment(scounter); }
		}
		else if (i == 2) {
			if (numOfC[0] * numOfC[1] == numOfC[2]) {
				scounter_increment(scounter); }
		}
		else if (i == 3) {
			if (numOfC[0] - numOfC[1] == numOfC[2]) {
				scounter_increment(scounter); }
		}
	}
	return NULL;
}


void writeToFile() {
	//grab some global variables
    int stringLength = (*myString).index;
    int verifiedSegments = scounter_get(scounter);
	int unverifiedSegments = M-(scounter_get(scounter));
	//write to the file
    fwrite((*myString).charArray , stringLength, 1 , filePointer);
    fwrite("\n", 1, 1, filePointer);
    fprintf(filePointer, "%d", verifiedSegments);
    //write to stdout
    printf("\nFinal Concatenated String: ");
    for(int in = 0; in < stringLength; in++){
		if (in%L == 0) { printf(" "); } //Makes the terminal output more readable by putting spaces in front of segments
        printf("%c", (*myString).charArray[in]);
	}	
	printf("\nTest case for property %d.\nNumber of verified segments: %d. \nNumber of unverified segments: %d. \n", i, verifiedSegments, unverifiedSegments);
}

struct sstr * sstr_create(struct sstr *self, int M, int L, char c0, char c1, char c2) {
    (*self).index = 0; // current index in the char array
    (*self).M = M ; // number of segments
    (*self).L = L; // length of each segment
    (*self).maxLentgh; // max length of the string
    (*self).checkedsegments = 0;
    (*self).currentsegments = 0;
    (*self).done = 0; // completion flag
    (*self).c0 = c0;
    (*self).c1 = c1;
    (*self).c2 = c2;
	(*self).charArray = (char*) malloc(sizeof(char)*M*L);
}

void sstr_count(struct sstr *self, int *returnArray){
    int i = 0;
    //clear the input array
    returnArray[0] = 0;
    returnArray[1] = 0;
    returnArray[2] = 0;
	# pragma omp critical
    if (!(*self).done){
        //check current segment
        for (i = (*self).currentsegments * (*self).L; i < (*self).index; i++){ //for the start index of the current segment to the current index that hasnt been written to, check
            //if the character matches one of the condition characters, increment that portion of the array
            if ((*self).charArray[i] == (*self).c0)
                returnArray[0]++;
            else if ((*self).charArray[i] == (*self).c1)
                returnArray[1]++;
            else if ((*self).charArray[i] == (*self).c2)
                returnArray[2]++;
        }
    }else{
		for (i = (*self).checkedsegments * (*self).L; i < ((*self).checkedsegments+1) * (*self).L; i++) {
			//if the character matches one of the condition characters, increment that portion of the array
            if ((*self).charArray[i] == (*self).c0)
                returnArray[0]++;
            if ((*self).charArray[i] == (*self).c1)
                returnArray[1]++;
            if ((*self).charArray[i] == (*self).c2)
                returnArray[2]++; 
        }
        (*self).checkedsegments++;
    }
}

void sstr_append(struct sstr *self, char myChar){
    # pragma omp critical
    if (!(*self).done){
        (*self).charArray[(*self).index] = myChar; //write the char to the current index
        (*self).index++; //increment index
        if ((*self).index % (*self).L == 0) //these checks are for if we're done the current segment, then if we're done the whole string
            (*self).currentsegments++;
            if ((*self).currentsegments == (*self).M)
                (*self).done++; //we use done as a bool value, so any value above 0, which is the initial, is true
    }
}

//these methods should be self explanitory and are almost unnecessary except for information hiding and object orientedness
void scounter_create(struct scounter *self) {
    (*self).counter = 0;
}
int scounter_get(struct scounter *self) {
    return (*self).counter;

}
void scounter_increment(struct scounter *self) {
    # pragma omp atomic 
    (*self).counter ++;
}





