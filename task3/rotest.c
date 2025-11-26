#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "Task 3 Test: Tentando escrever \n");
  
  char *p = (char*)main;
  
  *p = 'F'; 
  
  printf(1, "FALHA: Escrevi no codigo!\n");
  exit();
}
