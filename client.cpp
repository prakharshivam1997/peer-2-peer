#include<bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <openssl/sha.h>
#include <thread>
#include <openssl/sha.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/ip.h>  
#define BUFF 2048
#define PORT 5042
#define TRACKER_PORT 5437
using namespace std;
vector<thread>threadVector;
void clientRequest(string clientIPort,string tracker_info_path)
{
    //std::ifstream file("prak.txt");
    {
        std::ofstream outfile;
        outfile.open(tracker_info_path, std::ios_base::app); // append instead of overwrite
        outfile <<clientIPort+"\n"; 
    }
}
vector<string> stringSlicer(string str,char del)
{
    stringstream ss(str);
    vector<string>a;
    string temp;
    while(getline(ss,temp,del))
    {
        a.push_back(temp);
    }
    return a;
}
string IpParse(string str)
{
    int i=0;
    for(i=0;i<str.length();i++)
    {
        if(str[i]==':')
        {
            break;
        }
        else
        {
            continue;
        }
    }
    return str.substr(0,i-1);
}
void serveRequest(int sock,int portx)
{
    char input[1024];
    cout<<"In serving request area\n";
    recv(sock,input, 1024, 0);
    //cout<<input<<endl;
    string inputOut=input;
    vector<string>slicedArray=stringSlicer(inputOut,';');
    //slicedArray[0]=download peer ip and port slicedArray[1]=download file address slicedArray[2]=file size
    int inputPort=stoi(stringSlicer((slicedArray[0]),':')[1]);
    int size=stoi(slicedArray[2]);
    //cout<<inputPort<<endl;
    int new_sock1;
    struct sockaddr_in peer_addrx;
    bzero((char *) &peer_addrx, sizeof(peer_addrx));          //first socket assigned and bound, used as thread client 
    if ((new_sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {    
        printf("\n Socket creation error \n");  
    }
    peer_addrx.sin_family = AF_INET; 
    peer_addrx.sin_port = htons(inputPort);
    if(inet_pton(AF_INET, "127.0.0.1", &peer_addrx.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
    } 
    if (connect(new_sock1, (struct sockaddr *)&peer_addrx, sizeof(peer_addrx)) < 0) 
    {    
        printf("\nConnection to download peer Failed \n");  
    }
    //cout<<"request for connection\n";
    float csize=32*1024.0;
    char status[]="1";
    send(new_sock1,status,strlen(status),0);
    //cout<<"sending data try\n";
    int nchunks=ceil((float)size/csize);
    //cout<<nchunks<<endl;
    char mybuffer[32*1024];
    memset(mybuffer,0,sizeof(mybuffer));
    std::ifstream fin(slicedArray[1], ios::in );
    for(int i=1;i<=nchunks;i++)
    {
        fin.read(mybuffer, csize);
        //cout<<"sending in chunk loop "<<i<<endl;
        //cout<<mybuffer<<endl;
        send(new_sock1,mybuffer,strlen(mybuffer),0);
    }
    cout<<"file has been sent\n";    
}
void inExecution(int sock,int iport)
{
    int authLogin=0;
    int authGroup=0; 
    string userInput;
    cout<<"ready for input"<<endl;
    while(1)
    {
        struct sockaddr_in tracker_addr;
        bzero((char *) &tracker_addr, sizeof(tracker_addr));
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        { 
            printf("\n Socket creation error \n");  
        }
        tracker_addr.sin_family = AF_INET; 
        tracker_addr.sin_port = htons(TRACKER_PORT);
        if(inet_pton(AF_INET, "127.0.0.1", &tracker_addr.sin_addr)<=0)  
        { 
            printf("\nInvalid address/ Address not supported \n"); 
        } 
        if (connect(sock, (struct sockaddr *)&tracker_addr, sizeof(tracker_addr)) < 0) 
        { 
            printf("\nConnection to tracker Failed \n");  
        }
        cin>>userInput;
        if(userInput=="create_user")
        {
            string userId,passwd;
            cin>>userId>>passwd;
            string token=userInput+";"+userId+";"+passwd+'\0';
            char input[1048];
            send(sock , token.c_str() ,strlen(token.c_str()), 0 );
            int bytesReceived = recv(sock, input, 100, 0);
            //cout<<"recieved is"<<input<<endl;
            if(input[0]=='1')
            {
                cout<<"New user has been created successfully"<<endl;
            }
            else if(input[1]=='0')
            {
                cout<<"attempt was unsuccessful, try again"<<endl;
            }
            //cout<<token<<endl;
            userId.clear();
            passwd.clear();
        }
        if(userInput=="login")
        {
            string userId,passwd;
            cin>>userId>>passwd;
            string token=userInput+";"+userId+";"+passwd+";"+to_string(iport) +"\0";
            char input[1048];
            //cout<<token<<endl;
            send(sock , token.c_str() ,strlen(token.c_str()), 0 );
            int bytesReceived = recv(sock, input, 100, 0);
            //cout<<"recieved is"<<input<<endl;
            if(input[0]=='1')
            {
                authLogin=1;
                cout<<"login done"<<endl;
            }
            else if(input[0]=='0')
            {
                cout<<"login was unsuccessful, try again"<<endl;
            }
            //cout<<"relooping"<<endl;
        }
        if(userInput=="create_group")
        {
            string groupId;
            cin>>groupId;
            if(authLogin==0)
            {
                 cout<<"You are not logged in,try again\n";
            }
            else
            {
                char input[2048];
                string token=userInput+";"+groupId;
                //cout<<token;
                memset(input,0,sizeof(input));
                send(sock ,token.c_str() ,strlen(token.c_str()), 0 );
                int bytesReceived = recv(sock,input, 100, 0);
                if(input[0]=='1')
                {
                    cout<<"group created successfully"<<endl;
                }
                else
                {
                    cout<<"group was not created,try again"<<endl;
                }
            }
        }
        if(userInput=="join_group")
        {
            string groupId;
            cin>>groupId;
            if(authLogin==0)
            {
                cout<<"You are not logged in,try again\n";
            }
            else
            {

                char input[1024];
                string token=userInput+";"+groupId+";"+to_string(iport);
                memset(input,0,sizeof(input));
                send(sock,token.c_str(),strlen(token.c_str()),0);
                recv(sock,input, 100, 0);
                if(input[0]=='1')
                {
                    authGroup=1;
                    cout<<"group join request has been made"<<endl;
                }
                else if(input[0]=='0')
                {
                    cout<<"group join request failed"<<endl;
                }
            }
        }
        if(userInput=="list_requests")
        {
            string groupId;
            cin>>groupId;
            if(authLogin==0)
            {
                 cout<<"You are not logged in,try again\n";
            }
            else
            {
                char input[1024];
                string token=userInput+";"+groupId;
                memset(input,0,sizeof(input));
                send(sock,token.c_str(),strlen(token.c_str()),0);
                recv(sock,input, 1024, 0);
                vector<string>cutArray=stringSlicer(input,';');
                for(auto itr=cutArray.begin();itr!=cutArray.end();itr++)
                {
                    cout<<*itr<<" ";
                }
                cout<<endl;
            }
        }
        if(userInput=="leave_group")
        {
            string groupId;
            cin>>groupId;
            if(authLogin==0)
            {
                 cout<<"You are not logged in,try again\n";
            }
            else
            {
                char input[1024];
                string token=userInput+";"+groupId+";"+to_string(iport);
                memset(input,0,sizeof(input));
                send(sock,token.c_str(),strlen(token.c_str()),0);
                recv(sock,input, 100, 0);
                if(input[0]=='1')
                {
                    cout<<"successful removed from group"<<endl;
                }
                else
                {
                    cout<<"unsuccessful in removing"<<endl;
                }
            }
            
        }
        if(userInput=="list_groups")
        {
            char input[1024];
            memset(input,0,sizeof(input));
            string token="list_groups";
            send(sock,token.c_str(),strlen(token.c_str()),0);
            recv(sock,input, 1024, 0);
            vector<string>stringArray=stringSlicer(input,';');
            for(auto itr=stringArray.begin();itr!=stringArray.end();itr++)
            {
                cout<<*itr<<" ";
            }
            cout<<endl;
        }
        if(userInput=="accept_request")
        {
            string groupId,userId;
            cin>>groupId>>userId;
            string token=userInput+";"+groupId+";"+userId+";"+to_string(iport);
            send(sock,token.c_str(),strlen(token.c_str()),0);
            char input[1024];
            memset(input,0,sizeof(input));
            recv(sock,input, 1024, 0);
            //cout<<"input received is ->"<<input<<endl;
            if((input[0]=='-')&&(input[1]=='1'))                       //check if input is -1
            {
                cout<<"Not leader, cannot give accept request\n";
            }
            else if(input[0]=='0')
            {
                cout<<"Failed to accept request\n";
            }
            else
            {
                cout<<"Successful to accept request\n";
            }
            
        }
        if(userInput=="upload_file")
        {
            string filePath,groupId;
            cin>>filePath>>groupId;
            if(authLogin==0)
            {
                cout<<"You are not logged in,try again\n";
            }
            else
            {
                //if(authGroup==0)
                {
                    //cout<<"Not in any group, try again\n";
                }
                //else
                {
                    ifstream in(filePath,ios::ate|ios::binary);
	                int size=in.tellg();
	                in.close();
                    char input[1024];
                    string token=userInput+";"+filePath+";"+groupId+";" + to_string(iport)+";"+to_string(size);
                    //cout<<token<<endl;
                    send(sock,token.c_str(),strlen(token.c_str()),0);
                    recv(sock,input, 1024, 0);
                    if(input[0]=='1')
                    {
                        cout<<"File has been uploaded and seeder list has been updated"<<endl;
                    }
                    else
                    {
                        cout<<"Failed to upload file\n";
                    }
                }
            }
        }
        if(userInput=="list_files")
        {
            string groupId;
            cin>>groupId;
            if(authLogin==0)
            {
                cout<<"You are not logged in,try again\n";
            }
            else
            {
                //if(authGroup==0)
                {
                    //cout<<"Not in any group, try again\n";
                }
                //else
                {
                    char input[1024];
                    string token=userInput+";"+groupId;
                    send(sock,token.c_str(),strlen(token.c_str()),0);
                    recv(sock,input, 1024, 0);
                    if(input[0]=='0')
                    {
                        cout<<"Nothing here\n";
                    }
                    else
                    {
                        cout<<input<<endl;
                    }
                }
            }
        }
        if(userInput=="download_file")
        {
            char input[1024];
            string groupId,fileName,destPath;
            cin>>groupId>>fileName>>destPath;
            if(authLogin==0)
            {
                cout<<"You are not logged in,try again\n";
            }
            else
            {
                //if(authGroup==0)
                {
                    //cout<<"Not in any group,try again\n";
                }
                //else
                {
                    string token=userInput+";"+groupId+";"+fileName+";"+ to_string(iport);
                    send(sock,token.c_str(),strlen(token.c_str()),0);
                    recv(sock,input, 1024, 0);
                    //cout<<input<<endl;
                    string trackerOut=input;
                    vector<string>slicedArray=stringSlicer(trackerOut,';');
                    //slicedArray[0]=sourceIpPort slicedArray[1]="source file address" slicedArray[2]="size"
                    vector<string>ipPort=stringSlicer(slicedArray[0],':');
                    string myip=inet_ntoa(tracker_addr.sin_addr);
                    int myport=ntohs(tracker_addr.sin_port);
                    string sourceIp=ipPort[0];
                    int sourcePort=stoi(ipPort[1]);
                    int size=stoi(slicedArray[2]);
                   // cout<<"rec->"<<sourceIp<<" "<<sourcePort<<endl;                    
                    int new_sock1,new_sock2,new_sock3;
                    struct sockaddr_in peer_addr1,peer_addr2;

                    bzero((char *) &peer_addr1, sizeof(peer_addr1));          //first socket assigned and bound, used as thread client 
                    if ((new_sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
                    {    
                        printf("\n Socket creation error \n");  
                    }
                    peer_addr1.sin_family = AF_INET; 
                    peer_addr1.sin_port = htons(sourcePort);
                    if(inet_pton(AF_INET, "127.0.0.1", &peer_addr1.sin_addr)<=0)  
                    { 
                        printf("\nInvalid address/ Address not supported \n"); 
                    } 
                    if (connect(new_sock1, (struct sockaddr *)&peer_addr1, sizeof(peer_addr1)) < 0) 
                    {    
                        printf("\nConnection to seeder1 Failed \n");  
                    }



                    bzero((char *) &peer_addr2, sizeof(peer_addr2));            //second socket assigned and bound, used as thread server
                    if ((new_sock2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
                    {    
                        printf("\n Socket creation error \n");  
                    }
                    peer_addr2.sin_family = AF_INET; 
                    peer_addr2.sin_port = htons(6438);
                    int addrlen=sizeof(peer_addr2);
                    if(inet_pton(AF_INET, "127.0.0.1", &peer_addr2.sin_addr)<=0)  
                    { 
                        printf("\nInvalid address/ Address not supported \n"); 
                    } 
                    if (bind(new_sock2, (struct sockaddr *)&peer_addr2,sizeof(peer_addr2))<0) 
                    { 
                        perror("bind failed\n"); 
                        //exit(EXIT_FAILURE); 
                    }
                    string threadServer="127.0.0.1:6438";
                    string token2=threadServer+";"+slicedArray[1]+";"+slicedArray[2];
                    send(new_sock1,token2.c_str(),strlen(token2.c_str()),0);
                    if (listen(new_sock2, SOMAXCONN) < 0) 
                    {    
                        perror("listen"); 
                        exit(EXIT_FAILURE); 
                    }
                    //cout<<"crossed listening port\n";
                    if ((new_sock3 = accept(new_sock2, (struct sockaddr *)&peer_addr2,(socklen_t*)&addrlen))<0) 
                    { 
                            perror("accept"); 
                            exit(EXIT_FAILURE); 
                    }
                    //cout<<"made till here";
                    char mybuffer[32*1024];
                    memset(mybuffer,0,sizeof(mybuffer));
                    recv(new_sock3,mybuffer, 1024, 0);
                    if(input[0]=='1')
                    {
                        cout<<"ready for download\n";
                    }
                    float csize=32*1024.0;
                    int nchunks=ceil((float)size/csize);
                    cout<<"chunks are ->"<<nchunks<<endl;
                    ofstream outfile;
                    outfile.open(destPath, ios::out);
                    for(int i=1;i<=nchunks;i++)
                    {
                        //cout<<"recieving in chunk loop "<<i<<endl;
                        memset(mybuffer,0,sizeof(mybuffer));
                        recv(new_sock3,mybuffer,csize,0);
                        //cout<<mybuffer<<endl;
                        outfile<<mybuffer;
                    }

                    cout<<"done with downloading";
                    
                }
            }

        }
        if(userInput=="logout")
        {
            authLogin=0;                         //authorization taken off
            cout<<"Successfully logged out"<<endl;
        }
        if(userInput=="stop_share")
        {
            string groupId,fileName;
            cin>>groupId>>fileName;
            char input[1024];
            string token=userInput+";"+groupId+";"+fileName;
            send(sock,token.c_str(),strlen(token.c_str()),0);
            recv(sock,input, 1024, 0);
            cout<<input<<endl;
            if(input[0]=='1')
            {
                cout<<fileName<<"successfully removed from seeders list\n";
            }
            else
            {
                cout<<"failed to remove"<<fileName<<"from seeder's list\n";
            }
        }
        userInput.clear();
    }
}
int main (int argc,char ** argv) 
{
    string clientIPort=argv[1];
    string tracker_info_path=argv[2];
    vector<string>slicedArray=stringSlicer(argv[1],':');
    string ipaddress;
    int iport;
    ipaddress=slicedArray[0];
    iport=stoi(slicedArray[1]);
    clientRequest(clientIPort,tracker_info_path);
    int server_fd, new_socket, valread; 
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
    bzero((char *) &address, sizeof(address));
    address.sin_family = AF_INET; 
    address.sin_port = htons( iport );
    inet_pton(AF_INET,ipaddress.c_str(),&address.sin_addr); 
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    //cout<<"socket is"<<getsockname(server_fd, (struct sockaddr *)&address, (socklen_t *)sizeof(address));
    thread t1(inExecution,server_fd,iport);
    //t1.detach();
  
    if (listen(server_fd, SOMAXCONN) < 0) 
    {    
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }
    while(1)
    { 
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        }
        cout<<"accepted connection"<<endl;
        threadVector.push_back(thread(serveRequest,new_socket,iport));
    }
    vector<thread>:: iterator it;
    for(it=threadVector.begin();it!=threadVector.end();it++)
	{
		if(it->joinable()) 
			it->join();
	}
    return(0);
}