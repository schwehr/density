#include <cstdio>
#include <cstdlib>
using namespace std;
int main(void) {
  int s;
  char *c=(char *)&s;
  c[0]=0x00; c[1]=0x01; c[2]=0x02; c[3]=0x03;
  if (0x00010203==s) {printf("BIGENDIAN"   );return(EXIT_SUCCESS);}
  if (0x03020100==s) {printf("LITTLEENDIAN");return(EXIT_SUCCESS);}
  printf("ERROR!!!  I can not cope with this beast... HELP!!!!\n");
  return (EXIT_FAILURE);
}
