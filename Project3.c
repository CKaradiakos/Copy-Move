/* 					
ISCI 333. System Fundamentals
Christina Karadiakos
*/

//Project 3: Create a program that copies or moves a file or group of files to a specified path
//Source and destitation are full or relative paths
//Program must be able to extract the file or directory base name when needed
#include <stdio.h>
#include <sys/stat.h> //for stat() function
#include <string.h> //string stuff
#include<stdlib.h> //errors
#include<unistd.h> //files
#include <fcntl.h> //files
#include<dirent.h> //directory stuff
#include <errno.h> //errors

void copyFile(char *, char *, char *);
void moveFile(char *, char *, char *);
int getBufferLength(char *);

int main(int argc, char *argv[])
{
	//initial error handling (# arguments)
	if (argc < 3)
	{
		printf("You are missing arguments for the function call. Make sure to include the name of one or more files and the directory\n");
		exit(1);
	}
	else if (argc > 7)
	{
		printf("You entered too many files, please limit the number of files to 5 or less.\n");
		exit(1);
	}
	
	char mode[3]; //determines the function run, mv = move, cp = copy
	int numFiles = 0; //number of files being moved or copied, defaults to 0
	int fileFount =0; //whether or not the file being searched for was found in the directory, 
	char files[5][25]; //assuming no more than 5 files are entered (AKA max argc = 7, min = 3)
	char newDPath[15]; //path to the new directory
	char tempArg[15]; //temporary storage for argv so it's read properly

	struct dirent *pDirent; 
	DIR *directory; //pointer to current directory
	
	//ASSIGNING ARGUMENTS TO FILES OR DIRECTORY SECTION
	
	struct stat sb; //buffer to hold info from stat() function
	
	//assigning arguments
	if(strcmp(argv[0], "./copy") == 0) //copying files
	{
		strcpy(mode, "cp");
	}
	else if(strcmp(argv[0], "./move") == 0) //moving files
	{
		strcpy(mode, "mv");
	}
	else //invalid argument for argv[0]
	{
		printf("Invalid argument for argv[0]. Please call the function using './copy [...]' or './move [...]'. Exiting Program. \n");
		fflush(stdout);
		exit(1);
	}
	
	
	for( int i=1; i < argc; i++) //starts at 1 since 0 is mode (mv or cp)
	{
		strcpy(tempArg, argv[i]); //prevents a segmentation fault?
		
		stat(tempArg, &sb);
		if (S_ISREG(sb.st_mode))
		{
			//regular file handling
			//files[i-1] = argv[i];
			strcpy(files[i-1], argv[i]);
		}
		else if (S_ISDIR(sb.st_mode))
		{
			//directory path handling
			strcpy(newDPath, argv[i]);
		}
		else // file or directory not found error
		{
			printf("File/directory not found for argument %i; %s.\nNow exiting program\n", i, argv[i]);
			exit(1);
		}
	}
	numFiles = argc-2; //the first is the mode and the last is the directory
	
	//DIRECTORY OPENING/MANIPULATION SECTION
	
	//get reference to current working directory (cwd)
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	
	if (strcmp(cwd, newDPath) == 0) //new directory == cwd error
	{
		printf("The new path cannot be equal to the current working directory. Exiting program.\n");
		fflush(stdout);
		exit(1);
	}
	
	
	
	//From Lab 7 -- finding the file in current directory, do for each file
	for (int i=0; i < numFiles; i++)
	{
		//open directory
		directory = opendir(cwd);
		
		while ((pDirent = readdir(directory))!= NULL) //until the end of the directory is reached
		{		
			//finding the file to copy/move
			if(strcmp(pDirent->d_name, files[i]) == 0)
			{
				//copy/move file
				if(strcmp(mode, "cp") == 0)
				{
					copyFile(files[i], newDPath, "c");
					chdir(cwd);
				}
				else if (strcmp(mode, "mv") == 0)
				{
					moveFile(files[i], newDPath, cwd);
				}
			}
		}
		
		//close directory
		closedir(directory);
	}
	
	return 0;
}

//This function copies a file into a different directory
//char* fileName: The name of the file being copied
//char* directoryPath: Where the file is being copied to
void copyFile(char* fileName, char * directoryPath, char * type)
{
	int pid;
	int fd; //for the OG file
	int fdNew; //for the file being created
	int tempRead; //for reading
	int temp; //for writing
	DIR *nextDirectory; //pointer to other directory
	char fileBuffer[BUFSIZ] = { }; //large buffer to store the file contents
	char usrResponse[1]; //input for user response
	char tempName[25]; //name for fileName as something that isn't a pointer
	int actualBufLength;
	
	strncpy(tempName, fileName, 25);
	
		//COPYING FILE
			//open file
			fd = open(tempName, O_RDONLY); //opens file to read
			if (fd < 0) //failure to open file 	
			{ 		
				printf("Unable to open file %s to read, closing program.\n", tempName);
				fflush(stdout);
				exit(1); 	
			} 
			//copy file contents
			tempRead = read(fd, fileBuffer, BUFSIZ);
			if(tempRead == -1) //error in read
			{
				printf("Unable to read from file %s, closing program.\n", tempName);
				fflush(stdout);
				exit(1); 
			}
	
			if(tempRead == 0) //empty file
			{
				printf("The file %s is empty\n", tempName);
			}
			
			//close file
			close(fd);
		
		//WRITING TO FILE
			if ((chdir(directoryPath)) == -1) //change directories
	 		{
	 			printf("Unable to open other directory. Exiting program.\n");
	 			fflush(stdout);
	 			exit(1);
	 		} 
	 		
	 		if(access(tempName, F_OK) == 0) //checking whether the file already exists
	 		{
	 			//The file exists -- ask user for permission to continue
	 			printf("The file %s already exists in %s. Do you want to replace it? Y or N\n", tempName, directoryPath);
	 			scanf(" %c", usrResponse);
	 			
	 			if(strcmp(usrResponse, "N") == 0) //user doesn't want file replaced
	 			{
	 				return; //end function
	 			}
	 		}
	 		
	 		//(write to file in other directory)
	 		//open file -- from Lab7
	 		fdNew = open(tempName, O_WRONLY | O_CREAT, 00700); //creates file with rwx permissions for user
			if (fdNew == -1) //failure to create/open file
			{
				printf("Unable to open/create file. Closing program.\n");
				fflush(stdout);
				exit(1);
			}
	 	
	 		//paste file contents
	 		
	 		//brief detour to get actual buffer length
	 		actualBufLength = getBufferLength(fileBuffer)+1; //adding 1 since index+1=length
	 		
	 		temp = write(fdNew, fileBuffer, actualBufLength);
			if (temp == -1)
			{
				fprintf(stderr, "Error in write\n");
				exit(1);
			} 
	 		//close file
	 		close(fdNew);
	 	
	 		//close directory
	 		closedir(nextDirectory);
		
		if(strcmp(type, "c") == 0) //prevent from being printed when called for mv
		{
			printf("Successfully copied the file %s to %s \n", tempName, directoryPath);
			fflush(stdout);
		}
	
	strcpy(tempName, ""); //clearing tempName contents
	strcpy(fileBuffer, ""); //clearing fileBuffer contents
}


//This function moves a file to a different directory by copying the file and deleting the original one.
//char* fileName: The name of the file being moved
//char* directoryPath: the new directory that the file is being moved to
//char* original directory: the directory the file is being moved from.
void moveFile(char* fileName, char * directoryPath, char* originalDirectory)
{
	//call copyFile	
	copyFile(fileName, directoryPath, "d");

	//remove OG file link
	chdir(originalDirectory); //return to OG directory
	if ((unlink(fileName)) == -1)
	{
		printf("Error removing original file, exiting program.\n");
		fflush(stdout);
		exit(1);
	}

	printf("Successfully moved the file %s to %s\n", fileName, directoryPath);
	fflush(stdout);	
}

//This function retrieves the index of the actual end of the buffer file to prevent junk being printed to the new file.
//char* buffer: The buffer file whose length is being determined
//Returns the index of the last filled element in the buffer file (only works if buffer is initialized as buf[#] = { };)
int getBufferLength(char* buffer)
{
	for (int i = BUFSIZ-1; i > -1; i--)
	{
		if (buffer[i] >0)
		{
			return i;
		}
		
	}
	return -1; //filled portion not found
}

