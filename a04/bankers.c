/*
--------------------------------------------------
Project: CP386-assignment 4
File:    bankers.c
Author:  Daner Yasin     | 190773490
         Jacob Goldman   | 190509690
Version: 2021-07-26
--------------------------------------------------
*/

/*  IMPORTS  */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/*  FUNCTION DECLARATIONS   */

typedef struct customer
{
    int *allocation;
    int *max;
    int *need;
    int isFinished;
} customer;

int safety();
int request(int id, int *rq);
int release(int id, int *rl);
void status(int n, int m);
int isdigit();
int *arraySplitter(char *line);
int *fileStats();
int lineLength(char *line);
int compareArrays(int *array1, int *array2);
int run(int n, int m, int *available);
int n; //   number of processes
int m; //   number of resources
struct customer *customers;
int customerCount = 0;
int available[4] = {10, 5, 7, 8};
int isExit = 0;                          //  exit command has been inputted [0 = not entered, 1 = has been entered] -> exit loop and program
const int inputArray[4] = {10, 5, 7, 8}; //  for testing

/*
   used for finding number of processes and resources in the sample file
   Returns [n, m] || [processes, resources] 
*/
int *fileStats()
{
    static int stats[2]; //  returns n and m from sample file in list type-> [n, m] || [rows, columns] || [processes, resources]
    int n = 1;           //  number of processes (number of lines in file)
    int m = 0;           //  number of resources (number of columns in file)
    char c;              //  for storing each char read from file

    FILE *fp;

    fp = fopen("sample4_in.txt", "r");

    if (fp == NULL)
    {
        printf("File: sample4_in.txt does not exist");
        return 0;
    }

    for (c = getc(fp); c != EOF; c = getc(fp))
    {
        if (c == '\n') //  if current char in file is a new line -> n++ (row++)
            n++;

        if (n == 1 && isdigit(c)) //  if first line and char is a digit (not ,) and not new line char -> m++ (column++)
            m++;
    }

    fclose(fp);

    stats[0] = n;
    stats[1] = m;

    return stats;
}

/*
    function to create a list of all the customers, each customer contains max, allocation, and need
*/
int fileToCustomer()
{
    //create file, as of right now it will only read from sample4_in.txt
    FILE *fp = fopen("sample4_in.txt", "r");

    if (fp == NULL)
    {
        printf("File: sample4_in.txt does not exist");
        return 0;
    }

    //read the file once to figure out the amount of customers
    char line[128];

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        customerCount++;
    }

    //go back to the start of the file using fseek
    fseek(fp, 0, SEEK_SET);
    int index = 0;

    customers = malloc(sizeof(struct customer) * customerCount);

    //read the relevant info from the file, attach it to a customer, and finally append the customer to the customer list
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        struct customer customer;

        customer.max = arraySplitter(line);
        customer.allocation = malloc(sizeof(int) * 4);
        customer.need = arraySplitter(line);
        customer.isFinished = 0;
        customers[index] = customer;
        index++;
    }
    fclose(fp);
}

/*
    function to turn lines read from the file into an array
*/
int *arraySplitter(char *line)
{
    //create a copy of the input line
    char *copy = strdup(line);
    char *token;

    //initialize an array to store the values
    int *values = malloc(sizeof(int) * 4);
    int index = 0;

    //for each item in line fill the values array with the correct value
    while (token = strsep(&copy, ","))
    {
        *(values + index) = atoi(token);
        index++;
    }

    return values;
}

/*
    helper method to sum the values in an int array
    ex:
        [1,2,3] -> returns 6
*/
int sumArrayItems(int *array)
{
    int sum = 0;

    if (array == NULL)
    {
        printf("PASSED ARRAY IS NULL");
        return -1;
    }

    for (int i = 0; i < (sizeof(array) / sizeof(int)); i++)
    {
        sum += array[i];
    }

    return sum;
}

/*
    sums the values of two arrays and returns the summed array
    ex:
        [1,1,1] <- array 1
      + [2,2,2] <- array 2
      ---------
      = [3,3,3] <- the return array 
*/
int *sumTwoArrays(int *array1, int *array2)
{
    int arrayLength = (sizeof(array1) / sizeof(int));
    int *sumArray = malloc(sizeof(int) * arrayLength);

    for (int i = 0; i < arrayLength; i++)
    {
        sumArray[i] = array1[i] + array2[i];
    }

    return sumArray;
}

/*
    safety algorithm for bankers algorithm
*/
int safety()
{
    //  (step 1) -> init and set work = available
    int isSafe = 1; //  true or false for if the system is safe or not
    int *work;
    work = (int *)malloc(sizeof(int) * m); //  work array to be set to available array
    int isFound = 0;                       //  true or false for if found tmp

    for (int i = 0; i < m; i++) //  loop to init work array to = available
    {

        *(work + i) = available[i];
    }

    for (int i = 0; i < n; i++) //  init finish[i] to false
    {
        customers[i].isFinished = 0;
    }

    for (int i = 0; i < n; i++) //  (step 2) -> find processes to set to true for finish[i]
    {

        //  sum of need[i] array and work array to be used for if statement
        //int workSum = sumArrayItems(work);
        //int needSum = sumArrayItems(customers[i].need);

        if (customers[i].isFinished == 0 && compareArrays(customers[i].need, work) == 1) //  (step 3) -> if the customer process in the customer array is false and need[i] <= work
        {
            //  work = work + allocation[i] -> finish[i] = true (1)
            for (int k = 0; k < 4; k++)
            {
                work[k] += customers[i].allocation[k];
            }
            //work = sumTwoArrays(work, c_process.allocation);
            customers[i].isFinished = 1; //  customer process set to true
        }
    }

    int x = 0;
    for (int i = 0; i < n; i++)
    {
        if (customers[i].isFinished == 0)
        {
            isSafe = 0;
        }
    }

    return isSafe; //  return final 0 or 1 depending on safe state
}

/*
    for requesting resources (remember threads cannot request more than maximum number of resource for that thread)
    the command would fill the allocation array.
    
    return 1 if successful, 0 if unsuccessful
*/
int request(int id, int *rq)
{
    //check if request is <= need
    //check if request is <= available
    //then change available
    //then check if it is safe
    //if it is safe, change allocation and need
    //if its not safe, revert the change to available
    if (compareArrays(rq, customers[id].need) == 1 && compareArrays(rq, available) == 1)
    {
        for (int i = 0; i < 4; i++)
        {
            available[i] -= rq[i];
        }
    }
    else
    {
        printf("State is not safe, request will not be satisfied");
        return 0;
    }

    //check if the updated arrays satisfy the safety algorithm
    int isSafe = safety();

    if (isSafe == 1)
    {
        printf("state is safe, and request will be satisfied");
        for (int i = 0; i < 4; i++)
        {
            customers[id].allocation[i] += rq[i];
            customers[id].need[i] -= rq[i];
        }

        return 1;
    } // if the system isn't in a safe state, revert the changes to the available, allocation, and need arrays
    else
    {
        printf("\n\nState is not safe, request will not be satisfied\n\n");
        for (int i = 0; i < 4; i++)
        {
            available[i] += rq[i];
        }
        return 0;
    }

    //isSafe = 1 if the request was granted, otherwise the function has already returned 0
    return isSafe;
}

/*
    checks for an element of array1 is <= array 2
    
    returns 1 if found, 0 otherwise
*/
int compareArrays(int *array1, int *array2)
{
    int valid = 0;

    for (int i = 0; i < 4; i++)
    {
        if (*(array1 + i) <= *(array2 + i))
        {
            valid = 1;
        }
    }

    return valid;
}

/*
    for releasing resources and data structures will update accordingyly

    returns 1 if successful, 0 if not successful
*/
int release(int id, int *rl)
{
    int released = 1;
    for (int i = 0; i < 4; i++)
    {
        if (rl[i] > customers[id].allocation[i])
        {
            released = 0;
        }
    }
    if (released == 1)
    {
        for (int i = 0; i < 4; i++)
        {
            customers[id].allocation[i] -= rl[i];
            available[i] += rl[i];
        }
    }
    return 1;
}

/*
    output the current state of the available, maximum, allocation, and need arrays
*/
void status(int n, int m)
{ // print the available array
    printf("\n");
    printf("Available:\n");

    for (int i = 0; i < 4; i++)
    {
        printf("%d ", available[i]);
    }

    printf("\nMaximum: ");
    //print the max array
    for (int i = 0; i < 5; i++)
    {
        printf("\n");
        for (int k = 0; k < 4; k++)
        {
            printf("%d ", customers[i].max[k]);
        }
    }

    //print the allocation array
    printf("\nAllocation: ");
    for (int i = 0; i < 5; i++)
    {
        printf("\n");
        for (int k = 0; k < 4; k++)
        {
            printf("%d ", customers[i].allocation[k]);
        }
    }

    //print the need array
    printf("\nNeed: ");
    for (int i = 0; i < 5; i++)
    {
        printf("\n");
        for (int k = 0; k < 4; k++)
        {
            printf("%d ", customers[i].need[k]);
        }
    }

    printf("\n");

    return;
}

/*
    prints the intro when starting program
*/
void printIntro(int n, int m, int *available)
{
    printf("Number of Customers: %d\n", n);
    printf("Currently Available resources:");

    for (int i = 0; i < m; i++)
    {
        printf(" %d", available[i]);
    }

    printf("\nMaximum resources from file:");
    for (int i = 0; i < n; i++)
    {
        printf("\n");
        for (int k = 0; k < m; k++)
        {
            printf("%d ", customers[i].max[k]);
        }
    }
    printf("\n");
}

/*
    adds support for user input for unique inputted commands
*/
void runCommands(int n, int m, int *available)
{
    /*
    LIST OF COMMANDS:
    
    RQ      -   for requesting resources (remember threads cannot request more than maximum number of resource for that thread)
                the command would fill the allocation array.
                the customer would use ‘RQ’ to request for the resources for all the customers/thread
                ex: RQ 0 3 1 2 1

    RL      -   for releasing resources and data structures will update accordingly
                ex: RL 4 1 2 3 1

    Status  -   would  output the current state of the available, maximum, allocation, and need arrays

    Run     -   find the safe sequence and run each thread
                The command would execute the safe sequence based on the current state and all the threads would be run same function code and prints

    Exit    -   used to exit the loop and the program

    */

    while (isExit != 1) //  for each user input
    {
        char line[128];
        printf("\nEnter command: ");
        char *userInput = fgets(line, 128, stdin);

        /*
            input: RQ[command] 0[int customer num] 1[int resource 1] 0[resource 2] 0[resource 3] 1[resource 4]
                                                    ^^  int array of size m
        
        */

        char *item = strtok(userInput, " ");
        char *command = item; //  first item in the user input -> the command to be followed (Ex: 'Run')

        int x = 0;
        int i = 0;
        int *resources = malloc(sizeof(int) * m); //  resourceArray of size m (num resources)
        int customerNum;

        while (item != NULL) //  for each token in string (each item in entered command:: 'RQ'<token 0> '0'<token 1> '1'<token 2>)
        {
            if (x > 0) //  item 0 = command, 1 = customer Num, 2+ resources
            {
                //  numbers present in input -> get numbered values by converting str to int

                char *endPtr;
                int number = strtol(item, &endPtr, 10);
                //printf("iterated number: %d\n", number);

                if (x == 1) //  customerNum
                {
                    customerNum = number;
                }
                else if (x > 1) //  resource array
                {
                    resources[i] = number;
                    //printf("resources[%d] = %d\n", i, resources[i]);
                    i++; //  next resource index to be filled
                }
            }
            item = strtok(NULL, " ");
            x++;
        }

        if (strcmp(command, "Exit\n") == 0) //  exit command entered
        {
            //printf("Exit has been entered\n");
            isExit = 1; //  exit command has been entered -> exit loop and program
        }
        else if (strcmp(command, "Run\n") == 0) //  Run command entered
        {
            //printf("Run has been entered\n");
            run(n, m, available);
        }
        else if (strcmp(command, "Status\n") == 0) //  Status command entered
        {
            //printf("Status has been entered\n");
            status(n, m);
        }
        else if (strcmp(command, "RQ") == 0) //  RL command entered
        {
            //printf("RQ has been entered\n");
            request(customerNum, resources);
        }
        else if (strcmp(command, "RL") == 0) //  RL command entered
        {
            //printf("RL has been entered\n");
            release(customerNum, resources);
        }
    }
}

/*
    find the safe sequence and run each thread.
    The command 'Run' would execute the safe sequence based on the current state and all the threads would be run same function code and prints.

    Returns 1 if successful, 0 if unsuccessful
*/
int run(int n, int m, int *available)
{
    return 1;
}

/*
    driver code
*/
int main(int argc, char *argv[])
{
    //make sure the user has provided argv
    if (argc == 1)
    {
        printf("Available resources not given.\n");
        return 0;
    }

    //create processes array (available) from argv
    int processes[argc - 1];

    for (int i = 0; i < argc - 1; i++)
    {
        char *c;
        int num = strtol(argv[i + 1], &c, 10);
        processes[i] = num;
    }

    //  gets number of processes (n) and number of resources (m) from sample4_in.txt
    int *stats;
    stats = fileStats();
    n = stats[0];
    m = stats[1];

    fileToCustomer();
    printIntro(n, m, processes);
    runCommands(n, m, processes);
}
