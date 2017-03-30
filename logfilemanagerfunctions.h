//Fady Ibrahim 2017,03,30
//include all standard libraries used 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

//declare global constants
const char quitcommand[]="quit\n"; //must place \n to be able to compare to fgets output
const char appendcommand[]="appendlog ";
const char dumpcommand[]="dumplog ";
const char auxoption[]=" [-a] "; 
const char fileoption[]=" -f "; //defined with spaces to take out attached input -ttextmessage must be -t textmessage
const char textoption[]=" -t "; 
const int max_size_input= 130; //double the expected size for safety

//define structures of logfile and record
struct logfile{
	unsigned char totrecnum[2];
	unsigned short spare;
	struct record *recordlogheader;
	unsigned char checksum;
};
struct record{
	unsigned char recordnum[2];
	unsigned char auxdst;
	unsigned char timestamp[4];
	unsigned short spare;
	unsigned char text_message[30]; //size 29 bytes plus one byte for null terminator
	unsigned char checksum;
	struct record *nextrecordpointer;
};

/*
	Appends a dynamically allocated record struct to a logfile struct 
	@params: logfilemanager is a pointer to the logfile struct, recordtoappend is a pointer to a record struct 
*/
void appendrecord(struct logfile *logfilemanager, struct record *recordtoappend)
{
		//increment the previous total record number stored in the logfile struct
		int templogtotnum=(logfilemanager->totrecnum[0]<<8) + (logfilemanager->totrecnum[1]);
		templogtotnum++;
		//shift int templogtotnum 8 bits to the right then mask it 0xFF to get most significant byte in the least address then store it in the logfile struct's total record number
		logfilemanager->totrecnum[0]=(int)((templogtotnum>>8)&0xFF);
		logfilemanager->totrecnum[1]=(int)(templogtotnum&0xFF);
	if (logfilemanager->recordlogheader==NULL)
	{
		//append the first record onto the logfile 
		logfilemanager->recordlogheader=recordtoappend;
	}
	else{
		//append the next record onto the end of the first record file
		struct record *temprecordp=logfilemanager->recordlogheader;
		while(temprecordp->nextrecordpointer!=NULL)
		{
			//below changes the value of the pointer to point to the next record struct
			temprecordp= temprecordp->nextrecordpointer;
		}
		//below changes the member of the structure to re-assigning it to point to then new allocation in memory "appending the record to the logfile".
		temprecordp->nextrecordpointer=recordtoappend;
	}
}

/*
	Space pads the text message from the input then stores it into the recordfile
	@params: recordfile is a pointer to the record struct, inputstring is the user input string from fgets
*/
void init_text_message(struct record *recordfile, char inputstring[])
{
		  
  char *textmess = &inputstring[4];
  //length is the size of the string plus the terminating null charcter
  int len = strlen(textmess);
  // plus one byte for null terminatng character
  int sizeoftext=30;
  //store the text message string from the input into the record
  strncpy(recordfile->text_message,textmess,len); 
  //copies string until terminating character then paddes the rest with ascii space until 30 
  //at thirty it adds a null terminating character
  for(int i =0, templen=len ; i<= sizeoftext-len;i++, templen++){
	  if(i!=sizeoftext-len){
	  recordfile->text_message[templen]=' ';
	  }
	  else 
	  {
		  //if templen was switched to i it would cause an error because the terminating null would be placed at 17 not at 30
		  recordfile->text_message[templen]='\0';
	  }
  }  
}

/*
	Initializes time and stores it in the record
	@params: recordfile is a pointer to the record struct, auxset is the variable that holds the bool if [-a] was specified 
*/
void init_timestamp(struct record *recordfile,int auxset)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	//bitwise operation to set aux and dst by shifting aux seven bits to the right then OR it with a 6-bit shifted isdst
	recordfile->auxdst=auxset<<7;
	recordfile->auxdst=recordfile->auxdst |(tm.tm_isdst << 6);
	unsigned long calc= ((tm.tm_year+1900)-2000)*365*24*60*60 +(tm.tm_yday)*24*60*60 + (tm.tm_hour)*60*60+ tm.tm_min*60 +tm.tm_sec;
	//store long in a 4 byte char array using shifting and masking. Store using Big endian, most significant byte in smallest address. 
	recordfile->timestamp[0]=(int)((calc >> 24) & 0xFF); //most significant byte
	recordfile->timestamp[1]=(int)((calc >> 16) & 0xFF);
	recordfile->timestamp[2]=(int)((calc >> 8) & 0xFF);
	recordfile->timestamp[3]=(int)(calc & 0xFF); //least significant byte
}

/*
	Initializes logfile struct
	@params: logfile is a pointer to the logfile struct that will be initialized 
*/
void init_logfile(struct logfile *logfile)
{
	//sets total record number to zero
	logfile->totrecnum[0]= 0;
	logfile->totrecnum[1]= 0;
	logfile->spare=0;
	logfile->recordlogheader=NULL;
	
}

/*
	Calculates checksum for record then stores the value in the checksum member 
	@params: checksumrec is a pointer to the record struct that will store the calculated checksum
*/
void checksumrecord(struct record *checksumrec)
{
	//calculates checksum by adding the sums of all the least significant bytes to the ones complement of their sum 
	char tempsum = checksumrec->text_message[28] + checksumrec->spare + checksumrec->auxdst + checksumrec->timestamp[3] + checksumrec->recordnum[1];
	char onecomp = ~tempsum; //ones complement 
	checksumrec->checksum = tempsum + onecomp;
}

/*
	Initializes a recordfile with the given inputstring
	@params: inputstring the fgets input string from the user,  upperbyte of the total record number from the logfile,
	lowerbyte of the total record number from the logfile, auxset is the bool containing if [-a] was specified
	@return: returns a pointer to an allocated record struct
*/
struct record *init_record(char inputstring[], char upperbyte,char lowerbyte, int auxset)
{
	//change the total record number from bytes to int then increment it 
	int temprecnum=(upperbyte<<8) + (lowerbyte);
	temprecnum++;
	struct record *setrecord= malloc(sizeof(struct record));
	//check to make sure there is memory given for dynamic allocation
	assert(setrecord!=NULL);
	//change record number back into byte form and store the value, in big endian, in the recordnum member 
	setrecord->recordnum[0]=(int)((temprecnum>>8)&0xFF);
	setrecord->recordnum[1]=(int)(temprecnum&0xFF);
	setrecord->spare=0;
	//set the pointer to the next record to equal null, so it can be assigned later if another record is appended 
	setrecord-> nextrecordpointer=NULL;
	//add the timestamp in seconds as well as the aux and dst to the record 
	init_timestamp(setrecord,auxset);
	//add the message from the input string into the record
	init_text_message(setrecord,inputstring);
	//calculate checksum
	checksumrecord(setrecord);
	return  setrecord;
}

/*
	Iterates through the log file records and prints out the time, auxdst, and message 
	@params: logfiledump is a pointer to the logfile struct that will be used to print out appended records
*/
void dumplogcommand(struct logfile *logfiledump,char logfilename[])
{
	struct record *temprecordp=logfiledump->recordlogheader;
	int totalrecnum = (logfiledump->totrecnum[0]<<8) + (logfiledump->totrecnum[1]);
	long int convertbytearraytolong;
	printf("\n....Printing out all %d record(s) of log file %s....\n",totalrecnum,logfilename);
	
	while(temprecordp->nextrecordpointer!=NULL)
		{
			convertbytearraytolong= (temprecordp->timestamp[0] << 24) + (temprecordp->timestamp[1] << 16) + (temprecordp->timestamp[2] << 8) + temprecordp->timestamp[3];
			printf("Record sequence number: %d....\n", (temprecordp->recordnum[0]<<8) + temprecordp->recordnum[1]);
			if((temprecordp->auxdst & 0x80)== 0x80)
			{
				puts("Aux is set for this record....");
			}
			else
			{
				puts("Aux was not set for this record....");
			}
			if((temprecordp->auxdst & 0x40)== 0x40)
			{
				puts("It was day light saving time, when the record was appended....");
			}
			else
			{
				puts("It was not day light saving time, when the record was appended....");
			}		
			printf("Time in seconds is %ld....\n",convertbytearraytolong);			
			if((temprecordp->checksum & 0xFF)== 0xFF)
			{
				puts("Record passed validation....");
			}
			else
			{
				puts("Record did not pass validation....");
			}
			printf("Text message: %s\n", temprecordp->text_message);
			temprecordp= temprecordp->nextrecordpointer;
		}
	convertbytearraytolong= (temprecordp->timestamp[0] << 24) + (temprecordp->timestamp[1] << 16) + (temprecordp->timestamp[2] << 8) + temprecordp->timestamp[3];
	printf("Record sequence number: %d....\n", (temprecordp->recordnum[0]<<8) + temprecordp->recordnum[1]);
	if((temprecordp->auxdst & 0x80)== 0x80)
	{
		puts("Aux is set for this record....");
	}
	else
	{
		puts("Aux was not set for this record....");
	}
	if((temprecordp->auxdst & 0x40)== 0x40)
	{
		puts("It was day light saving time, when the record was appended....");
	}
	else
	{
		puts("It was not day light saving when the record was appended....");
	}			
	printf("Time in seconds is %ld....\n",convertbytearraytolong);	
	if((temprecordp->checksum & 0xFF)== 0xFF)
	{
		puts("Record passed validation....");
	}
	else
	{
		puts("Record did not pass validation....");
	}	
	printf("Text message: %s\n", temprecordp->text_message);
}

/*
	Cleans up any potential memory leaks by freeing any allocated resources
	@params: cleanlogfile is a pointer to the logfile struct that will iterate and release allocated record resources
*/
void cleanup(struct logfile *cleanlogfile)
{
	puts("Cleaning all allocated resources...\n");
	//if there is only one record, clean only the pointer to the allocated record struct
	if((cleanlogfile->recordlogheader)->nextrecordpointer==NULL)
	{
		free(cleanlogfile->recordlogheader);

	}else{
		struct record *temprecordp=cleanlogfile->recordlogheader;
		int converttointorecnum= (cleanlogfile->totrecnum[0] << 8) + cleanlogfile->totrecnum[1];
		
		// if there is more than one record then iterate through the records and free each allocated record struct 
		for(int i =0; i<converttointorecnum-1;i++)
		{
			while (((temprecordp->nextrecordpointer)->nextrecordpointer)!=NULL)
			{
				temprecordp=temprecordp->nextrecordpointer;
			}
			free(temprecordp->nextrecordpointer); //free allocated record struct
			temprecordp->nextrecordpointer=NULL; //must set the next pointer to equal null because the free() function does not set it to null 
			//release resources from the record linked list from the right to the left (higher sequence number to lower sequence number) through nested loops
			temprecordp=cleanlogfile->recordlogheader; 
		}
		free(cleanlogfile->recordlogheader);//last step, release head pointer to the first allocated record struct
		puts("Cleaning finished...\n");
	}
}

/*
	Calculates checksum for logfile then stores the value in the checksum member 
	@params: checksumlog is a pointer to the logfile struct that will take the sum of all the bytes within logfile and store it in the checksum member
*/
void checksumlog(struct logfile *checksumlog)
{
	//calculates checksum by adding the summs of all the least significant bytes (all record checksums) to the ones complement of their sum 
	struct record *temprecordp=checksumlog->recordlogheader;
	char tempsum=0;
	//iterate to go to the next record file while adding the checksum of that record file to the totalsum(represented by tempsum)
	while(temprecordp->nextrecordpointer!=NULL)
	{
		tempsum = tempsum + temprecordp->checksum;
		temprecordp= temprecordp->nextrecordpointer;
	}
	tempsum = tempsum + temprecordp->checksum +checksumlog->spare + checksumlog->totrecnum[1]; //add all the least significant bytes from logfile to the checksum
	char onecomp = ~tempsum;//ones complement of checksum
	checksumlog->checksum = tempsum + onecomp;// store the sum into the checksum member of logfile
}