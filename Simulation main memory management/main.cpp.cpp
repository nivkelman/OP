
#include "sim_mem.h"

using namespace std;
#define MEMORY_SIZE 200
extern char main_memory[MEMORY_SIZE];


int main()
{
    char val;
    char val1;
    char val2;
    char val3;
    char val4;
    char val5;
    char val6;

    sim_mem s((char*)"exec_file" ,(char*)" " , (char*)"swap_file", 5 , 5 , 5 , 40 , 11 , 5 , 2);

    s.store(1, 11 , '!');

    s.store(1,8,'?');

    s.store(1,17,'&');

    val2 = s.load(0,1);

    s.store(1 , 20 , '@');

    s.store(1,23, 'N');

    s.store(1,25, 'P');

    s.store(1,30,'*');

    val5 = s.load(1, 20);

    s.store(2,35,'$');

    s.store(2,23, 'A');

    s.store(2,25, 'B');

    s.store(2,40,'C');

    val5 = s.load(2, 20);


    s.print_memory();
    s.print_page_table();
    s.print_swap();

}
