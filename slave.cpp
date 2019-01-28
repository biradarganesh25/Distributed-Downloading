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

#define LENGTH 1024*1024

// #define TIMEOPT CURLINFO_TOTAL_TIME
// #define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL  2

// struct progress {
//   double lastruntime; 
//   CURL *curl;
// };

// char* index_global ;
// std::string index_global_string;


// int xferinfo(void *clientp,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow)
// {
// 	struct progress*main_prog = (struct progress *)clientp;
// 	CURL *curl = main_prog->curl;
// 	double curtime = 0;
// 	CURLcode res;
// 	curl_easy_getinfo(curl, TIMEOPT, &curtime);
// 	if((curtime - main_prog->lastruntime) >= 50)
// 	{
// 		main_prog->lastruntime = curtime;
// 		CURL *curl_post = curl_easy_init();
// 		if(curl_post){
// 			float percentage = (dlnow*1.0)/dltotal;
// 			std::string percentage_string = std::to_string(percentage);
// 			std::string jsonObj_string = "{\"percentage\" : " +  percentage_string +"}";
// 			//std::cout<<jsonObj_string<<std::endl;
// 			const char *jsonObj = jsonObj_string.c_str();
// 			/*fprintf(stderr, "UP: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
// 				"  DOWN: %" CURL_FORMAT_CURL_OFF_T " of %" CURL_FORMAT_CURL_OFF_T
// 				"\r\n",
// 				ulnow, ultotal, dlnow, dltotal);*/
// 			std::string curl_opt_string = "10.16.160.74:9517/progress/"+index_global_string;
// 			curl_easy_setopt(curl_post, CURLOPT_URL, curl_opt_string.c_str());
// 			curl_easy_setopt(curl_post, CURLOPT_POSTFIELDS, jsonObj);
// 			//curl_easy_setopt(curl_post, CURLOPT_POST, 1L);
// 			res = curl_easy_perform(curl_post);
// 			if(res != CURLE_OK)
// 			std::cout<<curl_easy_strerror(res)<<std::endl;
// 		}
// 		else
// 		std::cout<<"Error";
// 		} 
// return 0;
// }
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

std::string get_file_name(char * url, char * client_ip, char * index)
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
	std::string client_ip_string(client_ip);
	// cout<<"client ip string :"<<client_ip_string<<endl;
	std::string index_string(index);
	// cout<<"index string"<<index<<endl;
	std::string filename= client_ip_string +".part"+index_string + +"-"+s.substr(last);
	return filename;
}

std::string download_file(char * url, char * range, char * index, char * client_ip)
{
	CURL *curl;
	FILE *fw,*fr;
	CURLcode res;
	int readfd,connfd_read,n;
	std::string outfilename_string = get_file_name(url,client_ip, index);
	const char * outfilename = outfilename_string.c_str();
	cout<<"downloading to outfilename:"<<outfilename<<endl;
	cout<<"range: "<<range<<endl;
	
//	struct progress p;
	curl = curl_easy_init();
	 
	if (curl) 
	{
		// p.lastruntime = 0;
		// p.curl = curl;
		fw = fopen(outfilename,"wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);

		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 100); 
		curl_easy_setopt(curl, CURLOPT_RANGE, range);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		// curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
		// curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &p);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		cout<<curl_easy_strerror(res)<<endl;
		fclose(fw);
	}
	cout<<"Done downloading of :"<<index<<endl;
	return outfilename_string;
}

void send_file(int writefd,std::string fs_name){
	char *sdbuf=new char[LENGTH]; 
	int fd = open(fs_name.c_str(), 'r');
	int n;
	if(fd < 0)
	{
		cout<<"ERROR: File "<<fs_name <<"not found.\n" ;
		exit(1);
	}
	cout<<"Writing file: "<<fs_name;
	memset(sdbuf,0,LENGTH);
	int fs_block_sz; 
	//int success = 0;
	// while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))!=0)
	int total_file_size = 0;
	while((fs_block_sz = read(fd,sdbuf, LENGTH))!=0)
	{
		if((n=write(writefd, sdbuf, fs_block_sz) )< 0)
		{
			cout<<"ERROR: Failed to send file "<< fs_name<<".\n" ;
			break;
		}
		total_file_size += n;
		// printf("%d bytes sent\n",n);

		memset(sdbuf,0, LENGTH);
	}
	cout<<"File sent. Total size sent: "<<total_file_size<<endl;
	// cout<<"Ok File "<< fs_name<<" from Client was Sent!\n" ;

}


static void exit_cleanup(void)
{
	int n = sysconf(_SC_OPEN_MAX);
	for(int i = 0; i < n; i++)
	close(i);
}

int main(int argc, char * argv[])
{
	atexit(exit_cleanup);
	// cout<<"slave forked."<<endl;
	//0-connfd_read_string, 1-connfd_write_string, 2-client_ip
	std::string connfd_read_string(argv[0]);
	std::string connfd_write_string(argv[1]);
	char * client_ip = argv[2];

	int connfd_read = stoi(connfd_read_string);
	int connfd_write = stoi(connfd_write_string);

	char *buf =new char[12];
	char *buf_temp=buf;
	read(connfd_read,buf,12);

	int *url_length=new int;
	int *range_length=new int;
	int *index_length=new int;
	memcpy(url_length,buf,4);
	buf=buf+4;			
	memcpy(range_length,buf,4);
	buf=buf+4;			
	memcpy(index_length,buf,4);

	int total_length=*url_length+*range_length+*index_length;

	int n;
	char *new_buf=new char[total_length];
	char *new_buf_temp=new_buf;
	int total = 0;
	unsigned char temp1[200];
	while((n=read(connfd_read,temp1,200))!=0){
		std::memcpy(new_buf+total, temp1, n);
		total += n;
	}

	// std::cout<<"[SLAVE] "<<total<<std::endl;
	char *url=new char[*url_length+1];
	char *range=new char[*range_length+1];
	char *index=new char[*index_length+1];

	url[*url_length]='\0';
	range[*range_length]='\0';
	index[*index_length]='\0';

	memcpy(url,new_buf,*url_length);
	new_buf+=*url_length;
	memcpy(range,new_buf,*range_length);
	new_buf+=*range_length;
	memcpy(index,new_buf,*index_length);

		// //Index_global necessary for making the POST request.
		// memcpy(index_global,index,*index_length);

		// index_global_string = std::string(index_global);
/*		url=(char *)new_buf;
		new_buf=new_buf+(*url_length)+1;
		range=(char *)new_buf;		
		new_buf=new_buf+(*range_length)+1;
		index=(char *)new_buf;		

*/		std::cout<<"[SLAVE] "<<url<<std::endl;
		std::cout<<"[SLAVE] "<<range<<std::endl;
		std::cout<<"[SLAVE] "<<index<<std::endl;
		// int *
		delete[] buf_temp;
		delete[] new_buf_temp;

		close(connfd_read);
		//Downloading the actual file.
		std::string file_name = download_file(url, range, index, client_ip);
		// std::string file_name="/home/ubuntu/JP/hackpes/10.16.160.76.part0-bcompare-4.2.8.23479_amd64.deb";
		std::cout<<"[SLAVE] Sending file back to master\n";
		send_file(connfd_write,file_name);
		close(connfd_write);
	

	
}


