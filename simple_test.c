/*
 * Usage : simple_test [ifname1]
 * ifname is NIC interface, f.e. eth0
 *
 */
 
#include "ethercat.h"
#include <stdio.h>
#include <string.h>
#include <string.h>
/* Size of IOmap = sum of sizes of RPDOs + TPDOs */
/* Total size of RPDOs: ControlWord[32 bits] + Interpolation data record sub1[32 bits] = 64 bits
   Total size of TPDOs: StatusWord[32 bits] + Position actual value[32 bits] = 64 bits
   Therefore, number of entries of IOmap = 128 bits/8 bits per char = 64 */
char IOmap[64];

void initialize (char* ifname)
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
				/* ec_statecheck returns the value of the state, as defiend in ethercattypes.h (i.e. 4 for safe-op). 
		           In case the fisrt argument is 0, it returns the value of the lowest state of all the slaves */
				if (ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE) == EC_STATE_PRE_OP)
					printf("All slaves reached PRE_OP state\n");
				
				/* Due to a bug in EtherCAT implementation by Mecapion, we have to manually
				   enable syncmanagers 2 & 3, which are associated with TPDO and RPDOs 
				   respectively (p.26 of EtherCAT slave implementation guide). For more info,
				   refer to SOEM issue #198 */
				ec_slave[1].SM[2].SMflags |= EC_SMENABLEMASK;
				ec_slave[1].SM[3].SMflags |= EC_SMENABLEMASK;

				/* To do: - Run slaveinfo and this code without ec_config_map(&IOmap)
                          - Find out what ec_config_map does */
 				ec_config_map(&IOmap);
		}
	}
}

int main(int argc, char *argv[])
{
	
   printf("SOEM (Simple Open EtherCAT Master)\nSimple test\n");

   if (argc > 1)
   {
		initialize(argv[1]);
		
		/* Specify the desired state for the slave and then write it */
		ec_slave[1].state = EC_STATE_SAFE_OP;
		ec_writestate(1);
		
		if (ec_statecheck(1, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE) == EC_STATE_SAFE_OP)
			printf("Slave 1 reached SAFE_OP state\n");
		
		/* Line 122 of the original simple_test */
		printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n", 1, ec_slave[1].state, ec_slave[1].ALstatuscode, ec_ALstatuscode2string(ec_slave[1].ALstatuscode));
        
		/* Note that we can use SDOread/write after ec_config_init(FALSE), since init state is sufficient for SDO communication */
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
		
   }
   else
   {
      printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
   }

   printf("End program\n");
   return (0);
   
}
