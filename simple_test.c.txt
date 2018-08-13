/*
 * Usage : simple_test [ifname1]
 * ifname is NIC interface, f.e. eth0
 *
 * Derived from https://github.com/OpenEtherCATsociety/SOEM
 */
 
#include "ethercat.h"
#include <stdio.h>
#include <string.h>
#include <string.h>

void mecapion_test (char *ifname){
	
if(ec_init(ifname))
	printf("ec_init on %s succeeded. \n",ifname);

}

int main(int argc, char *argv[])
{
	
   printf("SOEM (Simple Open EtherCAT Master)\nSimple test\n");

   if (argc > 1)
   {
		mecapion_test(argv[1]);
   }
   else
   {
      printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
   }

   printf("End program\n");
   return (0);
   
}
