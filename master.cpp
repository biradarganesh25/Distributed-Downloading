//Socket headers.
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//C++ headers. 
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

//C headers. 
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <curl/curl.h>
#include <stdlib.h>
#include <sys/wait.h>


#define WRITE_PORT 9515
#define READ_PORT 9516

typedef struct download_url
{
    std::string url;
    std::string index;
    std::string range;
}download_url;

struct user_data
{
    int size;
};

std::string get_file_name(const char * url)
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
	return s.substr(last);
}

using std::cout;
using std::endl;
using std::cin;

size_t header_callback(char *buffer,   size_t size,   size_t nitems,   void *usr_data_temp)
{
    user_data *usr_data = (struct user_data *)usr_data_temp;
    
    std::istringstream iss(buffer);
    std::string s;
    while ( std::getline( iss, s, ' ' ) ) {
        if(s == "Content-Length:")
        {
            std::getline(iss, s, ' ');
            std::istringstream temp(s);
            temp>>(usr_data->size);
            // cout<<s<<endl;
            // cout<<buffer<<endl;
        }
        
    }    
    // cout<<buffer<<endl;
    return strlen(buffer);
}



int get_file_size(download_url url_struct)
{
    user_data usr_data;
    curl_global_init(CURL_GLOBAL_ALL);
    CURL * curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url_struct.url.c_str());
    curl_easy_setopt(curl_handle,CURLOPT_NOBODY ,1 );
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &usr_data);
    CURLcode res = curl_easy_perform(curl_handle);


    if(res != CURLE_OK)
    {
        cout<<curl_easy_strerror(res)<<endl;
    }
    
    return usr_data.size;
}

int main()
{
    // cout<<"hi"<<endl;
    download_url url_struct;
    cout<<"Enter the url to download: "<<endl;
    
    // std::cout<<"Enter the download url:"<<std::endl;
    std::cin>>url_struct.url;
    // url_struct.url = "http://www.inkjettips.com/chapter2.pdf";

    int file_size = get_file_size(url_struct);
    // cout<<file_size<<endl;
    std::vector< std::string > ip_address = { "10.16.160.74", "10.16.160.76", "10.16.160.72"};

    int num_slaves = ip_address.size();
    int size_per_chunk = file_size / num_slaves;
    int pid;
    for(int i = 0; i < num_slaves; i++)
    {
        std::string index = std::to_string(i);
        int start_num = i * size_per_chunk, end_num = std::min(start_num + size_per_chunk - 1, file_size);
        std::string start = std::to_string(start_num), end = std::to_string(end_num);
        std::string range = start+"-"+end;       
        //1 - ip address, 2 - url, 3 - range, 4 - index (for argv)
        pid = fork();
        if(pid == 0)
        execl("./master_fork", ip_address[i].c_str(), url_struct.url.c_str(),\
         range.c_str(),index.c_str(), (char *)0);

        
    }
    int status, wpid;
    while ((wpid = wait(&status)) > 0);
    
    // part0-bcompare-4.2.8.23479_amd64.deb(A sample file name)

    system(("cat part*-"+get_file_name(url_struct.url.c_str())+" > "+get_file_name(url_struct.url.c_str())).c_str());
    

    return 0;
}