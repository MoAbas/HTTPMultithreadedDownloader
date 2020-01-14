### Multithreaded HTTP Download Manager
##### Partial HTTP Downloads with multithreaded HTTP client implemented in C using Pthreads on a Linux machine.<br/>The program takes two command line arguments:
###### - URL of the target file stored on a web server<br/>- Number of threads to be used in download operation.
##### Each thread is responsible for downloading one of the equal sub parts of the target file.<br/><br/>Example Usage:<br/>
```
$ gcc -pthread -o http_client.exe http_client.c
$ ./http_client.exe TargetFileURL NumberOfThreads
```
##### With Example arguments:<br/>
`$ ./http_client.exe http://www.hochmuth.com/mp3/Haydn_Cello_Concerto_D-1.mp3 5`
