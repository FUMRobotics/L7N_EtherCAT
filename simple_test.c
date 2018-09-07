/*
 * See https://github.com/OpenEtherCATsociety/SOEM/blob/master/test/linux/simple_test/simple_test.c
 * Usage : simple_test [ifname1]
 * ifname is NIC interface, f.e. eth0
 */
 
#include "ethercat.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
/* This header is necessary for using the int64 format specifier, PRId64 */
#include <inttypes.h>
/* Size of IOmap = sum of sizes of RPDOs + TPDOs */
/* Total size of RPDOs: ControlWord[16 bits] + Interpolation data record sub1[32 bits] = 48 bits
   Total size of TPDOs: StatusWord[16 bits] + Position actual value[32 bits] = 48 bits
   Therefore, number of entries of IOmap = 96 bits/8 bits per char = 12 */
char IOmap[12];

/* For defining PDOs see issue #177 of SOEM's github and line 216 of tutorial.txt */
typedef struct PACKED
{
   uint16 value_6040;
   int32 value_607A;
} drive_RPDO_t;
drive_RPDO_t* drive_RPDO;

typedef struct PACKED
{
   uint16 value_6041;
   int32 value_6064;
} drive_TPDO_t;
drive_TPDO_t* drive_TPDO;

void initialize (char* ifname, uint16 slaveNum)
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
		           In case the fisrt argument is 0, it returns the value of the lowest state among all the slaves */
				if (ec_statecheck(slaveNum, EC_STATE_PRE_OP, EC_TIMEOUTSTATE) == EC_STATE_PRE_OP)
					printf("All slaves reached PRE_OP state\n");
				
				/* Due to a bug in EtherCAT implementation by Mecapion, we have to manually
				   enable syncmanagers 2 & 3, which are associated with TPDO and RPDOs 
				   respectively (p.26 of EtherCAT slave implementation guide). For more info,
				   refer to SOEM issue #198 */
				/* For some reason, setting the enable bit puts the drive in SAFE_OP mode */
				ec_slave[slaveNum].SM[2].SMflags |= 0x00010000;
				ec_slave[slaveNum].SM[3].SMflags |= 0x00010000;

				/* We need to this just once, while we might run initialize for multiple slaves */
				if (slaveNum == 1)
				{
				/* To do: - Run slaveinfo and this code without ec_config_map(&IOmap)
                          - Find out what ec_config_map does */
 				ec_config_map(&IOmap);
				ec_configdc();
				}
		}
	}
}

void ODwrite(uint16 slaveNum, uint16 Index, uint8 SubIndex, int32 objectValue)
{
	/* Note that we can use SDOread/write and therefore ODwrite/read after ec_config_init(FALSE), since init state is sufficient for SDO communication */
	/* For checking whether SDO write is successful */
	int result;
	/* Inspird by line 222 to 225 of ebox.c */
	int objectSize = sizeof(objectValue);
	result = ec_SDOwrite(slaveNum, Index, SubIndex, FALSE, objectSize, &objectValue, EC_TIMEOUTRXM);
	//result = ec_SDOwrite(1,0x6040, 0x00, FALSE, objectSize, &objectValue, EC_TIMEOUTRXM);
	if (result == 0) 
		printf("SDO write unsucessful\n");
}

int32 ODread(uint16 slaveNum, uint16 Index, uint8 SubIndex)
{
	/* Usage : int val = ODread(1, 0x6040, 0x00);
			   printf("%d", val);
	/* Note that we can use SDOread/write and therefore ODread/write after ec_config_init(FALSE), since init state is sufficient for SDO communication */
	/* For checking whether SDO write is successful */
	int result;
	/* Inspired by lines 211 to 221 of slaveinfo.c */
	uint16 rdat;
	/* rdat = read data, rdl = read data length (read as past sentence of read)*/
	int rdl = sizeof(rdat); rdat = 0;
	result = ec_SDOread(slaveNum, Index, SubIndex, FALSE, &rdl, &rdat, EC_TIMEOUTRXM);
	if (result != 0)
	{
		printf("Value of the OD entry is %d\n", rdat);
		return rdat;
	}
			
	else 
	{	
		printf("SDO read unsucessful\n");
		return 0;
	}
}

void storeAllParams(uint16 slaveNum)
{
	/* Stores all current OD entries to EEPROM so they're not lost after restarting the drive */
	/* See page 66 of EPOS3 EtherCAT application notes, or page 85 of EPOS4s */
	ODwrite(slaveNum, 0x01010, 0x00, 0x65766173);
	
}

void restoreDefParams(uint16 slaveNum)
{
	/* Sets all parametrs to default value */
	/* See page 66 of EPOS3 EtherCAT application notes, or page 85 of EPOS4s */
	ODwrite(slaveNum, 0x01011, 0x00, 0x64616F6C);
	
}

void switchOn_enableOp(uint16 slaveNum)
{
	/* Set bits 0,1,2 and 3 of ControlWord to 1. */
	/* See page 68 of the Mecapion manual, "State Machine Control Commands" */
	ODwrite(slaveNum, 0x6040, 0x00, 15);
	
}

void faultReset(uint16 slaveNum)
{
	
	/* Set bit 7 of ControlWord to 1. */
	/* See page 68 of the Mecapion manual, "State Machine Control Commands" */
	ODwrite(slaveNum, 0x6040, 0x00, 128);
	
}

void setModeCSP(uint16 slaveNum)
{
	
	/* Set index 0x6060 to 8 for cyclic synchronous position mode */
	/* See page 174 of the manual */
	ODwrite(slaveNum, 0x6060, 0x00, 8);
}

void stateRequest(uint16 slaveNum, uint8 reqState)
{
	
	/* Specify the desired state for the slave and then write it */
	switch (reqState)
	{
		case 1:
			char state[] = "init";
			break;
		case 2:
			char state[] = "pre-op";
			break;
		case 4:
			char state[] = "safe-op";
			break;
		case 8:
			char state[] = "op";
			break;
		default:
			printf("Requested state is not valid\n");
			return;
	}
				
	ec_slave[slaveNum].state = reqState;
	ec_writestate(slaveNum);
		
	if (ec_statecheck(slaveNum, Reqstate, EC_TIMEOUTSTATE) == reqState)
	{
		if (slaveNum == 0 )
			printf("All slaves reached %s state\n", state);
		else
			printf("Slave %d reached %s state\n", state);
	}
	else
		printf("Not all slaves reached %s state\n", state);
	
}

void readState(uint16 slaveNum)
{
	/* Without this pause, SOEM will not detect an alarm */
	usleep(EC_TIMEOUTRXM);
	/* Line 122 of the original simple_test */
	ec_readstate();
	printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n", slaveNum, ec_slave[slaveNum].state, ec_slave[slaveNum].ALstatuscode, ec_ALstatuscode2string(ec_slave[slaveNum].ALstatuscode));
}


int main(int argc, char *argv[])
{
	
   printf("SOEM (Simple Open EtherCAT Master)\nSimple test\n");

   if (argc > 1)
   {
		initialize(argv[1], 1);
		readState(1);
		/* Total size of slave 1 TPDOs, in bytes */
		slave_1_TPDO_size = ec_slave[1].Ibytes;
		int i, j, chk;
		/* According to issue #177, we first create a structure and then map it to ec_slave[1].inputs/outputs */
		/* Here we define drive_RPDO as a pointer to drive_RPDO_t, and assign it a value equal to ec_slave[1].outputs */
		/* drive_RPDO = (drive_RPDO_t*) ec_slave[1].outputs;
		drive_TPDO = (drive_TPDO_t*) ec_slave[1].inputs; */
		
		ec_slave[0].state = EC_STATE_OPERATIONAL;
        /* send one valid process data to make outputs in slaves happy*/
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        /* request OP state for all slaves */
        ec_writestate(0);
		/* According to ETG_Diagnostics_with_EtherCAT document, each successful write to the slave's memory (RPDO, outputs in SOEM)
		   increases the WKC by 2 and sucessful read (TPDO, inputs in SOEM) increments it by 1. */
		expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
        chk = 40;
        /* wait for all slaves to reach OP state */
		/* Note: After the drive has entered OP state, we need to continuously send process data, usually at 100 ms intervals.
		   Otherwise, the drive will give Alarm 67. See EtherCAT error 0x001B. */
        do
        {
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
        }
        while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
        if (ec_slave[0].state == EC_STATE_OPERATIONAL )
        {
			printf("Operational state reached for all slaves.\n");
			for(i = 1; i <= 10000; i++)
			{
				
				ec_send_processdata();
				wkc = ec_receive_processdata(EC_TIMEOUTRET);
					
					/* If RPDOs were sucessfully written to the slave and TPDO were written to EtherCAT frame. */
					/* In other words, we check whether a slave couldn't read from/write to the incoming frame */
					if(wkc >= expectedWKC)
					{
						
						/* Write every byte of TPDOs of slave 1 in one line */
						for(j = 0 ; j < slave_1_TPDO_size; j++)
						{
							/* ec_slave[1].inputs is a pointer to the first byte of slave 1 TPDOs.
							   Therefore, each time we increment the address and then dereference it in order to write that byte */
							/* Now, suppose the printed line is 
							   21 02 a9 5a 46 05
							   and we have status word (0x6041) and position actual value (0x6064) as TPDOs of slave 1.
							   Thus, 
							   0x6041 = 0x0546
							   0x6064 = 0x5AA90221
							*/
							printf(" %2.2x", *(ec_slave[1].inputs + j));
						}
						/* Move the active position to the beginning of the line, so that the next line is overwritten on
						   the current one */
						printf("\r");
					}
					
				/* Sleep for 5 milliseconds */	
				osal_usleep(5000);
			}
		
		
		}
		/* If, after running the loop for 40 times, not all slaves have reached OP state, */
		else
		{
			printf("Not all slaves reached operational state.\n");
			ec_readstate();
			
			for(i = 1; i<=ec_slavecount ; i++)
				readState(i);
		}
		
		printf("\nRequest init state for all slaves\n");
        ec_slave[0].state = EC_STATE_INIT;
		/* request INIT state for all slaves */
        ec_writestate(0);
		
	else
	{
		printf("Usage: simple_test ifname1\nifname = eth0 for example\n");
    }

   printf("End program\n");
   return 0;
   
}
