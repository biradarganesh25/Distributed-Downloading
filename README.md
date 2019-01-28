# Distributed-Downloading
A collection of four files, which when run on multiple computers on the same subnet, enable to distribute the downloading of a file across all the computers, effectively reducing the time take to downlaod the entire file by a factor of number of computers participating. 

Any computer can initiate a download across all the other computers. Of course, the more the number of different computers trying to download different files, the lesser will be the visibility of the parallelism in downloading. The IP address of the participating computers can be set in the master.cpp file. 

Requirements: 
Linux operating system (The entire code was written using sockets library and also uses some linux specific system calls). 
All the participating computes must be on the same subnet. 

Working: 
Edit the master.cpp file to include all the participating computers. 
Run all the four files in g++ -std=c++11 filename.c
Give the file to download in the prompt asked by the master.cpp file. 
