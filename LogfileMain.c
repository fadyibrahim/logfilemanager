//Fady Ibrahim 2017,03,30
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include "logfilemanagerfunctions.h"

int main(){
	char name[max_size_input]; //input string from user
	char logfilename[16];
	logfilename[0]=0;
	int logexist=0; //used for resource clean up to prevent un-initialized logfile error 
	struct logfile logfilemanager;
	puts("Welcome to the file system manager; Please input a command....\nAcceptable commands are: appendlog, dumplog, and quit....\nNote: Filenames and text messages must be between 3-15 characters and 0-29 characters respectively....\n");
	//compares first four bytest of quitcommand to stdin. If it equals quit then exit program
	while(strcmp(fgets(name,max_size_input,stdin),quitcommand)!=0)
	{
		//first condition ensures position is in the front of the string, second condition ensures spelling is correct 
		if(strncmp(name,appendcommand,strlen(appendcommand))==0 && strstr(name,appendcommand)!=NULL )
		{
			char * fileopt=strstr(name,fileoption), *textopt=strstr(name,textoption); //beginning pointers to ' -f ' and ' -t ' substring
			if (fileopt!=NULL && textopt!=NULL )
			{
				int indexofoptf, indexofoptt;
				indexofoptf = (int)(fileopt - name); //gets the position of the beginning of the fileopt substring
				indexofoptt = (int)(textopt - name);
				fileopt=&fileopt[4]; //reassigns the beginning of the fileopt string four characters to the right to get rid of the ' -t '
				if(indexofoptf<indexofoptt && (indexofoptt-(indexofoptf+4))<=15 &&(indexofoptt-(indexofoptf+4))>=3){ //ensures that -f comes before -t and that -f is between 3 to 15 characters
					char *textmess = &textopt[4];
					int sizeoftext=30; //size of 29 characters plus the terminating null 
					//leng is the size of the string plus the terminating null charcter
					int len = strlen(textmess);
					if(len<=sizeoftext) //check to see if the entered text is between 0 - 29 characters
					{
						if(strlen(logfilename)==0) //logfile has not yet been initialized
						{
							memcpy(logfilename, fileopt, indexofoptt-indexofoptf-4);
							logfilename[indexofoptt-indexofoptf-4] = '\0';
							//strncpy(logfilename,fileopt,indexofoptt-indexofoptf-4); //copies the name of the log file using the pointer to -f
							//logfilename[strlen(logfilename)]='\0'; //adds a terminating null 
							init_logfile(&logfilemanager);
							logexist=1;
							printf("Created log file name: %s ....\n",logfilename);

							if(strstr(name,auxoption)!=NULL) // ' [-a] ' option not specified
							{
								appendrecord(&logfilemanager, init_record(textopt, logfilemanager.totrecnum[0],logfilemanager.totrecnum[1],1));
								checksumlog(&logfilemanager);
								puts("....Record successfully appended...\n");
							}
							else // ' [-a] ' option specified
							{
								appendrecord(&logfilemanager, init_record(textopt, logfilemanager.totrecnum[0],logfilemanager.totrecnum[1],0));
								checksumlog(&logfilemanager);
								puts("....Record successfully appended...\n");
							}
						
						}
						else
						{
							char * templogfilename=strstr(name,logfilename);
							int lengthlog =strlen(logfilename);
							char holderlogfilename[16];
							if (templogfilename!= NULL) //checks to see if the substring of the file name exists within the input string 
							{
								if (lengthlog == (indexofoptt-indexofoptf-4)) //ensures that the file name length from the user input is the same number as the saved file name length 
								{
									if(strstr(name,auxoption)!=NULL)
									{
										appendrecord(&logfilemanager, init_record(textopt, logfilemanager.totrecnum[0],logfilemanager.totrecnum[1],1));
										checksumlog(&logfilemanager);
										puts("....Record successfully appended...\n");
									}
									else
									{
										appendrecord(&logfilemanager, init_record(textopt, logfilemanager.totrecnum[0],logfilemanager.totrecnum[1],0));
										checksumlog(&logfilemanager);
										puts("....Record successfully appended...\n");
									}
								}
								else
								{
									puts("Error: Specified file name does not exist....\nExtra characters found....\nNote: becareful of any accidental ' ' characters \n ");
								}
							}
							else
							{
								puts("Error: Specified file does not exist...\nUse the same casing and characters....\nNote: becareful of any accidental ' ' characters \n");
							}
						}
					}
					else
					{
						puts("Error: Text message is too large....\nOnly 29 characters allowed....\n");
					}
				}
				else
				{
					puts("Error: Either specify -f 'fileoption' before -t 'textoption' or logfile name is not between 3 - 15 characters...\nExample: appendlog -f exampfile -t examptext \n");
				}
			}
			else
			{
				puts("Error: Either -f 'fileoption' or -t 'textoption' was not specified....\nExample: appendlog -f exampfile -t examptext \n");
			}
		}
		else if(strncmp(name,dumpcommand,strlen(dumpcommand))==0 && strstr(name,dumpcommand)!=NULL){ //dumplog command was given
			char * fileopt=strstr(name,fileoption);
			if (fileopt!=NULL) //ensure that the file option exists
			{
				fileopt=&fileopt[4];
				fileopt[strlen(fileopt)-1]='\0';
				//check to see if filename exists if it doesnt output error
				if(strlen(logfilename)!=0)
				{
					if (strcmp(logfilename, fileopt)==0) //comapares saved file name to inputted file name 
					{
					dumplogcommand(&logfilemanager,logfilename);
					}
					else
					{			
						puts("Error: File name does not exist....\n");
					}
				}
				else
				{
					puts("Error: File has not been created....\n");
				}
			}
			else
			{
				puts("Error: No -f option specified....\nExample: dumplog -f exampfile....\n");
			}
		}
		else
		{
			puts("Error: Invaild command...\nOnly  'appendlog, quit, dumplog' are accepted....\n");
		}
	}
	if(logexist!=0) //logfile exists then clean resources
	{
		cleanup(&logfilemanager); //cleaning: while loop to release all malloc resources
	}
	puts("...Now exiting log file system manager....\n");
}