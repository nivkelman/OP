//
// Created by student on 6/1/22.
//
#include <iostream>
#include <cstdio>
#include <malloc.h>
#include <unistd.h>
#include <string>
#ifndef SIM_MEM_H
#define SIM_MEM_H
#define MEMORY_SIZE 200

typedef struct page_descriptor
{
    int V; // valid
    int D; // dirty
    int P; // permission
    int frame; //the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;

class sim_mem {

    int swapfile_fd; //swap file fd
    int program_fd[2]; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    page_descriptor **page_table; //pointer to page table


public:

    sim_mem(char exe_file_name1[],char exe_file_name2[], char swap_file_name2[], int text_size,
            int data_size, int bss_size, int heap_stack_size,
            int num_of_pages, int page_size, int num_of_process);

    char load(int num_of_proc, int address);
    void store(int num_of_proc, int address , char value);
    void print_memory();
    void print_swap();
    void print_page_table();


    // private method

    void placeInExe(int num_of_proc , int frame , int page);
    void notPlaceInExe(int num_of_proc, int page);
    void findFreeFrame(int num_of_proc, int address);
    void mainToSwap(int num_of_proc , int address);
    void changeThePage(int process_id , int address, int page);
    int fullFrames(int process_id,char arr[]);
    int findPage(int process_id , int head);
    int findSpaceSwap();
    int cmp(char space[]);



    ~sim_mem();

};

#endif //SIM_MEM_H
