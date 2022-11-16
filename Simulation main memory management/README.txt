
Niv Kalman
318163292

== Description ==

In the program I implemented a simulation of the access of the CPU to the memory. At each compilation, I needed to bring the specific page to the main memory. For each page I needed to check what it's permissions and where is it located. I used these details to get the page in the right place from the Exec file1 or Exec file2  or swap file. (Depending on the number of processes. ( 
In the program there are two main functions: load and store In addition to private methods that I will detail below.

==program files==
Main.cpp
Sim_mem.h
Sim_mem.cpp


== private methods==
Load - This function's goal is to load the current                                                                       page to the main memory. The function is divided into several cases. At each case I update physical address and return the main memory in the place of the physical address.
Store - This function's goal is to store the value to the page in the address. At each case, I calculate the physical address and update the value in the right place.
placeInExe – This function gets a page, its frame and process id.  and enter the page to the main memory.
notPlaceInExe - This function gets a page and check which frame I need to remove from the main memory, if the page has permission for writing and the page dirty, I copy the page to the swap file.
findFreeFrame - This function gets process id and address, and his goal is to find free frame for this page.
mainToSwap - The function gets an address and process id its purpose is to move the same page swap file from the main memory.
fullFrames - The function gets an array and process id its purpose is to check if the main memory is full
findPage - The function gets a number and process id its purpose is find which frame I need to use.
findSpaceSwap - The function searches for the first free space in the swap file.
Cmp - The function gets an array and checks if there are 5 zeros in the swap file

==input==
Function calls load and store

==Output==
Print main memory.
Print page table
print swap table

==how to compile==
compile: g++ -g main.cpp sim_mem.cpp -o Ex5
To run: ./ Ex5.cpp




