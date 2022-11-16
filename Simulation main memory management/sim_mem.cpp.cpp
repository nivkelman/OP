
//#include <cstdio>
//#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include "sim_mem.h"
//#include <iostream>
#include <string>
#include <fstream>
//#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <bits/stdc++.h>
using namespace std;

char main_memory[MEMORY_SIZE];
char* arrFrame;
char* newPage;
char* swaPage;
int swapSize;
int toWrite;
int toRead;
int textPage;
int frameSize;
int phyAddress = 0;
int headOfTheFrame = 0;
int index1 = 0;
int head = 0;
int full = 0;
int f = 0;

queue<int> Q_Of_Frame;   // save the frame number


sim_mem::sim_mem(char *exe_file_name1,char *exe_file_name2, char *swap_file_name2, int text_size, int data_size,
                 int bss_size,int heap_stack_size, int num_of_pages, int page_size, int num_of_process){

    this->text_size = text_size;    //initialization by the constructor
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;
    this->num_of_proc = num_of_process;

    program_fd[0] = open(exe_file_name1, O_RDONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);   // open text file (logical memory)
    if (program_fd[0] == -1) {
        perror("cant open the file\n");
        exit(1);
    }

    if(num_of_process == 2)
    {
        program_fd[1] = open(exe_file_name2, O_RDONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);   // open text file2 (logical memory)
        if (program_fd[1] == -1) {
            perror("cant open the file\n");
            exit(1);
        }
    }

    textPage = text_size / page_size;
    page_table = (page_descriptor **) malloc(num_of_pages * sizeof(page_descriptor *));   // create Two-dimensional pointer
    if (page_table == NULL)                                                                   // This table save information on the pages.
    {
        perror("cant allocate memory\n");
        exit(1);
    }

    for (int i = 0; i < num_of_process ; i++)
    {
        page_table[i] = (page_descriptor *)malloc( num_of_pages * sizeof (page_descriptor));   // Create an array according to the number of processes
        if (page_table[i] == NULL)                                                                 // This table save information on the pages.
        {
            perror("cant allocate memory\n");
            exit(1);
        }

        for (int j = 0; j < num_of_pages ; j++)   // Initialize the page table by the number of pages
        {
            page_table[i][j].V = 0;
            page_table[i][j].D = 0;
            page_table[i][j].swap_index = -1;
            page_table[i][j].frame = -1;

            if(j < textPage)
            {
                page_table[i][j].P = 0;   // read
            }
            else
            {
                page_table[i][j].P = 1;   // write
            }

        }

    }

    for(int i = 0; i < MEMORY_SIZE; i++)   // initialize the array of main memory (0)
    {
        main_memory[i] ='0';
    }

    swapfile_fd = open(swap_file_name2,  O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);   // create swap file
    if (swapfile_fd == -1) {
        perror("cant open this file\n");
    }

    swapSize = page_size * num_of_pages;
    for (int w = 0; w < swapSize; w++) {
        toWrite = write(swapfile_fd, "0", 1);   // initialization swap file (0)
        if (toWrite == -1)
        {
            perror("cant writing to the file");
            exit(EXIT_FAILURE);
        }
    }

    frameSize = MEMORY_SIZE / page_size;   // size of one frame
    arrFrame = (char *)malloc(frameSize * sizeof(char));   // create array to save frame's information
    if (arrFrame == NULL)
    {
        perror("cant allocate memory\n");
        exit(1);
    }
    for (int i = 0; i < frameSize; i++)   // reset all the frame cells (0 = empty)
    {
        arrFrame[i] = '0';
    }

    newPage = (char *)malloc(page_size * sizeof(char));   // create clean page and initialization (0)
    if(newPage == NULL)
    {
        perror("cant allocate memory\n");
        exit(1);
    }
    for (int i = 0; i < page_size; i++)
    {
        newPage[i] = '0';
    }

}


char sim_mem::load(int process_id, int address)   // load the page to the main memory
{
    int page = address / page_size;
    int offset = address % page_size;

    if(page_table[num_of_proc - 1][page].V == 1)   //  v = 1 , d = 0
    {
            phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;
            return main_memory[phyAddress];
    }

    else if(page_table[num_of_proc - 1][page].P == 1)   // page is not valid but has a permission (v = 0 , p = 1)
    {
        if (page_table[num_of_proc - 1][page].swap_index != -1)   // the page in the swap
        {
            changeThePage(num_of_proc - 1, address, page);
            return '\0' ;
        }
        if(page_table[num_of_proc - 1][page].D == 1)   // the page in the exe file
        {
            mainToSwap(num_of_proc - 1, address);   // send the address and correct process to private method
            phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;   // update the physical address
            return main_memory[phyAddress];

        }
        else   // p = 1 , d = 0 , v = 0
        {
            if(address > text_size + data_size + bss_size)
            {
                printf("Error !\n");   // first time in heap \ stack
                return '\0';
            }
            else if(address > text_size + data_size && address < heap_stack_size - bss_size ) // bss
            {
                findFreeFrame(num_of_proc - 1, address);

                page_table[num_of_proc - 1][page].D = 1;   //update the page table

            }
            else   // data
            {
                lseek(program_fd[num_of_proc - 1], page * page_size, SEEK_SET);   // get the correct place in the file
                toRead = read(program_fd[num_of_proc - 1], newPage, page_size);
                if(toRead == -1)
                {
                    perror("Error reading\n");
                    exit(EXIT_FAILURE);

                }
                findFreeFrame(num_of_proc - 1, address);
                phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;   // update the physical address
                return main_memory[phyAddress];

            }
        }
    }

    else if(page_table[num_of_proc - 1][page].P == 0)   // v = 0 , p = 0
    {
        lseek(program_fd[num_of_proc - 1], page * page_size, SEEK_SET);  // get the correct place in the file
        toRead = read(program_fd[num_of_proc - 1], newPage, page_size);
        if(toRead == -1)
        {
            perror("Error ! ");
            exit(EXIT_FAILURE);
        }

        findFreeFrame(num_of_proc - 1, address);
        phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;   // update the physical address
        return main_memory[phyAddress];
    }
}

void sim_mem::store(int process_id, int address,  char value)   // store the value to the page in the correct address
{
    int page = address / page_size;
    int offset = address % page_size;
    if (page_table[num_of_proc - 1][page].V == 1  )   // page is valid
    {
        if (page_table[num_of_proc - 1][page].P == 0)
        {
            printf("no permission for write\n");
            return;
        }
        phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;
        main_memory[phyAddress] = value;
        page_table[num_of_proc - 1][page].D = 1;

    }

    else if(page_table[num_of_proc - 1][page].P == 1)   // v = 0 (page is not valid)
    {
        if (page_table[num_of_proc - 1][page].swap_index != -1)   // the page in the swap
        {
            changeThePage(num_of_proc - 1, address, page);
            return;
        }
        if (fullFrames(num_of_proc - 1 , main_memory) == 1)   // page in the swap file
        {
            mainToSwap(num_of_proc - 1, address);
            phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;   // update the physical address
            main_memory[phyAddress] = value;

        }
        else   // p = 1 , d =0
        {
            if (page * page_size > (text_size + data_size) - 1)   // bss
            {

                findFreeFrame(num_of_proc - 1, address);   // (exe) - private method

                phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;   // updates

                main_memory[phyAddress] = value;

                page_table[num_of_proc - 1][page].D = 1;

            }

            else   // data
            {
                lseek(program_fd[num_of_proc - 1], page * page_size, SEEK_SET);
                toRead = read(program_fd[num_of_proc - 1], newPage, page_size);
                if (toRead == -1) {
                    perror("Error ! ");
                    exit(EXIT_FAILURE);
                }
                findFreeFrame(num_of_proc - 1, address);
                phyAddress = page_table[num_of_proc - 1][page].frame * page_size + offset;   // update the physical address
                main_memory[phyAddress] = value;
                page_table[num_of_proc - 1][page].D = 1;
            }
        }
    }
}


void sim_mem::print_memory()
{
    int i , j;
    printf("\n Physical memory\n");
    for (i = 0 , j = 0 ; i < MEMORY_SIZE; i++ , j++) {
        printf("%d."  "[%""c]\n", j, main_memory[i]);
    }
}
void sim_mem::print_swap() {

    char *str = (char *) malloc(this->page_size * sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size) {
        for (i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
}
void sim_mem::print_page_table() {
    int i;
    for (int j = 0; j < num_of_proc; j++) {
        printf("\n page table of process: %d \n", j);
        printf("Valid\t Dirty\tPermission\tFrame\tSwap index\n");
        for (i = 0; i < num_of_pages; i++) {
            printf("[%d]\t  [%d]\t [%d]\t [%d]\t [%d]\n",
                   page_table[j][i].V,
                   page_table[j][i].D,
                   page_table[j][i].P,
                   page_table[j][i].frame,
                   page_table[j][i].swap_index);
        }
    }
}

void  sim_mem::placeInExe(int process_id, int frame , int page)   //if the main memory not full , enter the page
{
    page_table[process_id][page].frame = frame;   // which specific frame
    arrFrame[frame] = '1';   // update frame array
    int sizeMemory = frame * page_size;
    for (int i = 0; i < page_size; i++)  // go to the right place in the main memory and copy the char's from the new page
    {
        main_memory[sizeMemory + i] = newPage[i];
    }
    page_table[num_of_proc - 1][page].V = 1;   // update page table
    Q_Of_Frame.push(frame);   // update the queue frame

}


void sim_mem::notPlaceInExe(int process_id, int page)   // if the main memory is full , enter to the swap
{
    headOfTheFrame = Q_Of_Frame.front();   // the first number in the Queue (correct frame)
    int change = 0;
    head = findPage(process_id , headOfTheFrame );

    Q_Of_Frame.pop();   // remove the number from the front of the Queue

    int sizeMemory = headOfTheFrame * page_size;
    int s = 0;

    if (page_table[num_of_proc - 1][head].P == 1 && page_table[num_of_proc - 1][head].D == 1)    // if the page has a permission, copy to swap file
    {
        change++;
        index1++;
        swaPage = (char *) malloc(page_size * sizeof(char));   // create memory for the specific page
        if(swaPage == NULL)
        {
            perror("cant allocate memory\n");
            exit(1);
        }

        for (int j = 0; j < page_size; j++)   // find free place in the swap file
        {
            swaPage[j] = main_memory[sizeMemory + j];
        }

         s = findSpaceSwap();
        lseek(swapfile_fd, findSpaceSwap() * page_size, SEEK_SET);   // get the correct place in the file
        toWrite = write(swapfile_fd, swaPage, page_size);    // copy to the swap file
        if(toWrite == -1)
        {
            perror("Error writing ! ");
            exit(EXIT_FAILURE);
        }
    }


    if(change == 1 || page_table[num_of_proc - 1][head].P == 0)
    {
        page_table[num_of_proc - 1][head].frame = -1;   // update the page table
        page_table[num_of_proc - 1][head].V = 0;
        if(page_table[num_of_proc - 1][head].P == 1)
        {
            page_table[num_of_proc - 1][head].swap_index = s ;
        }
    }

    page_table[num_of_proc - 1][page].frame = headOfTheFrame;
    page_table[num_of_proc - 1][page].V = 1;
    page_table[num_of_proc - 1][page].D = 1;

    for (int j = 0; j < page_size; j++)    // copy from new page to the main memory in the correct place
    {
        main_memory[j + sizeMemory] = newPage[j];
    }
    Q_Of_Frame.push(headOfTheFrame);   // update the Queue
}



void sim_mem::findFreeFrame(int process_id, int address)   // search free frame , if there is not send to private method
{
    int page = address / page_size;
    int placeFlag = 0;
    for (int i = 0; i < frameSize ; i++)
    {
        if (arrFrame[i] == '0')   // find free frame
        {
            full = 1;
            placeInExe(process_id , i , page);
            f++;
            placeFlag = 1;
            break;
        }
    }
    if(placeFlag == 0)
    {
        full = 0;
        notPlaceInExe(process_id , page);   //  if the main memory is full
    }
   for(int i = 0 ; i < page_size ; i++)   // reset the clean page
    {
        newPage[i] = '0';
    }
}


void sim_mem::mainToSwap(int process_id, int address)   // bring page from the main to the swap
{
    int page = address / page_size;
    int hasPlace = 0;
    lseek(swapfile_fd, page * page_size, SEEK_SET); // go to the right place in the swap file
    toRead = read(swapfile_fd, newPage, page_size);
    if(toRead == -1)
    {
        perror("Error reading");
        exit(EXIT_FAILURE);
    }
    for(int i = 0 ; i < frameSize ; i++)   // find free frame
    {
        if(arrFrame[i] == '0')
        {
            hasPlace = 1;
            placeInExe(num_of_proc , i , page);   // send the correct file , page and frame and insert the page to the main memory
            break;
        }
    }
      if(hasPlace == 0)   // if the main memory is full
      {
          notPlaceInExe(num_of_proc, page);   //send the correct file , page and frame if there is no place in the main memory
      }

    for(int j = 0 ; j < page_size ; j++)   // reset the clean page
    {
        newPage[j] = '0';
    }

}



void sim_mem:: changeThePage(int process_id , int address , int page)    // bring the page from the swap to main and from the main to swap
{
    char arrZero[page_size];
    for (int j = 0; j < page_size; j++)
    {
        arrZero[j] = '0';
    }

//    headOfTheFrame = Q_Of_Frame.front();   // the first number in the Queue (correct frame
//
//    Q_Of_Frame.pop();   // remove the number from the front of the Queue
//
//    Q_Of_Frame.push(headOfTheFrame);   // update the Queue


    lseek(swapfile_fd, page_table[num_of_proc -1][page].swap_index * page_size,SEEK_SET);   // get the correct place in the swap file

    toRead = read(swapfile_fd, swaPage, page_size);    // copy to the swap page
    if (toWrite == -1)
    {
        perror("Error writing ! ");
        exit(EXIT_FAILURE);
    }
    lseek(swapfile_fd, page_table[num_of_proc -1][page].swap_index * page_size,SEEK_SET);

    toWrite = write(swapfile_fd, arrZero, page_size);   // reset the swap in the right place
    if (toWrite == -1)
    {
        perror("Error writing ! ");
        exit(EXIT_FAILURE);
    }


    int correctPage = findPage(num_of_proc - 1 , headOfTheFrame );
    if(page_table[num_of_proc - 1][correctPage].D == 1)
    {
        int counter = headOfTheFrame * page_size;
        char arrTest[page_size];
        for (int i = 0; i < page_size; i++)
        {
            arrTest[i] = main_memory[counter];
            counter++;
        }

        lseek(swapfile_fd,findSpaceSwap() * page_size, page_size);
        toWrite = write(swapfile_fd, arrTest, page_size);
        page_table[num_of_proc - 1][correctPage].swap_index =  findSpaceSwap();

    int size5 = headOfTheFrame * page_size;
    for(int i = 0 ; i < page_size ; i++)
    {
        main_memory[i  + size5] = swaPage[i];
    }

    page_table[num_of_proc - 1][correctPage].V = 0;
    page_table[num_of_proc - 1][correctPage].frame = -1;
    page_table[num_of_proc -1][page].swap_index = -1;
    page_table[num_of_proc -1][page].frame = headOfTheFrame;
    page_table[num_of_proc -1][page].V = 1;

    }
}


int sim_mem:: fullFrames(int process_id ,char arr[]) // check if the main memory is full return 1
{
    int count = 0;
     for(int i = 0 ; i < num_of_pages ; i++)
     {
         if(page_table[num_of_proc - 1][i].V == 1)
         {
             count++;
         }
     }
     if(count == frameSize )
     {
         return 1;
     }
     return 0;

}

int sim_mem::findPage(int process_id , int head)  // find which frame I need to use
{
    for (int i = 0; i < num_of_pages; i++)
    {
        if(page_table[num_of_proc - 1][i].frame == head)
        {
            return i ;
        }
    }
    return -1;
}


int sim_mem::findSpaceSwap()   // find the free place in the swap file
{
    int num = 0;
    char space[page_size];
    char arrZero2[page_size];
    for (int j = 0; j < page_size; j++)
    {
        arrZero2[j] = '0';
    }

    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    for (int i = 0 ; i < swapSize ; i++) {
        num++;
        read(swapfile_fd, space, page_size);
        if (cmp(space) == 1) {
            return i;
        }
        lseek(swapfile_fd, num * page_size, SEEK_SET);
    }

}

int sim_mem::cmp(char space[])   // check if the swap file has place ( five 0 it free place)
{
    for (int j = 0; j < page_size; j++)
    {
       if(space[j] != '0')
           return -1;
    }
    return 1;
}







sim_mem::~sim_mem()
{
    free(swaPage);                      // free all the allocated memory
    free(newPage);
    free(arrFrame);
    for (int i = 0; i < num_of_proc ; i++)
    {
        free(page_table[i]);
    }
    free(page_table);

    for(int i = 0 ; i < num_of_proc ; i++)   // close all opened files
    {
        close(program_fd[i]);
    }
    close(swapfile_fd);

}






