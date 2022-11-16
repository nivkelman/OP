
Niv Kelman
318163292


==Description==
The program represents the implementation of commands in Linux in the shell system.
First, the user will enter input, the program will analyze the input and finally send it using execvp.
The program supports standard shell commands as well as pipes, & and nohup
The entire program history is saved in a separate file represented by the word history.
In order to exit a program, you must write an exit

==function==
Two functions in the program:
1 . Matrix – The function receives an array, analyzes it and finally returns each matrix.
2. sigHand - The function prevents the father from waiting for his son

==Program Files==
 main.c


==How to compile?==

compile: gcc main.c -o ex3
run: ./main.c


==Input:==
String  (command with pipes , & and nohup)
 history
 exit

==Output:==
Output of shell system
Number of char and  number of word
History file
Exit program


