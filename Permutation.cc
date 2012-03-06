/* 
 * File:   Permutation.cc
 * Author: Annie
 *
 * Created on April 21, 2010, 10:17 PM
 */

#include <stdlib.h>
#include <cstdio>
#include <iostream>
/*
 * 
 */
using namespace std;
char ** allPermutation;

void PrintPermutation(char * out){
    static int counter = 0;
    strcpy(allPermutation[counter],out);
    counter++;
    cout<<out<<endl;
}

void DoPermute(char in[],char out[],int used[],int length, int recursLev ){
    int i;
 
    /*Base Case*/
    if (recursLev==length){
        PrintPermutation(out);
    }

    /*Recursive Case*/
    for (i =0;i<length;i++){
     if (used[i]){
         continue;
     }

     out[recursLev] = in[i];
     used[i] = 1;
     DoPermute(in,out,used,length,recursLev+1);
     used[i]=0;
     
    }
}

void initializeAllPermutation(char * incomingString){
    //generate the number of permutation
    int noOfPermutation = 1, j =0;
    for (j=0;j<strlen(incomingString);j++){
        noOfPermutation = noOfPermutation*(j+1);
    }
    allPermutation = new char*[noOfPermutation];
    for (j=0;j<noOfPermutation;j++){
        *(allPermutation+j) = new char[strlen(incomingString)];
    }
}

int main(int argc, char** argv) {
    if (argc<2){
        cout<<"USAGE ./Permutation [STRING]"<<endl;
    }

    initializeAllPermutation(argv[1]);

    int length, i , *used;
    char *out;

    length = strlen(argv[1]);
    out = (char *) malloc (length+1);
    if(!out){
        return 0;/*failed*/
    }

    /*printf doesnt run past the end of the buffer*/
    out[length] = '\0';
    used = (int *) malloc (sizeof(int)*length);
    if (!used){
        return 0;/* failed*/
    }

    /*start with no letters used, so zero array*/
    for (i =0;i<length;i++){
        used[i]=0;
    }

    DoPermute(argv[1],out,used,length,0);
    free(out);
    free(used);

    return (EXIT_SUCCESS);
}

