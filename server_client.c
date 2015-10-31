#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/socket.h>
#include <regex.h>         	
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/md5.h>

//File Data Structure

int timeToclose;
int childPid;
int fd[2];
int pid;

struct fileData
{
	char fileName[100];
	off_t size;
	time_t modTime;
	char type[100]; 
};

int main(int argc, char *argv[])
{
	// Variables for for both server and client

	char* acting;
	int connType;
	int portno;
	int portno1;
	char uploadPermission[100];

	// Asking for ports

	printf("Enter host no:");
	scanf("%d",&portno);

	printf("Enter peer no:");
	scanf("%d",&portno1);

	printf("Allow Upload Files:");
	scanf("%s",uploadPermission);

	// Asking type of connection

	printf("Enter 0 for tcp and 1 for udp\n");
	scanf("%d",&connType);
	getchar();

	//forking variables

	int tcp_client = 0;
	int tcp_server = 0;
	int udp_client = 0;
	int udp_server = 0;

	// Fork for simultaneous transfer

	pipe(fd);
	childPid = getpid();
	pid = fork();
	if(!pid)
	{
		if(connType==0)
		{
			tcp_client = 1;
		}
		else
		{
			udp_client = 1;
		}
	}
	else 
	{
		if(connType==0)
		{
			tcp_server = 1;
		}
		else
		{
			udp_server = 1;
		}
	}

	if(connType == 0)
	{

		// Variables for server

		int listenSocket = 0;	
		int connectionSocket = 0;
		struct sockaddr_in serv_addr;
		char str1[1124];
		struct fileData longlist[100];

		// Assigning values for varibales
		bzero((char *) &serv_addr,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;	
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(portno);

		// Creating a socket

		listenSocket = socket(AF_INET,SOCK_STREAM,0);
		if(listenSocket<0)
		{
			printf("ERROR WHILE CREATING A SOCKET\n");
			return 0;
		}
		else
		{
			printf("[SERVER] SOCKET ESTABLISHED SUCCESSFULLY\n\n");
		}

		if(bind(listenSocket,(struct sockaddr * )&serv_addr,sizeof(serv_addr))<0)
		{
			if(tcp_client == 1)
			{
				// Variables for client

				acting = "c";
				acting = "s";
				int ClientSocket = 0;
				struct sockaddr_in serv_addr1;
				char buffer[1024];
				int entryCheck = 0;

				// Variable has info for file download -> receving data

				int fileCheck = 0;

				// Variable has info for file upload -> sending data
				int fileCheckUp = 0;

				// Assigning values for variables

				serv_addr1.sin_family = AF_INET;
				serv_addr1.sin_port = htons(portno1);
				serv_addr1.sin_addr.s_addr = inet_addr("127.0.0.1");

				// Creating a socket

				ClientSocket = socket(AF_INET,SOCK_STREAM,0);
				if(ClientSocket<0)
				{
					printf("ERROR WHILE CREATING A SOCKET\n");
					return 0;
				}
				else
				{
					printf("[CLIENT] Socket created \n");	
				}

				//Connection Establishment

				while(connect(ClientSocket,(struct sockaddr *)&serv_addr1,sizeof(serv_addr1))<0);

				printf("Connected as client\n");

				// Infinite loop for recieving and sending data

				while(1)
				{
					bzero(buffer,1024);
					if(entryCheck == 0)
					{
						printf("\n");

						// Getting input from user

						fgets(buffer,1023,stdin);

						// Checking for commands

						if(strstr(buffer, "IndexGet\0") != NULL) 
						{

							entryCheck = 1;
						}
						else if(strstr(buffer, "FileHash\0") != NULL)
						{
							entryCheck = 1;
						}
						else if(strstr(buffer, "FileDownload\0") != NULL)
						{
							entryCheck = 1;
							fileCheck  = 1;
						}
						else if(strstr(buffer, "FileUpload\0")!=NULL)
						{
							entryCheck = 1;
							fileCheckUp = 1;
						}
						if(send(ClientSocket,buffer,strlen(buffer),0) < 0)
						{
							printf("ERROR while writing to the socket\n");
							return 0;
						}
					}
					if(fileCheck == 0 && fileCheckUp == 0)
					{
						bzero(buffer,1023);
						if(recv(ClientSocket,buffer,1023,0) < 0)
						{
							printf("ERROR while reading from the socket\n");
							return 0;
						}
						if(strstr(buffer, "END\0") != NULL)
						{
							entryCheck = 0;
						}
						printf("%s",buffer);
					}
					else if(fileCheck == 1 && fileCheckUp == 0)
					{
						bzero(buffer,1023);
						if(recv(ClientSocket,buffer,1023,0) < 0)
						{
							printf("ERROR while reading from the socket\n");
						}

						// Code for downloading data

						char* fr_name = buffer;
						FILE *fr = fopen(fr_name, "a");
						int fr_block_sz = 0;
						while((fr_block_sz = recv(ClientSocket, buffer, 1023, 0)) > 0)
						{
							int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
							if(write_sz < fr_block_sz)
							{
								error("File write failed.\n");
								return 0;
							}
							bzero(buffer, 1023);
							if (fr_block_sz == 0 || fr_block_sz != 1023) 
							{
								break;
							}
						}
						if(fr_block_sz < 0)
						{
							if (errno == EAGAIN)
							{
								printf("recv() timed out.\n");
								return 0;
							}
							else
							{
								fprintf(stderr, "recv() failed due to errno = %d\n", errno);
								return 0;
							}
						}
						printf("Successfully received file from server!\n");
						fclose(fr);
						fileCheck = 0;
					}
					else if(fileCheck == 0 && fileCheckUp == 1)
					{
						char *token1 = strtok(buffer," ");
						token1 = strtok(NULL," ");

						printf("%s\n", token1);
						char tempFile[1024];

						// For loop for removing new line charater from the file name

						int s;
						for(s=0;s<strlen(token1) - 1;s++)
						{
							tempFile[s] = token1[s];
						}
						tempFile[s]='\0';

						// For recieving permission

						//	bzero(buffer,1023);
						//	if(recv(ClientSocket,buffer,1023,0)<0)
						//		printf("ERROR while reading from the socket\n");


						//if(strstr(buffer,"FileUploadAllow\0") != NULL)
						if(strstr(uploadPermission,"FileUploadAllow") != NULL)
						{
							// For sending filename

							bzero(buffer,1023);
							strcpy(buffer,tempFile);
							strcat(buffer,"\n");
							send(ClientSocket,buffer,strlen(buffer),0);

							// Code for uploading data 

							char* fs_name = tempFile;
							char sdbuf[1023]; 
							printf("Sending %s to the Server\n", fs_name);	
							FILE *fs = fopen(fs_name, "r");
							if(fs == NULL)
							{
								fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
								exit(1);
							}
							bzero(sdbuf, 1023); 
							int fs_block_sz; 
							while((fs_block_sz = fread(sdbuf, sizeof(char), 1023, fs))>0)
							{
								if(send(ClientSocket, sdbuf, fs_block_sz, 0) < 0)
								{
									fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
									exit(1);
								}
								bzero(sdbuf, 1023);
							}
							printf("Successfully sent to the server!\n");

							// Code for sending filesize, md5hash and last modified time

							DIR *dp;
							struct dirent *ep;
							dp = opendir("./");
							struct stat fileStat;
							if (dp != NULL)
							{
								int counter = 0;
								while (ep = readdir (dp))
								{
									if(stat(ep->d_name,&fileStat) < 0)
									{
										bzero(buffer,1023);
										strcpy(buffer,"Error occured");
										printf("%s\n", buffer );
										send(ClientSocket,buffer,strlen(buffer),0);
										return 0;
									}
									else if(strstr(ep->d_name,fs_name) != NULL)
									{
										unsigned char c[MD5_DIGEST_LENGTH];
										char filename[100];
										strcpy(filename,ep->d_name);
										int i;
										FILE *inFile = fopen (fs_name, "r");
										MD5_CTX mdContext;
										int bytes;
										unsigned char data[1024];
										if (inFile == NULL) 
										{
											printf ("%s can't be opened.\n", fs_name);
											return 0;
										}

										MD5_Init (&mdContext);
										while ((bytes = fread (data, 1, 1024, inFile)) != 0)
										{
											MD5_Update (&mdContext, data, bytes);
										}
										MD5_Final (c,&mdContext);
										unsigned char hash[MD5_DIGEST_LENGTH];
										bzero(buffer,1023);
										char temp[100];
										for(i = 0; i < MD5_DIGEST_LENGTH; i++)
										{
											sprintf(temp, "%x",c[i]);
											strcat(buffer,temp);
										}
										fclose (inFile);
										strcat(buffer,"\n");
										printf("%s",buffer);
										send(ClientSocket,buffer,strlen(buffer),0);

										bzero(buffer,1023);
										char sizeFile[100];
										sprintf(sizeFile,"%jd",fileStat.st_size); 
										strcat(sizeFile,"\n");
										printf("%s",sizeFile );
										strcpy(buffer,sizeFile);
										send(ClientSocket,buffer,strlen(buffer),0);

										char tempTime[100];
										sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
										strcpy(buffer,tempTime);
										strcat(buffer,"  \n");
										send(ClientSocket,buffer,strlen(buffer),0);
										printf("%s", buffer);
										break;
									}
								}
							}
							bzero(buffer,1023);
							sprintf(buffer,"%s\n","END");
							send(ClientSocket,buffer,strlen(buffer),0);
							fileCheckUp = 0;
							entryCheck = 0;
						}
						//else if(strstr(buffer,"FileUploadDeny\0") != NULL)
						else if(strstr(uploadPermission,"FileUploadDeny") != NULL)
						{
							entryCheck = 0;
							fileCheckUp = 0;
						}
					}
				}
			}
		}
		else
		{
			// Acts as server as the port is empty 

			acting = "s";
			printf("[SERVER] SOCKET BINDED SUCCESSFULLY\n");
		}

		if(acting == "s")
		{
			if(tcp_server == 1)
			{
				// Listening to connections

				if(listen(listenSocket,10) == -1)
				{
					printf("[SERVER] FAILED TO ESTABLISH LISTENING \n\n");
				}
				printf("[SERVER] Waiting fo client to connect....\n" );	

				// Accepting connections

checkpoint:while((connectionSocket=accept(listenSocket , (struct sockaddr*)NULL,NULL))<0);	

	   // NULL will get filled in by the client's sockaddr once a connection is establised

	   printf("Connected to a client\n");

	   // Infinite loop for sending and receiving data

	   while(1)
	   {
		   char buffer[1024];
		   bzero(buffer,1024);

		   // Checking status of client

		   if(recv(connectionSocket,buffer,1023,0) <= 0)
		   {
			   printf("Client Not responding well -- Closing -- Checking for new connection\n");
			   goto checkpoint;
		   }

		   memset(str1,'\0',sizeof(str1));
		   strcpy(str1,buffer);

		   // Token1 is for main commmand and for flags

		   char *token1=NULL;
		   token1 = strtok(str1," ");
		   printf("%s\n",buffer);
		   if(strstr(token1, "IndexGet\0") != NULL)
		   {
			   token1 = strtok(NULL," ");
			   if(strstr(token1, "longlist\0") != NULL)
			   {
				   // Code for sending all file related data

				   DIR *dp;
				   struct dirent *ep;
				   dp = opendir("./");
				   struct stat fileStat;
				   if (dp != NULL)
				   {

					   int counter = 0;
					   while (ep = readdir (dp))
					   {

						   if(stat(ep->d_name,&fileStat) < 0)
						   {
							   bzero(buffer,1023);
							   strcpy(buffer,"Error occured");
							   printf("%s\n", buffer );
							   send(connectionSocket,buffer,strlen(buffer),0);
							   return 0;
						   }
						   else
						   {
							   strcpy(longlist[counter].fileName, ep->d_name);
							   longlist[counter].size = fileStat.st_size;
							   longlist[counter].modTime = fileStat.st_mtime;
							   strcpy(longlist[counter].type,(S_ISDIR(fileStat.st_mode)) ? "d  " : "-  ");
							   bzero(buffer,1023);
							   strcat(longlist[counter].fileName,"  ");
							   strcpy(buffer,longlist[counter].fileName);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s", buffer);
							   bzero(buffer,1023);
							   strcpy(buffer,longlist[counter].type);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s", buffer);
							   bzero(buffer,1023);
							   char sizeFile[100];
							   sprintf(sizeFile,"%jd",longlist[counter].size); 
							   strcat(sizeFile,"  ");
							   strcpy(buffer,sizeFile);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s",buffer);
							   bzero(buffer,1023);
							   char tempTime[100];
							   sprintf(tempTime,"%s",ctime((&longlist[counter].modTime)));
							   strcpy(buffer,tempTime);
							   strcat(buffer,"  \n");
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s", buffer);
							   counter++;
						   }
					   }
					   bzero(buffer,1023);
					   sprintf(buffer,"%s","END");
					   printf("END\n");
					   send(connectionSocket,buffer,strlen(buffer),0);
					   closedir (dp);
				   }
				   else
				   {	
					   strcpy(buffer,"Error occured");
					   printf("%s\n", buffer );
					   send(connectionSocket,buffer,strlen(buffer),0);
					   return 0;
				   }
			   }
			   else if(strstr(token1, "shortlist\0") != NULL)
			   {
				   token1 = strtok(NULL," ");
				   struct tm tm;
				   strptime(token1, "%a %b %e %H:%M:%S %Y", &tm);
				   time_t t = mktime(&tm);
				   char temp[1000]; 
				   sprintf(temp,"%s",ctime((&t)));
				   token1 = strtok(NULL," ");
				   strptime(token1, "%a %b %e %H:%M:%S %Y", &tm);
				   time_t t1 = mktime(&tm);

				   // Code for sending all file related data 

				   DIR *dp;
				   struct dirent *ep;
				   dp = opendir("./");
				   struct stat fileStat;
				   if (dp != NULL)
				   {
					   int counter = 0;
					   while (ep = readdir (dp))
					   {

						   if(stat(ep->d_name,&fileStat) < 0)
						   {
							   bzero(buffer,1023);
							   strcpy(buffer,"Error occured");
							   printf("%s\n", buffer);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   return 0;
						   }
						   else if(difftime(fileStat.st_mtime,t) > 0 && difftime(t1,fileStat.st_mtime) > 0)
						   {
							   strcpy(longlist[counter].fileName, ep->d_name);
							   longlist[counter].size = fileStat.st_size;
							   longlist[counter].modTime = fileStat.st_mtime;
							   strcpy(longlist[counter].type,(S_ISDIR(fileStat.st_mode)) ? "d  " : "-  ");
							   bzero(buffer,1023);
							   strcat(longlist[counter].fileName,"  ");
							   strcpy(buffer,longlist[counter].fileName);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s", buffer);
							   bzero(buffer,1023);
							   strcpy(buffer,longlist[counter].type);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s", buffer);
							   bzero(buffer,1023);
							   char sizeFile[100];
							   sprintf(sizeFile,"%jd",longlist[counter].size); 
							   strcat(sizeFile,"  ");
							   strcpy(buffer,sizeFile);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s",buffer);
							   bzero(buffer,1023);
							   char tempTime[100];
							   sprintf(tempTime,"%s",ctime((&longlist[counter].modTime)));
							   strcpy(buffer,tempTime);
							   strcat(buffer,"  \n");
							   send(connectionSocket,buffer,strlen(buffer),0);
							   printf("%s", buffer);
							   counter++;
						   }
					   }
					   bzero(buffer,1023);
					   sprintf(buffer,"%s","END");
					   printf("END\n");
					   send(connectionSocket,buffer,strlen(buffer),0);
					   closedir (dp);
				   }
				   else
				   {	
					   strcpy(buffer,"Error occured");
					   printf("%s\n", buffer );
					   send(connectionSocket,buffer,strlen(buffer),0);
					   return 0;
				   }
			   }
			   else if(strstr(token1, "regex\0") != NULL)
			   {
				   token1 = strtok(NULL," ");

				   char regexInput[100];

				   // For loop for removing new line character

				   int s;
				   for(s=0;s<strlen(token1) - 1;s++)
				   {
					   regexInput[s] = token1[s];
				   }
				   DIR *dp;
				   struct dirent *ep;
				   dp = opendir("./");
				   struct stat fileStat;
				   if (dp != NULL)
				   {
					   int counter = 0;
					   while (ep = readdir (dp))
					   {

						   if(stat(ep->d_name,&fileStat) < 0)
						   {
							   bzero(buffer,1023);
							   strcpy(buffer,"Error occured");
							   printf("%s\n", buffer);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   return 0;
						   }
						   else
						   {	
							   // Code for regex matching 

							   regex_t regex;
							   int reti;
							   char msgbuf[100];

							   reti = regcomp(&regex,regexInput , 0);
							   if( reti )
							   {
								   fprintf(stderr, "Could not compile regex\n"); 
								   exit(1); 
							   }
							   reti = regexec(&regex, ep->d_name, 0, NULL, 0);
							   if( !reti )
							   {
								   strcpy(longlist[counter].fileName, ep->d_name);
								   longlist[counter].size = fileStat.st_size;
								   longlist[counter].modTime = fileStat.st_mtime;
								   strcpy(longlist[counter].type,(S_ISDIR(fileStat.st_mode)) ? "d  " : "-  ");
								   bzero(buffer,1023);
								   strcat(longlist[counter].fileName,"  ");
								   strcpy(buffer,longlist[counter].fileName);
								   send(connectionSocket,buffer,strlen(buffer),0);
								   printf("%s", buffer);
								   bzero(buffer,1023);
								   strcpy(buffer,longlist[counter].type);
								   send(connectionSocket,buffer,strlen(buffer),0);
								   printf("%s", buffer);
								   bzero(buffer,1023);
								   char sizeFile[100];
								   sprintf(sizeFile,"%jd",longlist[counter].size); 
								   strcat(sizeFile,"  ");
								   strcpy(buffer,sizeFile);
								   send(connectionSocket,buffer,strlen(buffer),0);
								   printf("%s",buffer);
								   bzero(buffer,1023);
								   char tempTime[100];
								   sprintf(tempTime,"%s",ctime((&longlist[counter].modTime)));
								   strcpy(buffer,tempTime);
								   strcat(buffer,"  \n");
								   send(connectionSocket,buffer,strlen(buffer),0);
								   printf("%s", buffer);
								   counter++;
							   }
							   else if( reti == REG_NOMATCH )
							   {
							   }
							   else
							   {
								   bzero(buffer,1023);
								   strcpy(buffer,"Error occured");
								   printf("%s\n", buffer);
								   send(connectionSocket,buffer,strlen(buffer),0);
								   return 0;
							   }
							   regfree(&regex);
						   }
					   }
				   }		   
				   bzero(buffer,1023);
				   sprintf(buffer,"%s\n","END");
				   printf("END\n");
				   send(connectionSocket,buffer,strlen(buffer),0);
			   }
		   }
		   else if(strstr(token1, "FileHash\0") != NULL)
		   {
			   token1 = strtok(NULL," ");
			   if(strstr(token1, "verify\0") != NULL)
			   {
				   token1 = strtok(NULL," ");

				   // MD5 Hash of a file

				   unsigned char c[MD5_DIGEST_LENGTH];
				   char filename[100];

				   // For loop for removing new line charater 

				   int i1;
				   for(i1=0;i1<strlen(token1)-1 ;i1++)
				   {
					   filename[i1] = token1[i1];
				   }
				   int i;
				   FILE *inFile = fopen (filename, "r");
				   MD5_CTX mdContext;
				   int bytes;
				   unsigned char data[1024];

				   if (inFile == NULL) {
					   printf ("%s can't be opened.\n", filename);
					   return 0;
				   }

				   MD5_Init (&mdContext);
				   while ((bytes = fread (data, 1, 1024, inFile)) != 0)
				   {
					   MD5_Update (&mdContext, data, bytes);
				   }
				   MD5_Final (c,&mdContext);
				   unsigned char hash[MD5_DIGEST_LENGTH];
				   bzero(buffer,1023);
				   char temp[100];
				   for(i = 0; i < MD5_DIGEST_LENGTH; i++)
				   {
					   sprintf(temp, "%x",c[i]);
					   strcat(buffer,temp);
				   }
				   fclose (inFile);
				   strcat(buffer,"\n");
				   printf("%s",buffer);
				   send(connectionSocket,buffer,strlen(buffer),0);
				   DIR *dp;
				   struct dirent *ep;
				   dp = opendir("./");
				   struct stat fileStat;
				   if (dp != NULL)
				   {

					   int counter = 0;
					   while (ep = readdir (dp))
					   {

						   if(stat(ep->d_name,&fileStat) < 0)
						   {
							   bzero(buffer,1023);
							   strcpy(buffer,"Error occured");
							   printf("%s\n", buffer );
							   send(connectionSocket,buffer,strlen(buffer),0);
							   return 0;
						   }
						   else
						   {
							   if(strcmp(filename,ep->d_name)==0)
							   {
								   char tempTime[100];
								   sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
								   strcpy(buffer,tempTime);
								   strcat(buffer,"  \n");
								   send(connectionSocket,buffer,strlen(buffer),0);
								   printf("%s", buffer);
								   break;
							   }
						   }
					   }
				   }
				   bzero(buffer,1023);
				   sprintf(buffer,"%s\n","END");
				   printf("END\n");
				   send(connectionSocket,buffer,strlen(buffer),0);
			   }
			   if(strstr(token1, "checkall\0") != NULL)
			   {
				   DIR *dp;
				   struct dirent *ep;
				   dp = opendir("./");
				   struct stat fileStat;
				   if (dp != NULL)
				   {
					   int counter = 0;
					   while (ep = readdir (dp))
					   {

						   if(stat(ep->d_name,&fileStat) < 0)
						   {
							   bzero(buffer,1023);
							   strcpy(buffer,"Error occured");
							   printf("%s\n", buffer );
							   send(connectionSocket,buffer,strlen(buffer),0);
							   return 0;
						   }
						   else
						   {
							   unsigned char c[MD5_DIGEST_LENGTH];
							   char filename[100];
							   strcpy(filename,ep->d_name);
							   bzero(buffer,1023);
							   strcpy(buffer,filename);
							   strcat(buffer,"\n");
							   printf("%s",buffer);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   int i;
							   FILE *inFile = fopen (filename, "r");
							   MD5_CTX mdContext;
							   int bytes;
							   unsigned char data[1024];

							   if (inFile == NULL) {
								   printf ("%s can't be opened.\n", filename);
								   return 0;
							   }

							   MD5_Init (&mdContext);
							   while ((bytes = fread (data, 1, 1024, inFile)) != 0)
							   {
								   MD5_Update (&mdContext, data, bytes);
							   }
							   MD5_Final (c,&mdContext);
							   unsigned char hash[MD5_DIGEST_LENGTH];
							   bzero(buffer,1023);
							   char temp[100];
							   for(i = 0; i < MD5_DIGEST_LENGTH; i++)
							   {
								   sprintf(temp, "%x",c[i]);
								   strcat(buffer,temp);
							   }
							   fclose (inFile);
							   strcat(buffer,"\n");
							   printf("%s",buffer);
							   send(connectionSocket,buffer,strlen(buffer),0);
							   DIR *dp;
							   struct dirent *ep;
							   dp = opendir("./");
							   struct stat fileStat;
							   if (dp != NULL)
							   {
								   int counter = 0;
								   while (ep = readdir (dp))
								   {

									   if(stat(ep->d_name,&fileStat) < 0)
									   {
										   bzero(buffer,1023);
										   strcpy(buffer,"Error occured");
										   printf("%s\n", buffer);
										   send(connectionSocket,buffer,strlen(buffer),0);
										   return 0;
									   }
									   else
									   {
										   if(strcmp(filename,ep->d_name)==0)
										   {
											   char tempTime[100];
											   sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
											   strcpy(buffer,tempTime);
											   strcat(buffer,"  \n");
											   send(connectionSocket,buffer,strlen(buffer),0);
											   printf("%s", buffer);
											   break;
										   }
									   }
								   }
							   }
						   }
					   }
				   }
				   bzero(buffer,1023);
				   sprintf(buffer,"%s\n","END");
				   printf("END\n");
				   send(connectionSocket,buffer,strlen(buffer),0);
			   }
		   }
		   else if(strstr(token1, "FileDownload\0") != NULL)
		   {
			   token1 = strtok(NULL," ");
			   char tempFile[1024];

			   // For loop for removing new line character

			   int s;
			   for(s=0;s<strlen(token1) - 1;s++)
			   {
				   tempFile[s] = token1[s];
			   }
			   tempFile[s]='\0';

			   // Sending file name

			   bzero(buffer,1023);
			   strcpy(buffer,tempFile);
			   send(connectionSocket,buffer,strlen(buffer),0);

			   char* fs_name = tempFile;
			   char sdbuf[1023]; 
			   printf("Sending %s to the Client", fs_name);
			   FILE *fs = fopen(fs_name, "r");
			   if(fs == NULL)
			   {
				   fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
				   exit(1);
			   }
			   bzero(sdbuf, 1023); 
			   int fs_block_sz; 
			   while((fs_block_sz = fread(sdbuf, sizeof(char), 1023, fs))>0)
			   {
				   if(send(connectionSocket, sdbuf, fs_block_sz, 0) < 0)
				   {
					   fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
					   exit(1);
				   }
				   bzero(sdbuf, 1023);
			   }
			   printf("Successfully sent to client.\n");

			   DIR *dp;
			   struct dirent *ep;
			   dp = opendir("./");
			   struct stat fileStat;
			   if (dp != NULL)
			   {
				   int counter = 0;
				   while (ep = readdir (dp))
				   {
					   if(stat(ep->d_name,&fileStat) < 0)
					   {
						   bzero(buffer,1023);
						   strcpy(buffer,"Error occured");
						   printf("%s\n", buffer );
						   send(connectionSocket,buffer,strlen(buffer),0);
						   return 0;
					   }
					   else if(strstr(ep->d_name,fs_name)!=NULL)
					   {
						   unsigned char c[MD5_DIGEST_LENGTH];
						   char filename[100];
						   strcpy(filename,ep->d_name);
						   int i;
						   FILE *inFile = fopen (fs_name, "r");
						   MD5_CTX mdContext;
						   int bytes;
						   unsigned char data[1024];
						   if (inFile == NULL) {
							   printf ("%s can't be opened.\n", fs_name);
							   return 0;
						   }

						   MD5_Init (&mdContext);
						   while ((bytes = fread (data, 1, 1024, inFile)) != 0)
						   {
							   MD5_Update (&mdContext, data, bytes);
						   }
						   MD5_Final (c,&mdContext);
						   unsigned char hash[MD5_DIGEST_LENGTH];
						   bzero(buffer,1023);
						   char temp[100];
						   for(i = 0; i < MD5_DIGEST_LENGTH; i++)
						   {
							   sprintf(temp, "%x",c[i]);
							   strcat(buffer,temp);
						   }
						   fclose (inFile);
						   strcat(buffer,"\n");
						   printf("%s",buffer);
						   send(connectionSocket,buffer,strlen(buffer),0);

						   bzero(buffer,1023);
						   char sizeFile[100];
						   sprintf(sizeFile,"%jd",fileStat.st_size); 
						   strcat(sizeFile,"\n");
						   printf("%s",sizeFile );
						   strcpy(buffer,sizeFile);
						   send(connectionSocket,buffer,strlen(buffer),0);

						   char tempTime[100];
						   sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
						   strcpy(buffer,tempTime);
						   strcat(buffer,"  \n");
						   send(connectionSocket,buffer,strlen(buffer),0);
						   printf("%s", buffer);
						   break;
					   }
				   }
			   }
			   bzero(buffer,1023);
			   sprintf(buffer,"%s\n","END");
			   printf("END\n");
			   send(connectionSocket,buffer,strlen(buffer),0);
		   }
		   else if(strstr(token1, "FileUpload\0") != NULL)
		   {
			   //printf("Please type FileUploadAllow for allowing and FileUploadDeny for denying: ");
			   //fgets(buffer,1023,stdin);
			   //printf("%s\n",buffer );
			   //if(strstr(buffer,"FileUploadAllow\0")!=NULL)
			   if(strstr(uploadPermission,"FileUploadAllow") != NULL)
			   {

				   // recieving the file name

				   bzero(buffer,1024);
				   if(recv(connectionSocket,buffer,1023,0)<0)
					   printf("ERROR while reading from the socket\n");

				   // Code for receiving data

				   char temp[1024];
				   int s;
				   for(s=0;s<strlen(buffer) - 1;s++)
				   {
					   temp[s] = buffer[s];
				   }
				   temp[s] = '\0';
				   char* fr_name = temp;
				   FILE *fr = fopen(fr_name, "a");
				   int fr_block_sz = 0;
				   while((fr_block_sz = recv(connectionSocket, buffer, 1023, 0)) > 0)
				   {
					   int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
					   if(write_sz < fr_block_sz)
					   {
						   error("File write failed.\n");
						   return 0;
					   }
					   bzero(buffer, 1023);
					   if (fr_block_sz == 0 || fr_block_sz != 1023) 
					   {
						   break;
					   }
				   }
				   if(fr_block_sz < 0)
				   {
					   if (errno == EAGAIN)
					   {
						   printf("recv() timed out.\n");
						   return 0;
					   }					   
					   else
					   {
						   fprintf(stderr, "recv() failed due to errno = %d\n", errno);
						   return 0;
					   }
				   }
				   printf("Successfully received the file.\n");
				   fclose(fr);	

				   while(strstr(buffer,"END\0")!=NULL)
				   {
					   bzero(buffer,1023);
					   if(recv(connectionSocket,buffer,1023,0) < 0)
					   {
						   printf("ERROR while reading from the socket\n");
					   }
					   printf("%s\n",buffer );
				   }
			   }
		   }
	   }
			}
		}
	}
	else
	{
		// Variables for server
		int listenSocket = 0;	
		int connectionSocket = 0;
		struct sockaddr_in serv_addr,client_addr;
		char str1[1124];
		struct fileData longlist[100];

		// Variables for udp connection

		int addr_len = sizeof(struct sockaddr);


		// Assigning values for varibales
		bzero((char *) &serv_addr,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;	
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(portno);

		// Creating a socket

		listenSocket = socket(AF_INET,SOCK_DGRAM,0);
		if(listenSocket<0)
		{
			printf("ERROR WHILE CREATING A SOCKET\n");
			return 0;
		}
		else
		{
			printf("[SERVER] SOCKET ESTABLISHED SUCCESSFULLY\n\n");
		}

		if(bind(listenSocket,(struct sockaddr * )&serv_addr,sizeof(serv_addr))<0)
		{
			// Variables for client
			if(udp_client == 1)
			{
				acting = "c";
				acting = "s";
				int ClientSocket = 0;
				struct sockaddr_in serv_addr1;
				char buffer[1024];
				int entryCheck = 0;

				// Variable has info for file download -> receving data

				int fileCheck = 0;

				// Variable has info for file upload -> sending data
				int fileCheckUp = 0;

				// Assigning values for variables

				serv_addr1.sin_family = AF_INET;
				serv_addr1.sin_port = htons(portno1);
				serv_addr1.sin_addr.s_addr = inet_addr("127.0.0.1");

				// Creating a socket

				ClientSocket = socket(AF_INET,SOCK_DGRAM,0);
				if(ClientSocket<0)
				{
					printf("ERROR WHILE CREATING A SOCKET\n");
					return 0;
				}
				else
				{
					printf("[CLIENT] Socket created \n");	
				}

				// Infinite loop for recieving and sending data

				while(1)
				{
					bzero(buffer,1024);
					if(entryCheck == 0)
					{
						printf("\n");

						// Getting input from user

						fgets(buffer,1023,stdin);

						// Checking for commands

						if(strstr(buffer, "IndexGet\0") != NULL) 
						{
							entryCheck = 1;
						}
						else if(strstr(buffer, "FileHash\0") != NULL)
						{
							entryCheck = 1;
						}
						else if(strstr(buffer, "FileDownload\0") != NULL)
						{
							entryCheck = 1;
							fileCheck  = 1;
						}
						else if(strstr(buffer, "FileUpload\0")!=NULL)
						{
							entryCheck = 1;
							fileCheckUp = 1;
						}
						if(sendto(ClientSocket, buffer,strlen(buffer), 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1)) < 0)
						{
							printf("ERROR while writing to the socket\n");
							return 0;
						}
					}
					if(fileCheck == 0 && fileCheckUp == 0)
					{
						bzero(buffer,1023);
						if(recvfrom(ClientSocket,buffer,1023,0,NULL,NULL) < 0)
						{
							printf("ERROR while reading from the socket\n");
							return 0;
						}
						if(strstr(buffer, "END\0") != NULL)
						{
							entryCheck = 0;
						}
						printf("%s",buffer);
					}
					else if(fileCheck == 1 && fileCheckUp == 0)
					{
						bzero(buffer,1023);
						if(recvfrom(ClientSocket,buffer,1023,0,NULL,NULL) < 0)
						{
							printf("ERROR while reading from the socket\n");
						}

						// Code for downloading data

						char* fr_name = buffer;
						FILE *fr = fopen(fr_name, "a");
						int fr_block_sz = 0;
						while((fr_block_sz = recvfrom(ClientSocket,buffer,1023,0,NULL,NULL)) > 0)
						{
							int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
							if(write_sz < fr_block_sz)
							{
								error("File write failed.\n");
								return 0;
							}
							bzero(buffer, 1023);
							if (fr_block_sz == 0 || fr_block_sz != 1023) 
							{
								break;
							}
						}
						if(fr_block_sz < 0)
						{
							if (errno == EAGAIN)
							{
								printf("recv() timed out.\n");
								return 0;
							}
							else
							{
								fprintf(stderr, "recv() failed due to errno = %d\n", errno);
								return 0;
							}
						}
						printf("Successfully received file from server!\n");
						fclose(fr);
						fileCheck = 0;
					}
					else if(fileCheck == 0 && fileCheckUp == 1)
					{
						char *token1 = strtok(buffer," ");
						token1 = strtok(NULL," ");

						printf("%s\n", token1);
						char tempFile[1024];

						// For loop for removing new line charater from the file name

						int s;
						for(s=0;s<strlen(token1) - 1;s++)
						{
							tempFile[s] = token1[s];
						}
						tempFile[s]='\0';

						//	// For recieving permission

						//	bzero(buffer,1023);
						//	if(recvfrom(ClientSocket,buffer,1023,0,NULL,NULL)<0)
						//		printf("ERROR while reading from the socket\n");


						if(strstr(uploadPermission,"FileUploadAllow\0") != NULL)
						{
							// For sending filename

							bzero(buffer,1023);
							strcpy(buffer,tempFile);
							strcat(buffer,"\n");
							sendto(ClientSocket, buffer,strlen(buffer), 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1));

							// Code for uploading data 

							char* fs_name = tempFile;
							char sdbuf[1023]; 
							printf("Sending %s to the Server\n", fs_name);	
							FILE *fs = fopen(fs_name, "r");
							if(fs == NULL)
							{
								fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
								exit(1);
							}
							bzero(sdbuf, 1023); 
							int fs_block_sz; 
							while((fs_block_sz = fread(sdbuf, sizeof(char), 1023, fs))>0)
							{
								if(sendto(ClientSocket, sdbuf, fs_block_sz, 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1)) < 0)
								{
									fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
									exit(1);
								}
								bzero(sdbuf, 1023);
							}
							printf("Successfully sent to the server!\n");

							// Code for sending filesize, md5hash and last modifies time

							DIR *dp;
							struct dirent *ep;
							dp = opendir("./");
							struct stat fileStat;
							if (dp != NULL)
							{
								int counter = 0;
								while (ep = readdir (dp))
								{
									if(stat(ep->d_name,&fileStat) < 0)
									{
										bzero(buffer,1023);
										strcpy(buffer,"Error occured");
										printf("%s\n", buffer );
										sendto(ClientSocket, buffer,strlen(buffer), 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1));;
										return 0;
									}
									else if(strstr(ep->d_name,fs_name) != NULL)
									{
										unsigned char c[MD5_DIGEST_LENGTH];
										char filename[100];
										strcpy(filename,ep->d_name);
										int i;
										FILE *inFile = fopen (fs_name, "r");
										MD5_CTX mdContext;
										int bytes;
										unsigned char data[1024];
										if (inFile == NULL) 
										{
											printf ("%s can't be opened.\n", fs_name);
											return 0;
										}

										MD5_Init (&mdContext);
										while ((bytes = fread (data, 1, 1024, inFile)) != 0)
										{
											MD5_Update (&mdContext, data, bytes);
										}
										MD5_Final (c,&mdContext);
										unsigned char hash[MD5_DIGEST_LENGTH];
										bzero(buffer,1023);
										char temp[100];
										for(i = 0; i < MD5_DIGEST_LENGTH; i++)
										{
											sprintf(temp, "%x",c[i]);
											strcat(buffer,temp);
										}
										fclose (inFile);
										strcat(buffer,"\n");
										printf("%s",buffer);
										sendto(ClientSocket, buffer,strlen(buffer), 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1));;

										bzero(buffer,1023);
										char sizeFile[100];
										sprintf(sizeFile,"%jd",fileStat.st_size); 
										strcat(sizeFile,"\n");
										printf("%s",sizeFile );
										strcpy(buffer,sizeFile);
										sendto(ClientSocket, buffer,strlen(buffer), 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1));;

										char tempTime[100];
										sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
										strcpy(buffer,tempTime);
										strcat(buffer,"  \n");
										sendto(ClientSocket, buffer,strlen(buffer), 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1));;
										printf("%s", buffer);
										break;
									}
								}
							}
							bzero(buffer,1023);
							sprintf(buffer,"%s\n","END");
							sendto(ClientSocket, buffer,strlen(buffer), 0,(struct sockaddr *)&serv_addr1, sizeof(serv_addr1));;
							fileCheckUp = 0;
							entryCheck = 0;
						}
						else if(strstr(uploadPermission,"FileUploadDeny\0") != NULL)
						{
							entryCheck = 0;
							fileCheckUp = 0;
						}
					}
				}
			}
		}
		else
		{
			// Acts as server as the port is empty 

			acting = "s";
			printf("[SERVER] SOCKET BINDED SUCCESSFULLY\n");
		}

		if(acting == "s")
		{
			if(udp_server == 1)
			{
				// Infinite loop for sending and receiving data

				while(1)
				{
					char buffer[1024];
					bzero(buffer,1024);

					// Checking status of client

					if(recvfrom(listenSocket,buffer,1023,0,(struct sockaddr *)&client_addr, &addr_len) < 0)
					{
						printf("Client Not responding well -- Closing -- Checking for new connection\n");
						goto checkpoint;
					}

					memset(str1,'\0',sizeof(str1));
					strcpy(str1,buffer);

					// Token1 is for main commmand and for flags

					char *token1=NULL;
					token1 = strtok(str1," ");
					printf("%s\n",buffer);
					if(strstr(token1, "IndexGet\0") != NULL)
					{
						token1 = strtok(NULL," ");
						if(strstr(token1, "longlist\0") != NULL)
						{
							// Code for sending all file related data

							DIR *dp;
							struct dirent *ep;
							dp = opendir("./");
							struct stat fileStat;
							if (dp != NULL)
							{

								int counter = 0;
								while (ep = readdir (dp))
								{

									if(stat(ep->d_name,&fileStat) < 0)
									{
										bzero(buffer,1023);
										strcpy(buffer,"Error occured");
										printf("%s\n", buffer );
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										return 0;
									}
									else
									{
										strcpy(longlist[counter].fileName, ep->d_name);
										longlist[counter].size = fileStat.st_size;
										longlist[counter].modTime = fileStat.st_mtime;
										strcpy(longlist[counter].type,(S_ISDIR(fileStat.st_mode)) ? "d  " : "-  ");
										bzero(buffer,1023);
										strcat(longlist[counter].fileName,"  ");
										strcpy(buffer,longlist[counter].fileName);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s", buffer);
										bzero(buffer,1023);
										strcpy(buffer,longlist[counter].type);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s", buffer);
										bzero(buffer,1023);
										char sizeFile[100];
										sprintf(sizeFile,"%jd",longlist[counter].size); 
										strcat(sizeFile,"  ");
										strcpy(buffer,sizeFile);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s",buffer);
										bzero(buffer,1023);
										char tempTime[100];
										sprintf(tempTime,"%s",ctime((&longlist[counter].modTime)));
										strcpy(buffer,tempTime);
										strcat(buffer,"  \n");
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s", buffer);
										counter++;
									}
								}
								bzero(buffer,1023);
								sprintf(buffer,"%s","END");
								printf("END\n");
								sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
								closedir (dp);
							}
							else
							{	
								strcpy(buffer,"Error occured");
								printf("%s\n", buffer );
								sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
								return 0;
							}
						}
						else if(strstr(token1, "shortlist\0") != NULL)
						{
							token1 = strtok(NULL," ");
							struct tm tm;
							strptime(token1, "%a %b %e %H:%M:%S %Y", &tm);
							time_t t = mktime(&tm);
							char temp[1000]; 
							sprintf(temp,"%s",ctime((&t)));
							token1 = strtok(NULL," ");
							strptime(token1, "%a %b %e %H:%M:%S %Y", &tm);
							time_t t1 = mktime(&tm);

							// Code for sending all file related data 

							DIR *dp;
							struct dirent *ep;
							dp = opendir("./");
							struct stat fileStat;
							if (dp != NULL)
							{
								int counter = 0;
								while (ep = readdir (dp))
								{

									if(stat(ep->d_name,&fileStat) < 0)
									{
										bzero(buffer,1023);
										strcpy(buffer,"Error occured");
										printf("%s\n", buffer);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										return 0;
									}
									else if(difftime(fileStat.st_mtime,t) > 0 && difftime(t1,fileStat.st_mtime) > 0)
									{
										strcpy(longlist[counter].fileName, ep->d_name);
										longlist[counter].size = fileStat.st_size;
										longlist[counter].modTime = fileStat.st_mtime;
										strcpy(longlist[counter].type,(S_ISDIR(fileStat.st_mode)) ? "d  " : "-  ");
										bzero(buffer,1023);
										strcat(longlist[counter].fileName,"  ");
										strcpy(buffer,longlist[counter].fileName);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s", buffer);
										bzero(buffer,1023);
										strcpy(buffer,longlist[counter].type);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s", buffer);
										bzero(buffer,1023);
										char sizeFile[100];
										sprintf(sizeFile,"%jd",longlist[counter].size); 
										strcat(sizeFile,"  ");
										strcpy(buffer,sizeFile);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s",buffer);
										bzero(buffer,1023);
										char tempTime[100];
										sprintf(tempTime,"%s",ctime((&longlist[counter].modTime)));
										strcpy(buffer,tempTime);
										strcat(buffer,"  \n");
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										printf("%s", buffer);
										counter++;
									}
								}
								bzero(buffer,1023);
								sprintf(buffer,"%s","END");
								printf("END\n");
								sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
								closedir (dp);
							}
							else
							{	
								strcpy(buffer,"Error occured");
								printf("%s\n", buffer );
								sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
								return 0;
							}
						}
						else if(strstr(token1, "regex\0") != NULL)
						{
							token1 = strtok(NULL," ");

							char regexInput[100];

							// For loop for removing new line character

							int s;
							for(s=0;s<strlen(token1) - 1;s++)
							{
								regexInput[s] = token1[s];
							}
							DIR *dp;
							struct dirent *ep;
							dp = opendir("./");
							struct stat fileStat;
							if (dp != NULL)
							{
								int counter = 0;
								while (ep = readdir (dp))
								{

									if(stat(ep->d_name,&fileStat) < 0)
									{
										bzero(buffer,1023);
										strcpy(buffer,"Error occured");
										printf("%s\n", buffer);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										return 0;
									}
									else
									{	
										// Code for regex matching 

										regex_t regex;
										int reti;
										char msgbuf[100];

										reti = regcomp(&regex,regexInput , 0);
										if( reti )
										{
											fprintf(stderr, "Could not compile regex\n"); 
											exit(1); 
										}
										reti = regexec(&regex, ep->d_name, 0, NULL, 0);
										if( !reti )
										{
											strcpy(longlist[counter].fileName, ep->d_name);
											longlist[counter].size = fileStat.st_size;
											longlist[counter].modTime = fileStat.st_mtime;
											strcpy(longlist[counter].type,(S_ISDIR(fileStat.st_mode)) ? "d  " : "-  ");
											bzero(buffer,1023);
											strcat(longlist[counter].fileName,"  ");
											strcpy(buffer,longlist[counter].fileName);
											sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
											printf("%s", buffer);
											bzero(buffer,1023);
											strcpy(buffer,longlist[counter].type);
											sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
											printf("%s", buffer);
											bzero(buffer,1023);
											char sizeFile[100];
											sprintf(sizeFile,"%jd",longlist[counter].size); 
											strcat(sizeFile,"  ");
											strcpy(buffer,sizeFile);
											sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
											printf("%s",buffer);
											bzero(buffer,1023);
											char tempTime[100];
											sprintf(tempTime,"%s",ctime((&longlist[counter].modTime)));
											strcpy(buffer,tempTime);
											strcat(buffer,"  \n");
											sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
											printf("%s", buffer);
											counter++;
										}
										else if( reti == REG_NOMATCH )
										{
										}
										else
										{
											bzero(buffer,1023);
											strcpy(buffer,"Error occured");
											printf("%s\n", buffer);
											sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
											return 0;
										}
										regfree(&regex);
									}
								}
							}		   
							bzero(buffer,1023);
							sprintf(buffer,"%s\n","END");
							printf("END\n");
							sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
					else if(strstr(token1, "FileHash\0") != NULL)
					{
						token1 = strtok(NULL," ");
						if(strstr(token1, "verify\0") != NULL)
						{
							token1 = strtok(NULL," ");

							// MD5 Hash of a file

							unsigned char c[MD5_DIGEST_LENGTH];
							char filename[100];

							// For loop for removing new line charater 

							int i1;
							for(i1=0;i1<strlen(token1)-1 ;i1++)
							{
								filename[i1] = token1[i1];
							}
							int i;
							FILE *inFile = fopen (filename, "r");
							MD5_CTX mdContext;
							int bytes;
							unsigned char data[1024];

							if (inFile == NULL) {
								printf ("%s can't be opened.\n", filename);
								return 0;
							}

							MD5_Init (&mdContext);
							while ((bytes = fread (data, 1, 1024, inFile)) != 0)
							{
								MD5_Update (&mdContext, data, bytes);
							}
							MD5_Final (c,&mdContext);
							unsigned char hash[MD5_DIGEST_LENGTH];
							bzero(buffer,1023);
							char temp[100];
							for(i = 0; i < MD5_DIGEST_LENGTH; i++)
							{
								sprintf(temp, "%x",c[i]);
								strcat(buffer,temp);
							}
							fclose (inFile);
							strcat(buffer,"\n");
							printf("%s",buffer);
							sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
							DIR *dp;
							struct dirent *ep;
							dp = opendir("./");
							struct stat fileStat;
							if (dp != NULL)
							{

								int counter = 0;
								while (ep = readdir (dp))
								{

									if(stat(ep->d_name,&fileStat) < 0)
									{
										bzero(buffer,1023);
										strcpy(buffer,"Error occured");
										printf("%s\n", buffer );
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										return 0;
									}
									else
									{
										if(strcmp(filename,ep->d_name)==0)
										{
											char tempTime[100];
											sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
											strcpy(buffer,tempTime);
											strcat(buffer,"  \n");
											sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
											printf("%s", buffer);
											break;
										}
									}
								}
							}
							bzero(buffer,1023);
							sprintf(buffer,"%s\n","END");
							printf("END\n");
							sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
						if(strstr(token1, "checkall\0") != NULL)
						{
							DIR *dp;
							struct dirent *ep;
							dp = opendir("./");
							struct stat fileStat;
							if (dp != NULL)
							{
								int counter = 0;
								while (ep = readdir (dp))
								{

									if(stat(ep->d_name,&fileStat) < 0)
									{
										bzero(buffer,1023);
										strcpy(buffer,"Error occured");
										printf("%s\n", buffer );
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										return 0;
									}
									else
									{
										unsigned char c[MD5_DIGEST_LENGTH];
										char filename[100];
										strcpy(filename,ep->d_name);
										bzero(buffer,1023);
										strcpy(buffer,filename);
										strcat(buffer,"\n");
										printf("%s",buffer);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										int i;
										FILE *inFile = fopen (filename, "r");
										MD5_CTX mdContext;
										int bytes;
										unsigned char data[1024];

										if (inFile == NULL) {
											printf ("%s can't be opened.\n", filename);
											return 0;
										}

										MD5_Init (&mdContext);
										while ((bytes = fread (data, 1, 1024, inFile)) != 0)
										{
											MD5_Update (&mdContext, data, bytes);
										}
										MD5_Final (c,&mdContext);
										unsigned char hash[MD5_DIGEST_LENGTH];
										bzero(buffer,1023);
										char temp[100];
										for(i = 0; i < MD5_DIGEST_LENGTH; i++)
										{
											sprintf(temp, "%x",c[i]);
											strcat(buffer,temp);
										}
										fclose (inFile);
										strcat(buffer,"\n");
										printf("%s",buffer);
										sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
										DIR *dp;
										struct dirent *ep;
										dp = opendir("./");
										struct stat fileStat;
										if (dp != NULL)
										{
											int counter = 0;
											while (ep = readdir (dp))
											{

												if(stat(ep->d_name,&fileStat) < 0)
												{
													bzero(buffer,1023);
													strcpy(buffer,"Error occured");
													printf("%s\n", buffer);
													sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
													return 0;
												}
												else
												{
													if(strcmp(filename,ep->d_name)==0)
													{
														char tempTime[100];
														sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
														strcpy(buffer,tempTime);
														strcat(buffer,"  \n");
														sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
														printf("%s", buffer);
														break;
													}
												}
											}
										}
									}
								}
							}
							bzero(buffer,1023);
							sprintf(buffer,"%s\n","END");
							printf("END\n");
							sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
						}
					}
					else if(strstr(token1, "FileDownload\0") != NULL)
					{
						token1 = strtok(NULL," ");
						char tempFile[1024];

						// For loop for removing new line character

						int s;
						for(s=0;s<strlen(token1) - 1;s++)
						{
							tempFile[s] = token1[s];
						}
						tempFile[s]='\0';

						// Sending file name

						bzero(buffer,1023);
						strcpy(buffer,tempFile);
						sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

						char* fs_name = tempFile;
						char sdbuf[1023]; 
						printf("Sending %s to the Client", fs_name);
						FILE *fs = fopen(fs_name, "r");
						if(fs == NULL)
						{
							fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
							exit(1);
						}
						bzero(sdbuf, 1023); 
						int fs_block_sz; 
						while((fs_block_sz = fread(sdbuf, sizeof(char), 1023, fs))>0)
						{
							if(sendto(listenSocket, sdbuf, fs_block_sz, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0)
							{
								fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
								exit(1);
							}
							bzero(sdbuf, 1023);
						}
						printf("Successfully sent to client.\n");

						DIR *dp;
						struct dirent *ep;
						dp = opendir("./");
						struct stat fileStat;
						if (dp != NULL)
						{
							int counter = 0;
							while (ep = readdir (dp))
							{
								if(stat(ep->d_name,&fileStat) < 0)
								{
									bzero(buffer,1023);
									strcpy(buffer,"Error occured");
									printf("%s\n", buffer );
									sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
									return 0;
								}
								else if(strstr(ep->d_name,fs_name)!=NULL)
								{
									unsigned char c[MD5_DIGEST_LENGTH];
									char filename[100];
									strcpy(filename,ep->d_name);
									int i;
									FILE *inFile = fopen (fs_name, "r");
									MD5_CTX mdContext;
									int bytes;
									unsigned char data[1024];
									if (inFile == NULL) {
										printf ("%s can't be opened.\n", fs_name);
										return 0;
									}

									MD5_Init (&mdContext);
									while ((bytes = fread (data, 1, 1024, inFile)) != 0)
									{
										MD5_Update (&mdContext, data, bytes);
									}
									MD5_Final (c,&mdContext);
									unsigned char hash[MD5_DIGEST_LENGTH];
									bzero(buffer,1023);
									char temp[100];
									for(i = 0; i < MD5_DIGEST_LENGTH; i++)
									{
										sprintf(temp, "%x",c[i]);
										strcat(buffer,temp);
									}
									fclose (inFile);
									strcat(buffer,"\n");
									printf("%s",buffer);
									sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

									bzero(buffer,1023);
									char sizeFile[100];
									sprintf(sizeFile,"%jd",fileStat.st_size); 
									strcat(sizeFile,"\n");
									printf("%s",sizeFile );
									strcpy(buffer,sizeFile);
									sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

									char tempTime[100];
									sprintf(tempTime,"%s",ctime((&fileStat.st_mtime)));
									strcpy(buffer,tempTime);
									strcat(buffer,"  \n");
									sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
									printf("%s", buffer);
									break;
								}
							}
						}
						bzero(buffer,1023);
						sprintf(buffer,"%s\n","END");
						printf("END\n");
						sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));
					}
					else if(strstr(token1, "FileUpload\0") != NULL)
					{
						//printf("Please type FileUploadAllow for allowing and FileUploadDeny for denying: ");
						//fgets(buffer,1023,stdin);
						if(strstr(uploadPermission,"FileUploadAllow\0") != NULL)
						{

							// Sending the approval permission

							//	sprintf(buffer,"%s\n","FileUploadAllow");
							//	printf("%s\n",buffer );
							//	sendto(listenSocket, buffer, 1023, 0,(struct sockaddr *)&client_addr, sizeof(struct sockaddr));

							// recieving the file name

							bzero(buffer,1024);
							if(recvfrom(listenSocket,buffer,1023,0,(struct sockaddr *)&client_addr, &addr_len) < 0)
								printf("ERROR while reading from the socket\n");

							// Code for receiving data

							char temp[1024];
							int s;
							for(s=0;s<strlen(buffer) - 1;s++)
							{
								temp[s] = buffer[s];
							}
							temp[s] = '\0';
							char* fr_name = temp;
							FILE *fr = fopen(fr_name, "a");
							int fr_block_sz = 0;
							while((fr_block_sz = recvfrom(listenSocket,buffer,1023,0,(struct sockaddr *)&client_addr, &addr_len)) > 0)
							{
								int write_sz = fwrite(buffer, sizeof(char), fr_block_sz, fr);
								if(write_sz < fr_block_sz)
								{
									error("File write failed.\n");
									return 0;
								}
								bzero(buffer, 1023);
								if (fr_block_sz == 0 || fr_block_sz != 1023) 
								{
									break;
								}
							}
							if(fr_block_sz < 0)
							{
								if (errno == EAGAIN)
								{
									printf("recv() timed out.\n");
									return 0;
								}					   
								else
								{
									fprintf(stderr, "recv() failed due to errno = %d\n", errno);
									return 0;
								}
							}
							printf("Successfully received the file.\n");
							fclose(fr);	

							while(strstr(buffer,"END\0")!=NULL)
							{
								bzero(buffer,1023);
								if(recvfrom(listenSocket,buffer,1023,0,(struct sockaddr *)&client_addr, &addr_len) < 0)
								{
									printf("ERROR while reading from the socket\n");
								}
								printf("%s\n",buffer );
							}
						}
					}
				}
			}
		}

	}
	return 0;
}
