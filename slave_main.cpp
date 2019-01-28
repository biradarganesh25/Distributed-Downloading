//Socket headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//C++ headers. 
#include <string>
#include <iostream>
#include <curl/curl.h>

//C headers. 
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <fcntl.h>
#include <signal.h>

using std::cout;
using std::endl;
using std::cin;

void exit_cleanup()
{
    int n = sysconf(_SC_OPEN_MAX);
	for(int i = 0; i < n; i++)
	close(i);
}

int main()
{
    atexit(exit_cleanup);
    int readfd,connfd_read;
	struct sockaddr_in addr_read,client_read;

	// Descriptors to send the file back to the master
	int writefd, connfd_write;
	struct sockaddr_in addr_write,client_write;

	socklen_t len;
	
	readfd=socket(AF_INET,SOCK_STREAM,0);
	if(readfd < 0)
	{
		cout<<"[SLAVE] "<<"socket not created."<<endl;
	}
	addr_read.sin_family=AF_INET;
	addr_read.sin_port=htons(9515);
	addr_read.sin_addr.s_addr=INADDR_ANY;
	bind(readfd, (struct sockaddr *) &addr_read,sizeof(addr_read));
	cout<<"[SLAVE] "<<"Bind successfull for reading on 9515\n"<<endl;
	listen(readfd,5);

    //Socket for writing on 9516
	writefd=socket(AF_INET,SOCK_STREAM,0);
	if(writefd < 0)
	{
		cout<<"[SLAVE] "<<"socket created."<<endl;
	}

	addr_write.sin_family=AF_INET;
	addr_write.sin_port=htons(9516);
	addr_write.sin_addr.s_addr=INADDR_ANY;
	bind(writefd, (struct sockaddr *) &addr_write,sizeof(addr_write));
	cout<<"[SLAVE] "<<"Bind successfull for write on 9516\n"<<endl;
	listen(writefd,5);

    for(;;)
	{
		// Accept connection to read on 9515
        // cout<<"Accepting connections:"<<endl;
		len=sizeof(client_read);
		connfd_read=accept(readfd,(struct sockaddr *) &client_read,&len);
		cout<<"[SLAVE] "<<"Connection accepted on 9515"<<endl;

        char * client_ip = new char[20];
        inet_ntop(AF_INET, &client_read.sin_addr, client_ip, 20);
        
        // cout<<"client ip:"<<client_ip<<endl;
		

		//Setting the alarm so that if this connection fails, it 
		//calls the handler which will clean up and restart the program.
		// alarm(10);

		// Accept connection to write on 9516
		len=sizeof(client_write);
        int connection_flag = 0;
        char * client_ip_temp = new char[20];
        // connfd_write=accept(writefd,(struct sockaddr *) &client_write,&len);
        // inet_ntop(AF_INET, &client_write.sin_addr, client_ip_temp, 20);
        // cout<<"client ip:"<<client_ip_temp<<endl;
        
        while(connection_flag == 0)
        {
            connfd_write=accept(writefd,(struct sockaddr *) &client_write,&len);
            inet_ntop(AF_INET, &client_write.sin_addr, client_ip_temp, 20);
            // cout<<"client ip:"<<client_ip_temp<<endl;
            if(strcmp(client_ip, client_ip_temp) == 0)
            {
                connection_flag = 1;
                
            }
            else
            {
                cout<<"Multiple connections requested."<<endl;
                close(connfd_write);
            }
        }
        delete[] client_ip_temp;
		
		cout<<"[SLAVE] "<<"Connection accepted on 9516"<<endl;
        
        std::string connfd_read_string = std::to_string(connfd_read);
        std::string connfd_write_string = std::to_string(connfd_write);
        
        int pid = fork();
        if(pid == 0)
        {
            execl("./slave", connfd_read_string.c_str(), connfd_write_string.c_str(), client_ip, (char *)0);
        }
        close(connfd_read);
        close(connfd_write);
        delete[] client_ip;


    }
    close(readfd);
    close(writefd);


}