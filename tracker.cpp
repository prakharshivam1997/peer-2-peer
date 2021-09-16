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
#define PORT 5437
using namespace std;
vector<thread>threadVector;
char input[1024];
map<string,string>userReg;
map<string,set<string>>groupInfo;
map<string,string>leaderMap;
map<string,vector<string>>pendingRequest;
map<string,string>userToIp;
map<string,string>ipToUser;
map<pair<string,string>,set<string> >seederlist;
map<pair<string,string>,int>FilesizeMap;
vector<string> stringSlicer(string str,char del)
{
    stringstream ss(str);
    vector<string>a;
    string temp;
    while(getline(ss,temp,del))
    {
  	    //cout<<"hi"<<endl;
        a.push_back(temp);
    }
    return a;
}
string fileNameExtractor(string fileAddress)
{
    string fileName;
    for(auto itr=fileAddress.rbegin();itr!=fileAddress.rend();itr++)
    {
        if(*itr=='/')
        {
            break;
        }
        else
        {
            fileName=fileName+(*itr);
        }
    }
    reverse(fileName.begin(),fileName.end());
    return fileName;
}
void trackerkiller()
{
    string str;
    cin>>str;
    if(str=="quit")
    {
        cout<<"tracker is exiting\n";
        sleep(1);
        
        exit(0);
    }
}
void serveRequest(int new_sock,string ipaddress)
{
    int authLogin=0;
    memset(input,0,sizeof(input));
    int bytesReceived = recv(new_sock, input, 1024, 0);
    //cout<<input<<endl;
    vector<string>slicedArray=stringSlicer(input,';');
    if(slicedArray[0]=="create_user")
    {
        if(userReg.find(slicedArray[1])!=userReg.end())
        {
            char status[]="0";
            send(new_sock,status,strlen(status),0);   
        }
        else
        {
            userReg[slicedArray[1]]=slicedArray[2];
            //cout<<userReg[slicedArray[1]]<<" "<<endl;
            char status[]="1";
            send(new_sock,status,strlen(status),0);
        }

    }
    if(slicedArray[0]=="login")
    {
        if(userReg.find(slicedArray[1])!=userReg.end())
        {
            if(slicedArray[2]==userReg[slicedArray[1]])
            {
                authLogin=1;
                char status[]="1";
                userToIp[slicedArray[1]]=ipaddress+":"+slicedArray[3];//slicedArray[1];
                ipToUser[ipaddress+":"+slicedArray[3]]=slicedArray[1];
                send(new_sock,status,strlen(status),0);
            }
            else
            {
                char status[]="0";
                send(new_sock,status,strlen(status),0);
            }
        }
        else
        {
            char status[]="0";
            send(new_sock,status,strlen(status),0);
        }
        
    }
    if(slicedArray[0]=="create_group")
    {
        //cout<<"create_group group"<<endl;
        if(groupInfo.find(slicedArray[1])==groupInfo.end())
        {
            groupInfo[slicedArray[1]].insert("*****");                 //dummy insertion inside groupInfo map
            char status[]="1";
            //cout<<*(groupInfo[slicedArray[1]].begin())<<endl;
            send(new_sock,status,strlen(status),0);
        }
        else
        {
            char status[]="0";
            send(new_sock,status,strlen(status),0);
        }
        
    }
    if(slicedArray[0]=="join_group")
    {
        string groupId=slicedArray[1];
        //string IpPort=ipaddress+ ":" + slicedArray[2];
        //cout<<*(groupInfo[slicedArray[1]].begin())<<" "<<groupId;
        if(groupInfo.find(groupId)!=groupInfo.end())
        {
            //cout<<"entered"<<endl;
            string IpPort=ipaddress+ ":" + slicedArray[2];
            if((leaderMap.find(groupId)==leaderMap.end())||(leaderMap[groupId]==""))
            {
                leaderMap[groupId]=IpPort;
                groupInfo[groupId].insert(IpPort);
                //cout<<"leader group is ->"<<IpPort;
            }
            else
            {
                //cout<<"pushed to pending request"<<endl;
                pendingRequest[groupId].push_back(IpPort);
                //cout<<pendingRequest[groupId][0]<<" -> "<<IpPort<<endl;
            }
            char status[]="1";
            send(new_sock,status,strlen(status),0);
            
        }
        else
        {
            char status[]="0";
            send(new_sock,status,strlen(status),0);
        }
        
    }
    if(slicedArray[0]=="list_requests")
    {
        string request="";
        //cout<<"group Id given "<<slicedArray[1]<<" "<<endl;
        //cout<<pendingRequest[slicedArray[1]][0];
        for(auto itr=pendingRequest[slicedArray[1]].begin();itr!=pendingRequest[slicedArray[1]].end();itr++)
        {
            request+=ipToUser[*itr]+";";
        }
        //cout<<"request list "<<request<<endl;
        request[request.size()-1]='\0';
        if(request=="")
        {
            send(new_sock,"Nothing",strlen("Nothing"),0);
        }
        else
        {
            send(new_sock,request.c_str(),strlen(request.c_str()),0);
            //cout<<request;
        }
    }
    if(slicedArray[0]=="leave_group")
    {
        string Iport=ipaddress+":"+slicedArray[2];
        cout<<Iport<<endl;
        string groupId=slicedArray[1];
        bool flag=false;
        if(leaderMap[groupId]==Iport)
        {
            cout<<"1"<<endl;
            leaderMap.erase(groupId);
            flag=true;
            bool echecker=false;
            for(auto itr=groupInfo[groupId].begin();itr!=groupInfo[groupId].end();itr++)
            {
                if(*itr!="*****")
                {
                    echecker=true;
                    groupInfo[groupId].erase(itr);
                    leaderMap[groupId]=*itr;
                    break;
                }
            }
            
        }
        else
        {
            //cout<<"2"<<endl;
            for(auto itr=groupInfo[groupId].begin();itr!=groupInfo[groupId].end();itr++)
            {
                if(*itr==Iport)
                {
                    groupInfo[groupId].erase(itr);
                    flag=true;
                    break;
                }
            }
            if(flag==false)
            {
                for(auto itr=pendingRequest[slicedArray[1]].begin();itr!=pendingRequest[slicedArray[1]].end();itr++)
                {
                    if(*itr==Iport)
                    {
                        pendingRequest[groupId].erase(itr);
                        flag=true;
                        break;
                    }
                }
            }
        }
        if(flag==true)
        {
            char status[]="1";
            send(new_sock,status,strlen(status),0);
        }
        else
        {
            char status[]="0";
            send(new_sock,status,strlen(status),0);
        }
        
    }
    if(slicedArray[0]=="list_groups")
    {
        string group_list;
        for(auto itr=groupInfo.begin();itr!=groupInfo.end();itr++)
        {
            group_list+=itr->first+";";
        }
        group_list[group_list.size()-1]='\0';
        send(new_sock,group_list.c_str(),strlen(group_list.c_str()),0);
    }
    if(slicedArray[0]=="accept_request")
    {
        string groupId=slicedArray[1];
        string userId=slicedArray[2];
        string inPort=slicedArray[3];               //ipPort in this case
        string inputIpPort=ipaddress+":"+inPort;    //ip and port with which request was made
        bool flag=false;
        //cout<<slicedArray[1]<<" "<<slicedArray[2]<<endl;
        string str;
        //cout<<inputIpPort<<" "<<userId<<" "<<leaderMap[groupId]<<" "<<userToIp[userId]<<endl;
        if(leaderMap[groupId]!=inputIpPort)
        {
            char status[]="0";                              //only a leader can accept request
            send(new_sock,status,strlen(status),0);
        }
        else
        { 
            for(auto itr=pendingRequest[slicedArray[1]].begin();itr!=pendingRequest[slicedArray[1]].end();itr++)
            {
                //cout<<"pending request"<<*itr<<endl;
                if(*itr==userToIp[userId])
                {
                    //cout<<"entered"<<endl;
                    flag=true;
                    groupInfo[slicedArray[1]].insert(*itr);
                    //cout<<"entered 2"<<endl;
                    pendingRequest[slicedArray[1]].erase(itr);
                    break;
                }
            }
            //cout<<"gone till here"<<endl;
            if(flag==true)
            {
                char status[]="1";
                send(new_sock,status,strlen(status),0);
            }
            else
            {
                char status[]="0";
                send(new_sock,status,strlen(status),0);
            }
        }
    }
    if(slicedArray[0]=="upload_file")
    {
        //sliceArray[0]="upload_file"  slicedArray[1]=filePath  slicedArray[2]=groupId  slicedArray[3]=port  slicedArray[4]=filesize
        //cout<<slicedArray[0]<<" "<<slicedArray[1]<<" "<<slicedArray[2]<<" "<<slicedArray[3]<<" "<<slicedArray[4]<<endl; 
        int size=stoi(slicedArray[4]);
        string ipPort=ipaddress+":"+slicedArray[3];
        pair<string,string> gr_ip=make_pair(slicedArray[2],ipPort);
        seederlist[gr_ip].insert(slicedArray[1]);
        FilesizeMap[make_pair(slicedArray[1],ipPort)]=size;
        char status[]="1";
        send(new_sock,status,strlen(status),0);
    }
    if(slicedArray[0]=="list_files")
    {
        string token;
        string groupId=slicedArray[1];
        //cout<<*(seederlist.begin()->second.begin());
        bool flag=false;
        for(auto itr=seederlist.begin();itr!=seederlist.end();itr++)
        {
            //cout<<"inside loop\n";
            if(itr->first.first==groupId)
            {
                for(auto qtr=itr->second.begin();qtr!=itr->second.end();qtr++)
                {
                    token+=fileNameExtractor(*qtr)+";";
                    flag=true;
                }
            }
        }
        if(flag==true)
        {
            send(new_sock,token.c_str(),strlen(token.c_str()),0);
        }
        else
        {
            char status[]="0";
            send(new_sock,status,strlen(status),0);
        }
        
    }
    if(slicedArray[0]=="download_file")
    {
        //slicedArray[0]="download_file" slicedArray[1]=groupId  slicedArray[2]=fileName (without address)  slicedArray[3]=port
        string groupId,fileName,destPath;
        groupId=slicedArray[1];
        fileName=slicedArray[2];
        string IpPort=ipaddress+":"+slicedArray[3];
        string fileAddress;
        string sourceIpPort;
        string token;
        for(auto itr=seederlist.begin();itr!=seederlist.end();itr++)
        {
            if(itr->first.first==groupId)
            {
                for(auto qtr=itr->second.begin();qtr!=itr->second.end();qtr++)
                {
                    if(fileNameExtractor(*qtr)==fileName)
                    {
                        fileAddress=*qtr;
                        sourceIpPort=itr->first.second;
                        break;
                    }
                }
            }
        }
        int size=-1;
        for(auto itr=FilesizeMap.begin();itr!=FilesizeMap.end();itr++)
        {
            if(itr->first.first==fileAddress)
            {
                size=(itr->second);
            }
        }
        //fileAddress[fileAddress.size()-1]='\0';
        token=sourceIpPort+";"+fileAddress+";"+to_string(size);
        send(new_sock,token.c_str(),strlen(token.c_str()),0);
    }
    if(slicedArray[0]=="stop_share")
    {
        string groupId=slicedArray[1];
        string fileName=slicedArray[2];
        bool flag=false;
        //cout<<"starting execution\n";
        for(auto itr=seederlist.begin();itr!=seederlist.end();itr++)
        {
            //cout<<"I am here\n";
            if(itr->first.first==groupId)
            {
                //cout<<"entered in inner loop\n";
                for(auto qtr=itr->second.begin();qtr!=itr->second.end();qtr++)
                {
                    //cout<<"third in inner loop "<<*qtr<<" "<<fileName<<fileNameExtractor(*qtr)<<endl;
                    if(fileName==fileNameExtractor(*qtr))
                    {
                        itr->second.erase(qtr);
                        flag=true;
                    }
                    if(qtr->empty()==true)
                    {
                        break;
                    }
                }
            }
        }
        if(flag==false)
        {
            char status[]="0";
            send(new_sock,status,strlen(status),0);
        }
        else
        {
            char status[]="1";
            send(new_sock,status,strlen(status),0);
        }
        
    }
   

}
int main () 
{
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
    address.sin_port = htons( PORT );
    inet_pton(AF_INET,"127.0.0.1",&address.sin_addr); 
    if(bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    if (listen(server_fd, SOMAXCONN) < 0) 
    {    
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }
    thread t1(trackerkiller);
    //t1.detach(); 
    while(1)
    { 

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        }
        string ipaddress=inet_ntoa(address.sin_addr);
        int iport=ntohs(address.sin_port);
        //cout<<"port is->"<<iport<<endl;
        //cout<<"accepted connection"<<endl;
        //thread t1 (serveRequest,new_socket,ipaddress,iport);
        //t1.join();
        threadVector.push_back(thread (serveRequest,new_socket,ipaddress));
        //threadVector[threadVector.size()-1].detach();
        //cout<<"goene here"<<endl;
        //cout<<"completed execution"<<endl;
        //close(new_socket);

    }
    vector<thread>:: iterator it;
    for(it=threadVector.begin();it!=threadVector.end();it++)
	{
		if(it->joinable()) 
			it->join();
	}
    return(0);
}