/*
 * Usage : simple_test [ifname1]
 * ifname is NIC interface, f.e. eth0
 *
 */
 
#include "ethercat.h"
#include <stdio.h>
#include <string.h>
#include <string.h>

void initialize (char *ifname)
{
/* See https://openethercatsociety.github.io/doc/soem/tutorial_8txt.html */	

	if (ec_init(ifname))
	{
		printf("ec_init on %s succeeded. \n",ifname);

		if (ec_config_init(FALSE) > 0)
		{		
			printf("%d slaves found and PRE_OP state requested\n", ec_slavecount);
				/* See red_test line 55 */
				/* Passing 0 for the first argument means check All slaves */
				if (ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE) > 0)
					printf("All slaves reached PRE_OP state\n");
		}
	}
}

int main(int argc, char *argv[])
{
	
   printf("SOEM (Simple Open EtherCAT Master)\nSimple test\n");

   if (argc > 1)
   {
		initialize(argv[1]);
		/* Check whether SDO read/write is successful */
		int result;
		/* Inspird by line 222 to 225 of ebox.c */
		int16 objectValue = 0x7;
		int objectSize = sizeof(objectValue);
		result = ec_SDOwrite(1,0x6040, 0x00, FALSE, objectSize, &objectValue, EC_TIMEOUTRXM);
		if (result == 0) 
		{	
			printf("SDO write unsucessful\n");
			
		}
		/* Inspired by lines 211 to 221 of slaveinfo.c */
		uint16 rdat;
		/* rdat = read data, rdl = read data length (read as past sentence of read)*/
		int rdl = sizeof(rdat); rdat = 0;
		result = ec_SDOread(1, 0x6040, 0x00, FALSE, &rdl, &rdat, EC_TIMEOUTRXM);
		if (result != 0)
		{
			printf("Value of the OD entry is %d\n", rdat);
			return 0;
		}
			
		else printf("SDO read unsucessful\n");

		/***********************************/
		
   }
   else
   {
      printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
   }

   printf("End program\n");
   return (0);
   
}
