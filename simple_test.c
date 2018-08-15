
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
sjvd[pojvp[ojpojswvdojdspoj


   {
		mecapion_test(argv[1]);
		/* Inspird by line 222 to 225 of ebox.c */
		int16 objectValue = 0x7;
		int objectSize = sizeof(objectValue);
		ec_SDOwrite(1,0x6040, 0x00, FALSE, objectSize, &objectValue, EC_TIMEOUTRXM);
		/* Inspired by lines 211 to 221 of slaveinfo.c */
		int controlRead = 0; uint16 rdat;
		int rdl = sizeof(rdat); rdat = 0;
		controlRead = ec_SDOread(1, 0x6040, 0x00, FALSE, &rdl, &rdat, EC_TIMEOUTRXM);
		printf("%d", controlRead);
		/***********************************/
		
   }
   else
   {
      printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
   }

   printf("End program\n");
   return (0);
   
}
