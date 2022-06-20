// Katia Lopez
// FAT32 Assignment

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_NUM_ARGUMENTS 4

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size



struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};
struct DirectoryEntry dir [16];


int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int32_t BPB_FATSz32;
int32_t BPB_RootClus;
int32_t currentDir;


/*
Function: LBAToOffset
Parameters: The current sector number that points to a block of data
Returns: The value of the address for that block of data
Description: Finds the starting address of a block of data given the
sector number corresponding to that data block
*/
int LBAToOffset(int32_t sector)
{
    // to handle cd ..
    if (sector == 0)
    {
        sector = 2;
    }

    return ((sector-2) * BPB_BytsPerSec) + (BPB_BytsPerSec*BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}


/*
Function: NextLB
Purpose: Given a logical block address, look up into the first FAT and return the logical
block address of the block in the file. If there is no further blocks then return -1
*/
int16_t NextLB(uint32_t sector, FILE *fp)
{
    uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector*4);
    int16_t val;
    fseek(fp, FATAddress, SEEK_SET);
    fread(&val, 2, 1, fp);
    return val;
}


/*
Function: compare
Purpose: Compare user input file name with FAT32 filename
Return true if they match, false otherwise
*/
bool compare(char *IMG_Name, char *input)
{
  char expanded_name[12];
  memset( expanded_name, ' ', 12 );

  char *token = strtok( input, "." );

  strncpy( expanded_name, token, strlen( token ) );

  token = strtok( NULL, "." );

  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }

  expanded_name[11] = '\0';

  int i;
  for( i = 0; i < 11; i++ )
  {
    expanded_name[i] = toupper( expanded_name[i] );
  }

  if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
  {
    return 1; //true
  }
  else
  {
    return 0; //false
  }
}



int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  // Create file pointer for FAT32 image
  FILE *fp = NULL;

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the command line.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    // If the user types a blank line, print another prompt and accept a new line of input.
    if (cmd_str[0] == '\n' || cmd_str[0] == ' ' || cmd_str[0] == '\t')
    {
        // will skip the rest of the code and return to top of
        // while loop statement to print prompt again
        continue;
    }

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str  = strdup( cmd_str );

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }


    // Will check what command user has entered based on token array
    // The following commands shall be supported:
    if (strcmp(token[0], "open") == 0)
    {
        /* open <filename>
        This command shall open a fat32 image. Filenames of fat32 images
        shall not contain spaces and shall be limited to 100 characters. */
        if(token[1] != NULL)
        {
            if (fp == NULL)
            {
                // open the file
                fp = fopen(token[1], "r");

                if (fp == NULL)
                {
                    printf("Error: File system image not found.\n");
                }
                else
                {
                    // fill directory array
                    fseek(fp, 11, SEEK_SET);
                    fread(&BPB_BytsPerSec, 2, 1, fp);

                    fseek(fp, 13, SEEK_SET);
                    fread(&BPB_SecPerClus, 1, 1, fp);

                    fseek(fp, 14, SEEK_SET);
                    fread(&BPB_RsvdSecCnt, 2, 1, fp);

                    fseek(fp, 16, SEEK_SET);
                    fread(&BPB_NumFATs, 1, 1, fp);

                    fseek(fp, 36, SEEK_SET);
                    fread(&BPB_FATSz32, 4, 1, fp);

                    fseek(fp, 44, SEEK_SET);
                    fread(&BPB_RootClus, 4, 1, fp);

                    currentDir = BPB_RootClus;

                    int offset = LBAToOffset(currentDir);
                    fseek(fp, offset, SEEK_SET);
                    fread(&dir, sizeof(struct DirectoryEntry), 16, fp);
                }
            }
            else
            {
                // If a file system is already opened then your program shall output:
                // "Error: File system image already open."
                printf("Error: File system image already open.\n");
            }
        }
        else
        {
            // If the file is not found your program shall output: "Error: File system image not found.".
            printf("Error: File system image not found.\n");
        }

    }
    else if (strcmp(token[0], "close") == 0)
    {
        // This command shall close the fat32 image.
        if (fp != NULL)
        {
            // file is open, so close it
            fclose(fp);
            fp = NULL; //set file equal to NULL again
        }
        else
        {
            /* If the file system is not currently open your program shall output:
            "Error: File system not open." */
            printf("Error: File system not open.\n");
        }

        /* Any command issued after a close, except for open, shall result in:
        "Error: File system image must be opened first." */
    }
    else if (strcmp(token[0], "info") == 0)
    {
        /* This command shall print out information about the file system in both hexadecimal and base 10:
        � BPB_BytesPerSec
        � BPB_SecPerClus
        � BPB_RsvdSecCnt
        � BPB_NumFATS
        � BPB_FATSz32 */

        if (fp == NULL)
        {
            printf("Error: File system image must be opened first.\n");
        }
        else
        {
            printf("BPB_BytsPerSec: %d %x\n", BPB_BytsPerSec, BPB_BytsPerSec);

            printf("BPB_SecPerClus: %d %x\n", BPB_SecPerClus, BPB_SecPerClus);

            printf("BPB_RsvdSecCnt: %d %x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);

            printf("BPB_NumFATs:    %d %x\n", BPB_NumFATs, BPB_NumFATs);

            printf("BPB_FATSz32:    %d %x\n", BPB_FATSz32, BPB_FATSz32);
        }
    }
    else if (strcmp(token[0], "stat") == 0)
    {
        /* stat <filename> or <directory name>
        This command shall print the attributes and starting cluster number of the file or directory name. */

        /* If the parameter is a directory name then the size shall be 0.
        If the file or directory does not exist then your program shall output "Error: File not found". */

        if (fp == NULL)
        {
            printf("Error: File system image must be opened first.\n");
        }
        else
        {
            int i;
            bool found = false;
            int temp = 0;
            for (i=0; i<16; i++) // directory contains 16 32-byte records
            {
                // get user input file name
                char input[13];
                strncpy(&input[0], token[1], 12);
                input[12] = '\0';

                // get each FAT32 filename
                char filename[12];
                strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
                filename[11] = '\0';

                // compare names
                found = compare(filename, input);

                if (found)
                {
                    // print all attributes
                    printf("File Size: %d\n", dir[i].DIR_FileSize);
                    printf("First Cluster Low: %d\n", dir[i].DIR_FirstClusterLow);
                    printf("DIR_ATTR: %d\n", dir[i].DIR_Attr);
                    printf("First Cluster High: %d\n", dir[i].DIR_FirstClusterHigh);
                    temp++;
                }
            }
            if (temp == 0)
            {
                printf("Error: File not found.\n");
            }
        }
    }
    else if (strcmp(token[0], "get") == 0)
    {
        // get <filename>
        // This command shall retrieve the file from the FAT 32 image and place it in your current working directory.
        // If the file or directory does not exist then your program shall output "Error: File not found".

        if (fp == NULL)
        {
            printf("Error: File system image must be opened first.\n");
        }
        else
        {
            int i;
            bool found = false;
            int temp = 0;
            for (i=0; i<16; i++) // directory contains 16 32-byte records
            {
                // get user input file name
                char input[13];
                strncpy(&input[0], token[1], 12);
                input[12] = '\0';

                // get filename
                char filename[12];
                strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
                filename[11] = '\0';

                // compare file names
                found = compare(filename, input);

                if (found)
                {
                    uint8_t value;

                    // create file where we will be writing to
                    FILE *newfp = fopen(token[1], "w");

                    int size = dir[i].DIR_FileSize;

                    // find low cluster number of file
                    int cluster = dir[i].DIR_FirstClusterLow;

                    // use LBAToOffset to find the offset
                    int fileOffset = LBAToOffset(cluster);

                    fseek(fp, fileOffset, SEEK_SET);

                    char file[size+1];

                    // store file data in char array
                    fread(file, size, 1, fp);

                    // write data to new file
                    fwrite(file, size, 1, newfp);
                    fclose(newfp);
                    temp++;
                }
            }
            if (temp == 0)
            {
                printf("Error: File not found.\n");
            }
        }
    }
    else if (strcmp(token[0], "cd") == 0)
    {
        // cd <directory>
        // This command shall change the current working directory to the given directory. Your program
        // shall support relative paths, e.g cd ../name and absolute paths

        if (fp == NULL)
        {
            printf("Error: File system image must be opened first.\n");
        }
        else
        {
            if (strcmp(token[1], "..") == 0) // handle cd ..
            {
                int i;
                for (i = 0; i < 16; i++)
                {
                    if (strncmp(dir[i].DIR_Name, "..", 2) == 0)
                    {
                        int offset = LBAToOffset(dir[i].DIR_FirstClusterLow);
                        currentDir = dir[i].DIR_FirstClusterLow;
                        fseek(fp, offset, SEEK_SET);
                        // fread(&dir[0], 32, 16, fp);
                        fread(&dir[0], sizeof(struct DirectoryEntry), 16, fp);
                    }
                }
            }
            else
            {
                int i;
                bool found = false;
                int temp = 0;
                for (i=0; i<16; i++)
                {
                    // get user input file name
                    char input[13];
                    strncpy(&input[0], token[1], 12);
                    input[12] = '\0';

                    // get filenames
                    char filename[12];
                    strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
                    filename[11] = '\0';

                    // compare names
                    found = compare(filename, input);

                    if (found)
                    {
                        int cluster = dir[i].DIR_FirstClusterLow;

                        // change the current directory
                        currentDir = cluster;

                        int offset = LBAToOffset(cluster);
                        fseek (fp, offset, SEEK_SET);

                        // update the struct
                        fread(&dir, sizeof(struct DirectoryEntry), 16, fp);
                        temp++;
                    }
                }
                if (temp == 0)
                {
                    printf("Directory not found\n");
                }
            }
        }
    }
    else if (strcmp(token[0], "ls") == 0)
    {
        // ls
        /* Lists the directory contents. Your program shall support listing "." and ".." .
        Your program shall not list deleted files or system volume names. */

        if (fp == NULL)
        {
            printf("Error: File system image must be opened first.\n");
        }
        else
        {
            // get the offset of the current directory
            int offset = LBAToOffset(currentDir);
            fseek(fp, offset, SEEK_SET);

            int i;
            for (i=0; i<16; i++) // directory contains 16 32-byte records
            {
                // get each filename
                char filename[12];
                strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
                filename[11] = '\0';

                // Only print 0x01 0x10 and 0x20
                if (((dir[i].DIR_Attr == 0x1) || (dir[i].DIR_Attr == 0x10) || (dir[i].DIR_Attr == 0x20)) && (dir[i].DIR_Name[0] != (char)0xe5))
                {
                    // print file names in given directory
                    printf("%s\n",  filename);
                }
            }
        }
    }
    else if (strcmp(token[0], "read") == 0)
    {
        // read <filename> <position> <number of bytes>
        /* Reads from the given file at the position, in bytes, specified by the position parameter and output
        the number of bytes specified. */

        if (fp == NULL)
        {
            printf("Error: File system image must be opened first.\n");
        }
        else
        {
            int position = atoi(token[2]); // position user entered
            int numOfBytes = atoi(token[3]); // number of bytes

            int i;
            bool found = false;
            int temp = 0;
            for (i=0; i<16; i++) // directory contains 16 32-byte records
            {
                char input[13];
                strncpy(&input[0], token[1], 12);
                input[12] = '\0';

                // get filename
                char filename[12];
                strncpy(&filename[0], &dir[i].DIR_Name[0], 11);
                filename[11] = '\0';

                // compare file names
                found = compare(filename, input);

                if (found)
                {
                    uint8_t value;
                    int userOffset = position;

                    // find low cluster number of file
                    int cluster = dir[i].DIR_FirstClusterLow;

                    // use LBAToOffset to find the offset
                    int fileOffset = LBAToOffset(cluster);

                    fseek(fp, fileOffset, SEEK_SET);
                    while (userOffset > BPB_BytsPerSec)
                    {
                        // Use NextLB to find next block in file
                        // if it is -1 then done
                        cluster = NextLB(cluster, fp);
                        userOffset -= BPB_BytsPerSec;
                    }

                    // fseek to the offset
                    fileOffset = LBAToOffset(cluster);
                    fseek(fp, fileOffset+userOffset, SEEK_SET);

                    int i;
                    for (i=0; i<numOfBytes; i++)
                    {
                        // read one byte at a time
                        fread(&value, 1, 1, fp);

                        // print data to screen
                        printf("%x ", value);
                    }
                    printf("\n");
                    temp++;
                }
            }
            if (temp == 0)
            {
                printf("Error: File not found.\n");
            }
        }
    }
    else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
    {
        exit(0);
    }
    else
    {
        printf("Error: Command not found. \n");
    }

    free( working_root );

  }
  return 0;
}
