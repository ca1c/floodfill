#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NUM_POSSIBLE_CHARS 128

bool arrContains(char * charArr, char val) {
    int size = strlen(charArr);

    for(int i = 0; i < size; i++) {
        if(charArr[i] == val) {
            return true;
        }
    }

    return false;
}

int findCharIndex(char * charArr, char val) {
    int size = strlen(charArr);

    for(int i = 0; i < size; i++) {
        if(charArr[i] == val) {
            return i;
        }
    }

    return -1;
}

int findMaxIndex(int * intArr) {
    int size = sizeof(intArr) / sizeof(intArr[0]);

    int max = intArr[0];
    int maxIndex = 0;
    for(int i = 0; i < size; i++) {
        if(intArr[i] > max) {
            max = intArr[i];
            maxIndex = i;
        }
    }

    return maxIndex;
}

int main() {

    FILE* fptr;

    fptr = fopen("description.txt", "r");

    if (fptr == NULL) {
        printf("The file is not opened. The program will "
              "now exit.");
        exit(0);
    }

    char c;
    char chars[NUM_POSSIBLE_CHARS];
    int counts[NUM_POSSIBLE_CHARS];
    int ci = 0; 

    while(!feof(fptr)) {
        c = fgetc(fptr);

        if(!arrContains(chars, c)) {
            chars[ci] = c;
            counts[ci] = 1;
            ci++;
        }
        else {
            counts[findCharIndex(chars, c)]++;
        }

        printf("%c", c);
    }

    int maxIndex = findMaxIndex(counts);
    int mostFrequentCharacter = chars[maxIndex];
    int numEncounters = counts[maxIndex];

    printf("\n");
    printf("The Most Frequent Char: %c, encountered %d times.", mostFrequentCharacter, numEncounters);

    return 0;
}