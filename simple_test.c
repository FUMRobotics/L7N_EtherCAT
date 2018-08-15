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
		/* Lines 211 to 221 of slaveinfo.c */
		int controlVal = 0; uint16 rdat;
		rdl = sizeof(rdat); rdat = 0;
		int controlVal = ec_SDOread(1, 0x6040, 0x00, FALSE, &rdl, &rdat, EC_TIMEOUTRXM);
		printf(%d, controlVal);
		/***********************************/
		
   }
   else
   {
      printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
   }

   printf("End program\n");
   return (0);
   
}
