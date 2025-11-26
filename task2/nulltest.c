#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "Teste Null Pointer\n");
  
  int *p = 0; 
  uint val = *p; 
  
  printf(1, "FALHA CRITICA: Consegui ler o endereco 0! Valor: %x\n", val);
  exit();
}
