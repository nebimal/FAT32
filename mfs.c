#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>
#include <stdint.h>

#define WHITESPACE " \t\n"   
#define MAX_COMMAND_SIZE 255    

#define MAX_NUM_ARGUMENTS 32    

FILE *fp;
int isOpen = 0;

char     BS_OEMName[8];
char     BS_VolLab[11];
int8_t   BPB_SecPerClus;
int8_t   BPB_NumFATs;
int16_t  BPB_BytsPerSec;
int16_t  BPB_RsvdSecCnt;
int16_t  BPB_RootEntCnt;
int16_t  BPB_ExtFlags;
int16_t  BPB_FSInfo;
int32_t  BPB_FATSz32;
int32_t  BPB_RootClus;

int32_t  FatSize = 0;
int32_t  ClusterSize = 0;
int32_t  RootDirSectors = 0;
int32_t  FirstDataSector = 0;
int32_t  FirstSectorofCluster = 0;
int32_t  RootAddress = 0;



struct  __attribute__ ((__packed__)) DirectoryEntry {
char         DIR_Name[11];
uint8_t      DIR_Attr;
uint8_t      Unused1[8];
uint16_t     DIR_FirstClusterHigh;
uint8_t      Unused2[4];
uint16_t     DIR_FirstClusterLow;
uint32_t     DIR_FileSize;
};
struct DirectoryEntry dir[16];

void openFile(char *filename);
void closeFile();
void ls();
void info();
void stat(char *filename);
void readFile(char *filename, int position, int numBytes, char *option);
void put(char *filename, char *newfilename);
void get(char *filename, char *newfilename);
void del(char *filename);
void undel(char *filename);
void cd(char *directory);


int main(int argc, char * argv[] )
{
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    printf ("mfs> ");
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
    char *token[MAX_NUM_ARGUMENTS];
    int token_count = 0;                                 
    char *argument_pointer;                                                                 
    char *working_string  = strdup( command_string );                
    char *head_ptr = working_string;
    

    while ( ( (argument_pointer = strsep(&working_string, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_pointer, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }


    //code starts here

    if(token[0] == NULL)
    {
      continue;
    }
    else if(strcasecmp(token[0], "open") == 0)
    {
      openFile(token[1]);
    }
    else if(strcasecmp(token[0], "close") == 0)
    {
      closeFile();
    }
    else if(strcasecmp(token[0], "info") == 0)
    {
      info();
    }
    else if(strcasecmp(token[0], "ls") == 0)
    {
      ls();
    }
    else if(strcasecmp(token[0], "stat") == 0)
    {
      if(token[1] == NULL)
      {
        printf("Error: Please enter filename.\n");
        continue;
      }
      stat(token[1]);
    }
    else if(strcasecmp(token[0], "read") == 0)
    {
      if(token[1] == NULL)
      {
        printf("Error: Please enter filename.\n");
        continue;
      }
      if(token[2] == NULL)
      {
        printf("Please enter starting position.\n");
        continue;
      }
      if(token[3] == NULL)
      {
        printf("Please enter number of bytes.\n");
        continue;
      }
      int position = atoi(token[2]);
      int numBytes = atoi(token[3]);
      readFile(token[1], position, numBytes, token[4]);
    }
    else if(strcasecmp(token[0],"put")==0)
    {
     if(token[1] != NULL)
     {
        put(token[1], token[2]);
     }
     else
     {
      printf("Error: No filename for 'put' command.\n");
     }
    }
    else if(strcasecmp(token[0],"get")==0)
    {
      if(token[1] != NULL)
      {
        get(token[1], token[2]);
      }
      else
      {
        printf("Error: No filename provided for 'get' command.\n");
      }
    }
    else if(strcasecmp(token[0], "del")==0)
    {
      if(token[1] != NULL)
      {
        del(token[1]);
      }
      else
      {
        printf("Error: No filename provided for 'del' command.\n");
      }
    }
    else if(strcasecmp(token[0],"undel") == 0)
    {
      if(token[1] != NULL)
      {
        undel(token[1]);
      }
      else
      {
        printf("Error: No filename provided for 'undel' command.\n");
      }
    }
    else if(strcasecmp(token[0],"cd") == 0)
    {
      if(token[1] != NULL)
      {
        cd(token[1]);
      }
      else
      {
        printf("Error: No filename provided for 'cd' command.\n");
      }
    }
    else if((strcasecmp(token[0], "quit") == 0) || (strcasecmp(token[0], "exit") == 0))
    {
      exit(0);
    }
    else
    {
      printf("Error: Invalid command\n");
    }

    //code ends here


    free( head_ptr );

  }
  return 0;
  
}

// open <filename>
void openFile(char *filename)
{

  if(isOpen)
  {
    printf("Error: File system image already open.\n");
    return;
  }

  fp = fopen(filename, "r"); //ask about r+

  if(fp == NULL)
  {
    printf("Error: File system image not found.\n");
    return;
  }
  isOpen = 1;

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

  fseek(fp, 40, SEEK_SET);
  fread(&BPB_ExtFlags, 2, 1, fp);

  fseek(fp, 44, SEEK_SET);
  fread(&BPB_RootClus, 4, 1, fp);

  fseek(fp, 48, SEEK_SET);
  fread(&BPB_FSInfo, 2, 1, fp);

  RootAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);
  fseek(fp, RootAddress, SEEK_SET);

  for(int i = 0; i < 16; i++)
  {
    fread(&dir[i], 32, 1, fp);
  }
  FatSize = BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec;
  FirstSectorofCluster = (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);
  ClusterSize = BPB_SecPerClus * BPB_BytsPerSec;

  for(int j= 0; j< 16; j++)
  {
    if(dir[j].DIR_Attr != 0x10 && dir[j].DIR_Attr != 0x01 && dir[j].DIR_Attr != 0x02 && dir[j].DIR_Attr != 0x04 && dir[j].DIR_Attr != 0x20 && dir[j].DIR_Attr != 0x08)
    {
      dir[j].DIR_Attr = 0x00;
    }
  }

}



int32_t LBAToOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

int16_t NextLB(uint32_t sector)
{
  uint32_t FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector * 4);
  int16_t val;
  fseek( fp, FATAddress, SEEK_SET);
  fread(&val, 2, 1, fp);
  return val;
}

// close
void closeFile()
{
  if(!isOpen)
  {
    printf("Error: File system not open.\n");
    return;
  }
  fclose(fp);
  fp = NULL;
  isOpen = 0;
}

// info
void info()
{
  printf(
  "BPB_BytsPerSec:\t %x\t %d\n"
  "BPB_SecPerClus:\t %x\t %d\n"
  "BPB_RsvdSecCnt:\t %x\t %d\n"
  "BPB_NumFATs:\t %x\t %d\n"
  "BPB_FATSz32:\t %x\t %d\n"
  "BPB_ExtFlags:\t %x\t %d\n"
  "BPB_RootClus:\t %x\t %d\n"
  "BPB_FSInfo:\t %x\t %d\n", 
  BPB_BytsPerSec, BPB_BytsPerSec,
  BPB_SecPerClus, BPB_SecPerClus, 
  BPB_RsvdSecCnt, BPB_RsvdSecCnt, 
  BPB_NumFATs,    BPB_NumFATs, 
  BPB_FATSz32,    BPB_FATSz32,
  BPB_ExtFlags,   BPB_ExtFlags, 
  BPB_RootClus,   BPB_RootClus, 
  BPB_FSInfo,     BPB_FSInfo);
}

// ls
void ls()
{
  char DirName[11];
  for(int i = 0; i < 16; i++)
  {
    if((dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20))
    {
      if(dir[i].DIR_Name[0] != (char)0xe5)
      {
        strncpy(DirName, dir[i].DIR_Name, 11);
        printf("%s\n", DirName);
      }
    }
  }
}

// stat <filename> or <directory name>
void stat(char *filename)
{
  char DirName[11];
  int FileValid = 0;
  char expanded_name[12];
      memset( expanded_name, ' ', 12 );

      char *token = strtok( filename, "." );
      strncpy( expanded_name, token, strlen( token ) );
      token = strtok( NULL, "." );
      if( token )
      {
        strncpy( (char*)(expanded_name+8), token, strlen(token ) );
      }

      expanded_name[11] = '\0';

      int j;
      for( j = 0; j < 11; j++ )
      {
        expanded_name[j] = toupper( expanded_name[j] );
      }
  for(int i = 0; i < 16; i++)
  {
    if((dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20))
    {
      strncpy(DirName, dir[i].DIR_Name, 11); 
      if(strncmp(expanded_name, DirName, 11) == 0)
      {
        printf(
        "Name: %s\n"
        "Attribute: %x\n"
        "Starting Cluster: %x\n"
        "FileSize: %d\n", 
        expanded_name, 
        dir[i].DIR_Attr, 
        dir[i].DIR_FirstClusterLow, 
        dir[i].DIR_FileSize
        );
        FileValid = 1;
      } 
    }
  }
  if(!FileValid)
  {
    printf("Error: File not found.\n");
  }
}

// get <filename>
// get <filename> <new filename>
void get(char *filename, char *newfilename)
{
  if(newfilename == NULL)
  {
    newfilename = malloc(strlen(filename) + 1);
    strcpy(newfilename, filename); //default to original name if new name not provided
  }
  
  if(!isOpen) //check if file system if open
  {
    printf("Error: File system not open.\n");
    return;
  }
  //convert filename to fat32 8.3 format with space padding
  char expanded_name[12];
  memset(expanded_name, ' ', 12);
  char *token = strtok(filename, ".");
  strncpy(expanded_name, token, strlen(token));
  token = strtok(NULL, ".");
  if(token != NULL)
  {
    strncpy(expanded_name + 8, token, strlen(token));
  }
  expanded_name[11] = '\0';

  int j;
  for( j = 0; j < 11; j++ )
  {
    expanded_name[j] = toupper( expanded_name[j] );
  }
  //locate the file in the directory of entries
  int fileIndex = -1;
  int i = 0;
  int flag = 0;
  while (i<16 && !flag)
  {
    if(strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
    {
        fileIndex = i;
        flag = 1;
    }

    i++;
  }

  if(!flag)
  {
    printf("Error: File not found.\n");
    return;
  }

  //open a new file in write mode to store the retrieved data
  FILE *output = fopen(newfilename, "w");
  if(!output)
  {
    printf("Error: Could not create output file.\n");
    return;
  }

  int bytes_to_copy = dir[fileIndex].DIR_FileSize;
  int cluster = dir[fileIndex].DIR_FirstClusterLow;

  //loop through the clusters to read file data
  while(bytes_to_copy > BPB_BytsPerSec)
  {
    int offset = LBAToOffset(cluster); //convert cluster number to file offset
    fseek(fp,offset, SEEK_SET);        //move to the offset in the file system image

    char buffer[BPB_BytsPerSec]; //read data from the image
    fread(buffer, 1, BPB_BytsPerSec, fp); //write data to the output file
    fwrite(buffer, 1, BPB_BytsPerSec, output);

    //find the next cluster and update bytes_to_copy
    cluster = NextLB(cluster);
    bytes_to_copy = bytes_to_copy - BPB_BytsPerSec;
  }

  //handle remaining bytes if any
  if(bytes_to_copy > 0)
  {
    int offset = LBAToOffset(cluster);
    fseek(fp, offset, SEEK_SET);
    char buffer[BPB_BytsPerSec];
    fread(buffer, 1, bytes_to_copy, fp);
    fwrite(buffer, 1, bytes_to_copy, output);
  }
  fclose(output);
  printf("File successfully retrieved as %s \n", newfilename);
}


// put <filename> 
// put <filename> <new filename>
void put(char *filename, char *newfilename)
{
  if(newfilename == NULL)
  {
    newfilename = filename;
  }
  if(!isOpen)
  {
    printf("Error = File system not open. \n");
    return;
  }

  //open file in read mode from the current directory
  FILE *input = fopen(filename, "r");
  printf("%s",filename);
  if(!input)
  {
    printf("Error: File not found in current directory.\n");
    return;
  }

  fseek(input, 0, SEEK_END);
  int fileSize = ftell(input);
  fseek(input, 0, SEEK_SET);

  //locate an avaliable directory entry
  int fileIndex = -1;
  int flag = 0;
  for(int i=0 ;i < 16; i++)
  {

    if(dir[i].DIR_Attr == 0x00 || dir[i].DIR_Attr == 0xE5)
    {
      fileIndex = i;
      flag = 1;
      i = 16; //leaves loop
    }
  }
  if(!flag)
  {
    printf("Error: No avaliable directory entry.\n");
    fclose(input);
    return;
  }
  memset(dir[fileIndex].DIR_Name, ' ', 11); //clear name field
  char *token = strtok(filename, ".");
  strncpy(dir[fileIndex].DIR_Name, token, strlen(token));
  token = strtok(NULL, ".");
  if(token != NULL)
  {
    strncpy(dir[fileIndex].DIR_Name + 8, token, strlen(token));
  }

  dir[fileIndex].DIR_Attr = 0x20; //set file attribute to "archive"
  dir[fileIndex].DIR_FileSize = fileSize;

  //find the first free cluster for this file
  int cluster = 2; //function to find free cluster
  int FATAdd;
  int16_t clusterVal;
  int clusterFound = 0;

  while(cluster < BPB_FATSz32 && !clusterFound)
  {
    FATAdd = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (cluster * 4);
    fseek(fp, FATAdd, SEEK_SET);
    fread(&clusterVal, 2, 1, fp);

    if(clusterVal == 0x0000) //check if cluster is free
    {
      clusterFound = 1;
    }
    else
    {
      cluster ++;
    }
  }

  if(!clusterFound)
  {
    printf("Error: No free clusters avaliable. \n");
    fclose(input);
    return;
  }

  dir[fileIndex].DIR_FirstClusterLow = cluster; //store starting cluster 
  int bytes_to_copy = fileSize;

  //loop through the cluster to write file data

  int nextClusterFound = 0;
  while(bytes_to_copy > 0)
  {
    int offset = LBAToOffset(cluster);
    fseek(fp, offset, SEEK_SET);

    char buffer[BPB_BytsPerSec];
    int bytes_to_write;
    if(bytes_to_copy > BPB_BytsPerSec)
    {
      bytes_to_write = BPB_BytsPerSec;
    }
    else
    {
      bytes_to_write = bytes_to_copy;
    }
    fread(buffer, 1, BPB_BytsPerSec, input);
    fwrite(buffer, 1, BPB_BytsPerSec, fp);
    bytes_to_copy = bytes_to_copy - bytes_to_write;

    if(bytes_to_copy > 0)
    {
      nextClusterFound = 0;
      int nextCluster = cluster + 1;

      while(nextCluster < BPB_FATSz32 && !nextClusterFound)
      {
        FATAdd = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (nextCluster * 4);
        fseek(fp, FATAdd, SEEK_SET);
        fread(&clusterVal, 2, 1, fp);

        if(clusterVal == 0x0000)
        {
          nextClusterFound = 1;
        }
        else
        {
          nextCluster++;
        }
      }

      if(!nextClusterFound)
      {
        printf("Error: No more free clusters avaliable.\n");
        fclose(input);
        return;
      }

      //link current cluster to the next
      FATAdd = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (cluster * 4);
      fseek(fp,FATAdd, SEEK_SET);
      fwrite(&nextCluster, 2, 1, fp);

      cluster = nextCluster;
    }
    else
    {
      int endOfChain = 0xFFF8;
      FATAdd= (BPB_BytsPerSec * BPB_RsvdSecCnt) + (cluster * 4);
      fseek(fp, FATAdd, SEEK_SET);
      fwrite(&endOfChain, 2, 1, fp);
    }
  }

    fclose(input);
    printf("File successfully added as %s\n", newfilename);
}

    

// del <filename> 
void del(char *filename)
{
if(!isOpen)
  {
    printf("Error: File system not open.\n");
    return;
  }

  //convert filename to fat32 8.3 format with space padding
  char expanded_name[12];
  memset(expanded_name, ' ', 12);
  char *token = strtok(filename, ".");
  strncpy(expanded_name, token, strlen(token));
  token = strtok(NULL, ".");
  if(token != NULL)
  {
    strncpy(expanded_name + 8, token, strlen(token));
  }
  expanded_name[11] = '\0';

  int j;
  for( j = 0; j < 11; j++ )
  {
    expanded_name[j] = toupper( expanded_name[j] );
  }

  //locate the file in the directory of entries
  int fileIndex = -1;
  int flag = 0;
  for (int i = 0; i<16 && !flag; i++)
  {
    if(strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
    {
        fileIndex = i;
        flag = 1;
    }
  }

  if(!flag)
  {
    printf("Error: File not found.\n");
    return;
  }

  //mark the file's directory entry as deleted by setting DIR_Name[0] to 0xE5
  dir[fileIndex].DIR_Name[0] = 0xE5;

  //get the starting cluster of the file
  int cluster = dir[fileIndex].DIR_FirstClusterLow;
  int FATAddress;
  while(cluster != -1)
  {
    FATAddress = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (cluster * 4);
    fseek(fp, FATAddress, SEEK_SET);
    int16_t freeClusterValue = 0x0000;
    fwrite(&freeClusterValue, 2, 1, fp);
    cluster = NextLB(cluster); //move to the next cluster in the file chain
  }

  printf("File '%s' marked as deleted.\n",filename);
}

void undel(char *filename)
{

  if(!isOpen)
  {
    printf("Error: File system not open.\n");
    return;
  }

  //convert filename to fat32 8.3 format with space padding
  char expanded_name[12];
  memset(expanded_name, ' ', 12);
  char *token = strtok(filename, ".");
  strncpy(expanded_name, token, strlen(token));
  token = strtok(NULL, ".");
  if(token != NULL)
  {
    strncpy(expanded_name + 8, token, strlen(token));
  }
  expanded_name[11] = '\0';

  int j;
  for( j = 0; j < 11; j++ )
  {
    expanded_name[j] = toupper( expanded_name[j] );
  }

  int flag = 0;
  for(int i = 0; i<16 && !flag; i++)
  {
    if(dir[i].DIR_Name[0] == 0xE5 && strncmp(&dir[i].DIR_Name[1], &expanded_name[1], 10) == 0)
    {
      //restore first character to undelete
      dir[i].DIR_Name[0] = expanded_name[0];
      flag = 1;
    }
  }

  if(flag == 1)
  {
    printf("File %s has been restored.\n", filename);
  }
  else
  {
    printf("Error: Deleted file '%s' not found.\n", filename);
  }

}

// read <filename> <position> <number of bytes> <OPTION>
// -ascii
// -dec
void readFile(char* filename, int position, int numBytes, char* option)
{

  char DirName[11];
  int FileValid = 0;
  int cluster;
  char expanded_name[12];
  char value[numBytes];
  memset( expanded_name, ' ', 12 );

  char *token = strtok( filename, "." );
  strncpy( expanded_name, token, strlen( token ) );
  token = strtok( NULL, "." );
  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }
  expanded_name[11] = '\0';
  int j;
  for( j = 0; j < 11; j++ )
  {
    expanded_name[j] = toupper( expanded_name[j] );
  }
  for(int i = 0; i < 16; i++)
  {
    if((dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20))
    {
      strncpy(DirName, dir[i].DIR_Name, 11); 
      if(strncmp(expanded_name, DirName, 11) == 0)
      {
        cluster = dir[i].DIR_FirstClusterLow;
        fseek(fp, LBAToOffset(cluster) + position, SEEK_SET);
        fread(&value, numBytes, 1, fp); 
        if(option == NULL)
        {
          for(int i = 0; i < numBytes; i++)
          {
            printf("%x ", (int)value[i]); // prints in hex
          }
          printf("\n");
        }
        else if(strcasecmp(option, "-ascii") == 0)
        {
          printf("%s\n", value); // prints in ascii
        }
        else if(strcasecmp(option, "-dec") == 0)
        {
          for(int i = 0; i < numBytes; i++)
          {
            printf("%d ", (int)value[i]); // prints in decimal
          }
          printf("\n");
        }
        else
        {
          printf("Please type in '-ascii', '-dec', or nothing for 4th parameter.\n");
          return;
        }
        FileValid = 1;
      } 
    }
  }
  if(!FileValid)
  {
    printf("Error: File not found.\n");
  }
}

// cd <directory>
void cd(char *directory)
{
  int currentCluster;
  int flag = 0;
  if(!isOpen)
  {
    printf("Error: File system not open.\n");
    return;
  }

  //special cases for current and parent directories
  if(strcmp(directory, ".") ==0)
  {
    //'.' refers to the current directory, so no actions is needed
    return;
  }
  else if(strcmp(directory, "..") == 0)
  {

    currentCluster = 2;
    flag = 1;
  }
  else
  {
    char expanded_name[12]= "            ";
    int len = strlen(directory);
    for(int i = 0; i<len && i<8; i++)
    {
      expanded_name[i] = toupper(directory[i]);
    }

    for(int i = 0;i < 16; i++)
    {
      if( dir[i].DIR_Attr == 0x10 && strncmp(dir[i].DIR_Name, expanded_name, 11) == 0 && dir[i].DIR_Name[0] != 0xe5)
      {
        currentCluster = dir[i].DIR_FirstClusterLow;
        flag = 1;
      }
    }
  }
  
  fseek(fp, LBAToOffset(currentCluster), SEEK_SET);
  for(int j = 0; j < 16; j++)
  {
    fread(&dir[j], 32, 1, fp);
  }
  if(!flag)
  {
    printf("Error: Directory '%s' not found. \n", directory);
  }
  else
  {
    printf("Changed directory to '%s'.\n", directory);
  }
    
}

