//Socket headers.
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//C++ headers. 
#include <string>
#include <iostream>
#include <curl/curl.h>

//C headers. 
#include <stdlib.h>
#include <string.h>
#include<fcntl.h>
#include <unistd.h>

using std::cout;
using std::endl;

#define WRITE_PORT 9515
#define READ_PORT 9516

typedef struct download_url
{
    std::string url;
    std::string index;
    std::string range;
}download_url;

std::string get_file_name(const char * url,const char * index)
{
	int pos;
	std::string s(url);
	std::string delimiter = "/";
	std::string token = s.substr(0, s.find(delimiter));
    size_t last = 0; 
    size_t next = 0; 
    while ((next = s.find(delimiter, last)) != std::string::npos) {
    	// cout << s.substr(last, next-last) << endl; 
    	last = next + 1; 
    } 
    // cout << "substring:"<<s.substr(last) << endl;
	std::string index_string(index);
	// cout<<"index string"<<index<<endl;
    std::string filename= "part"+index_string +"-"+s.substr(last);
	// cout<<"filean:"<<filename<<endl;
	return filename;
}

void receive_file(int readfd,const char*url,const char*index)
{
    int n;
    char * buf = new char[100];
    std::string filename_string  = get_file_name(url,index);
    const char * filename = filename_string.c_str();
    cout<<"filename :"<<filename<<endl;
    
    int file_fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if( file_fd < 0)
    {
        cout<<"Error opening file"<<endl;
    }
    int written  = 0, total = 0;

    while( (n = read(readfd,buf,99))!= 0)
    {
        if( ( written = write(file_fd,buf,n) ) < 0)
        {
            cout<<"Error writing to file"<<endl;
            break;
        }   
        total += written;
    }
    system(("chmod +rw "+filename_string).c_str());
    cout<<"total bytes written: "<<total<<endl;
    close(file_fd);
    close(readfd);
}

//1 - ip address, 2 - url, 3 - range, 4 - index (for argv)
int main(int argc, char * argv[])
{
    // cout<<"args 0:"<<argv[0]<<endl;
    int writefd,readfd, n;
    struct sockaddr_in servaddr_write, servaddr_read;     

    if( (writefd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout<<"Not able to open socket."<<std::endl;
        exit(0);
    }
    // else
    // {
    //     std::cout<<"Socket created.";
    // }

    if( (readfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout<<"Not able to open socket."<<std::endl;
        exit(0);
    }
    // else
    // {
    //     std::cout<<"Socket created.";
    // }

    memset(&servaddr_write, 0, sizeof(servaddr_write));

    servaddr_write.sin_family = AF_INET;
    servaddr_write.sin_port = htons(WRITE_PORT);
    inet_pton(AF_INET, argv[0], &servaddr_write.sin_addr);    

    servaddr_read.sin_family = AF_INET;
    servaddr_read.sin_port = htons(READ_PORT);
    inet_pton(AF_INET, argv[0], &servaddr_read.sin_addr);
    
    if( (connect(writefd, (sockaddr*)&servaddr_write, sizeof(servaddr_write))) < 0)
    {
        cout<<"write connect error, "<<"index :"<<argv[3]<<endl;
        exit(0);
    }
    // else
    // {
    //     cout<<"write Connected"<<endl;
    // }

    if( (connect(readfd, (sockaddr*)&servaddr_read, sizeof(servaddr_read))) < 0)
    {
        cout<<"read connect error"<<"index :"<<argv[3]<<endl;
        exit(0);
    }

    // else
    // {
    //     cout<<"read Connected"<<endl;
    // }

    
    download_url url_struct;
    // url_struct.url = "http://dl1.irani-dl.com/serial/The%20Legend%20of%20Korra/Season%201/The%20Legend%20of%20Korra-S01E01E02.720p.WEB-DL.x264(www.irani-dl.ir).mkv";
    // url_struct.range = "0-1000";
    // url_struct.index = "0";

    url_struct.url = std::string(argv[1]);
    url_struct.range = std::string(argv[2]);
    url_struct.index = std::string(argv[3]);

    int size = url_struct.url.length() + url_struct.index.length() + url_struct.range.length(),bytes;
    int url_length = url_struct.url.length();
    int range_length = url_struct.range.length();
    int index_length = url_struct.index.length();
    
    char * buffer = new char[size+1];
    memcpy(buffer, (void *)&url_length, 4);    
    memcpy(buffer+4, (void *)&range_length, 4);
    memcpy(buffer+8, (void *)&index_length,4);

    if((bytes = write(writefd, (void *)buffer,12)) < 12)
    {
        cout<<"Error writing.";
    }

    const char *url  = url_struct.url.c_str();
    const char *range = url_struct.range.c_str();
    const char *index = url_struct.index.c_str();
    
    memcpy(buffer, (void *)url, url_length);
    memcpy(buffer+url_length, (void *)range, range_length);
    memcpy(buffer+range_length+url_length, (void *)index, index_length);

    if((bytes = write(writefd, (void *)buffer,size)) < size)
    {
        cout<<"Error writing.";
    }
    // else
    // {
    //     cout<<"bytes written:"<<bytes<<endl;
    // }
    delete[] buffer;
    close(writefd);  

    //Getting back the chunks of file. 
    receive_file(readfd,url,index);
    
    cout<<"index :"<<index<<"exited"<<endl;

}