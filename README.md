# File-Sharing-Protocol

### Description
1. The goal is to create a application level File­ Sharing­ Protocol with support for download and upload for files and indexed searching.
2. The system should has 2 clients (acting as servers simultaneously) listening to the communication channel for requests and waiting to share files (avoiding collisions) using an application layer protocol.
3. Each client has the ability to do the following :
	- Know the files present on each other machines in the designated shared folders
	- Download files from this shared folder
	- Upload files to this shared folder

### How to run ?
- Socket Programming :
	- Compiling code : gcc server_client.c -lcrypto -lssl
	- To run code : ./a.out

- After running, it asks for the host port and then next server port. These should be vice versa while running the server and client. There is a FileUploadAllow or FileUploadDeny input to be given for the permissions for uploading files.

- And then :
	- To run commands through TCP protocol, select 0.
	- To run commads through UDP protocol, select 1.

### Commands Implemented
1. IndexGet longlist
2. IndexGet shortlist {timestamp} {Example: IndexGet shortlist WedJul0909:02:262014 FriFeb0618:23:442015
3. IndexGet regex <regular expression>
4. FileDownload <filename>
5. FileUpload <filename>
6. FileHash verify <filename>
7. FileHash checkall

