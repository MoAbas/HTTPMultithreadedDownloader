#include "util.h"
#include <pthread.h>
#include <libgen.h>

struct request_args {
        int threadNum;
        char start[MAX_STR_LEN];
        char end[MAX_STR_LEN];
        int isHeadRequest;
    };
void *sendRequest(void *args);
void mergeFiles();
int fileSize;
int fileSizeDivided;
char url[MAX_STR_LEN];
int NUM_THREADS;

int main(int argc, char * argv[]) {
    if (argc < 3) {
      printf("\n Not Enough Arguments \n");
      exit(ERROR);
    }
    //assign url and number of threads from user input
    strcpy(url, argv[1]);
    NUM_THREADS = strtol(argv[2], NULL, 10);
    //calling sendRequest function to send a Head request and get file size from url
    struct request_args *StructHeadRequest;
    StructHeadRequest = malloc(sizeof(struct request_args));
    (*StructHeadRequest).isHeadRequest = 1;
    sendRequest((void*)StructHeadRequest);
    //after the Head request we have a valid file size to divide by number of threads
    fileSizeDivided = fileSize/NUM_THREADS;
    //declaring start and end string arrays to specify each thread's starting and ending bytes to download
    char start[NUM_THREADS][MAX_STR_LEN];
    //starting byte for first thread is always 0
    strcpy(start[0], "0");
    int startCnt;
    for(startCnt = 1; startCnt<NUM_THREADS;startCnt++){
        sprintf(start[startCnt], "%d", (startCnt*fileSizeDivided)+1);
    }
    char end[NUM_THREADS][MAX_STR_LEN];
    int endCnt;
    for(endCnt = 0; endCnt<NUM_THREADS-1;endCnt++){
        sprintf(end[endCnt], "%d", (endCnt*fileSizeDivided)+fileSizeDivided);
    }
    //ending byte for last thread is always equal to file size
    sprintf(end[NUM_THREADS-1], "%d", fileSize);
    //creating threads and initializing a struct to pass to sendRequest function
    int cnt;
    pthread_t p_thread[NUM_THREADS];
    struct request_args *StructThread[NUM_THREADS];
    for (cnt = 0; cnt < NUM_THREADS; cnt++){
        //allocating memory for every structure
        StructThread[cnt] = malloc(sizeof(struct request_args));
        //filling every structure with values
        (*StructThread[cnt]).threadNum = cnt;
        strcpy((*StructThread[cnt]).start, start[cnt]);
        strcpy((*StructThread[cnt]).end, end[cnt]);
        if (pthread_create( & p_thread[cnt], NULL, sendRequest, (void*)StructThread[cnt]) != 0)
            fprintf(stderr, "Error creating the thread");
    }
    //Cycle through each thread and wait until it is completed.
    for (cnt = 0; cnt < NUM_THREADS; cnt++) {
//        printf("\nWaiting for Thread %d\n",cnt);
        pthread_join(p_thread[cnt], NULL);
    }
    //start merging downloaded partial files
    mergeFiles();
    return SUCCESS;
}

void mergeFiles(){
    //method to merge every partial file into a new output file
    FILE *outputFile;
    FILE *partialFile[NUM_THREADS];
    int ch;
    char fname[NUM_THREADS][20];
    printf("\n\n\t\t--Merging Partial Files--\n");
    outputFile=fopen(basename(url), "w");
    if(outputFile==NULL){
        printf("Can't Create New Output File...!!\n");
        exit(EXIT_FAILURE);
    }
    int cnt;
    for(cnt=0;cnt<NUM_THREADS;cnt++){
        sprintf(fname[cnt], "part%d", cnt);
        partialFile[cnt]=fopen(fname[cnt],"ab+");
        if(partialFile[cnt]==NULL){
            printf("Can't Create New Partial File...!!\n");
            exit(EXIT_FAILURE);
        }
        printf("\t\t.Merging File %s.\n",fname[cnt]);
        while((ch=fgetc(partialFile[cnt]))!=EOF){
            fputc(ch, outputFile);
        }
        fclose(partialFile[cnt]);
    }
    printf("\t\t--Success!--\n");
    fclose(outputFile);
}

void *sendRequest(void *args){
    struct request_args *my_args = (struct request_args*)args;
    char * request = 0;
    char identifier[MAX_STR_LEN];
    char hostname[MAX_STR_LEN];
    struct sockaddr_in localAddr, servAddr;
    struct hostent * h;
    int port;
    int sd, rc;
    char buf[MAX_STR_LEN];
    char buffer[BUFFER_SIZE];
    char line[MAX_MSG];
    int len, size;
    
    request = buf;
    parse_URL(url, hostname, & port, identifier);
    //((*my_args).isHeadRequest) statement to check if the request is Head or Get(thread request to download)
    if((*my_args).isHeadRequest)
        printf("\n-- Hostname = %s , Port = %d , Identifier = %s\n", hostname, port, identifier);
  
    h = gethostbyname(hostname);
    if (h == NULL) {
      printf("unknown host: %s \n ", hostname);
      exit(ERROR);
    }
    servAddr.sin_family = h -> h_addrtype;
    memcpy((char * ) & servAddr.sin_addr.s_addr, h -> h_addr_list[0], h -> h_length);
    servAddr.sin_port = htons(port); //(LOCAL_SERVER_PORT);
    // create socket 
    if((*my_args).isHeadRequest)
        printf("-- Create socket...    ");
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
      perror("cannot open socket ");
      exit(ERROR);
    }
    //  bind port number 
    if((*my_args).isHeadRequest)
        printf("Bind port number...  ");
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);
    rc = bind(sd, (struct sockaddr * ) & localAddr, sizeof(localAddr));
    if (rc < 0) {
      printf("cannot bind port TCP %d\n", port); //(LOCAL_SERVER_PORT);
      perror("error ");
      exit(ERROR);
    }
    // connect to server 
    if((*my_args).isHeadRequest)
        printf("Connect to server...\n");
    rc = connect(sd, (struct sockaddr * ) & servAddr, sizeof(servAddr));
    if (rc < 0) {
      perror("cannot connect ");
      exit(ERROR);
    }
    //send Head or Get request
    if((*my_args).isHeadRequest)
        strcpy(request, "HEAD ");
    else
        strcpy(request, "GET ");
    request = (char * ) strcat(request, identifier);
    request = (char * ) strcat(request, " HTTP/1.1\r\nHOST: ");
    request = (char * ) strcat(request, hostname);
    request = (char * ) strcat(request, "\r\n");
    if(!(*my_args).isHeadRequest){
        request = (char * ) strcat(request, "RANGE: bytes=");
        request = (char * ) strcat(request, (*my_args).start);
        request = (char * ) strcat(request, "-");
        request = (char * ) strcat(request, (*my_args).end);
        request = (char * ) strcat(request, "\r\n");
        request = (char * ) strcat(request, "\r\n");
    }
    request = (char * ) strcat(request, "\r\n");
    // send request
    if((*my_args).isHeadRequest)
        printf("-- Send HTTP request:\n\n%s", request);
    rc = write(sd, request, strlen(request));
    if (rc < 0) {
      perror("cannot send data ");
      close(sd);
      exit(ERROR);
    }
    //display response
    memset(buffer, 0x0, BUFFER_SIZE); //  init line 
    if ((rc = read(sd, buffer, BUFFER_SIZE)) < 0) {
      perror("read");
      exit(1);
    }
    char sizeTag[1000];
    strcpy(sizeTag, "Content-Length: ");
    sprintf(sizeTag, "%s", strstr(buffer, sizeTag) + strlen(sizeTag));
    //is request is Head then display response and get file size from url
    if((*my_args).isHeadRequest){
        printf("-- Recieved response:\n\tfrom server: %s, IP = %s,\n\tlength = %d,%zu\n\n%s\n\n",url, inet_ntoa(servAddr.sin_addr), rc, strlen(buffer), buffer);
        fileSize = atoi(sizeTag);
    }
    //else read bytes from socket
    else{
        //set file name to be created as part[thread Number]
        char name[40];
        sprintf(name, "part%d", (*my_args).threadNum);	
        size = rc - (strstr(buffer, "\r\n\r\n") - buffer + 4);
	//recalculate the partial size
	int fileSizePartial = atoi(sizeTag);
        int remaining = fileSizePartial - size;
    //    printf("  %8s  %8s %8s\n", "So far", "bytes", "Remaining");
        printf("\n\t--Thread %d  Starting Byte: %s\t\tEnding Byte: %s",(*my_args).threadNum, (*my_args).start, (*my_args).end);
        FILE * fp;
        fp = fopen(name, "w+b");
        fwrite( & buffer[strstr(buffer, "\r\n\r\n") - buffer + 4], 1, size, fp);
        rc = 0;
        do {
          memset(buffer, 0x0, BUFFER_SIZE); //  init line
          //read bytes from sockeet to buffer
          if (remaining < BUFFER_SIZE)
            rc = read(sd, buffer, remaining);
          else
            rc = read(sd, buffer, BUFFER_SIZE);
          //write bytes from buffer to a partial file
          if (rc > 0) {
            fwrite(buffer, 1, rc, fp);
            size += rc;
            remaining -= rc;
    //        printf("  %8d  %8d %8d\n", size, rc, remaining);
          }
        }
        while ((rc > 0) && (remaining > 0));
        fclose(fp);
        printf("\n\t**Total recieved response bytes for thread %d: %d", (*my_args).threadNum,size);
    }
    close(sd);
}