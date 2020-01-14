### Multithreaded HTTP Download Manager
##### Partial HTTP Downloads with multithreaded HTTP client implemented in C using Pthreads on a linux machine.<br/>The program takes two command line arguments:
###### - URL of the target file stored on a web server<br/>- The number of threads to be used in download operation.
##### Each thread is responsible for downloading one of the equal sub parts of the target file.
