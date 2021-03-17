//Yağmur SELEK
//Id:2017400273
//cmpe322 project2
//PS: except input1 all input gives desired outputs in macos 
//However in Ubuntu sometimes it gives the lines switched for processes or a teller took wrong client
//but if I run the code second or third time  it gives desired output
//Unfortunately I couldn't solve this problem
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <vector>
#include <istream>
#include <iostream>
#include <sstream>
#include <thread>
#include <queue>
#include <chrono>
//global Variables are defined here
int numb_clients; //holds the total client number
std::vector<std::vector<int> > clients; //holds the given inputs for clients: client id,arrivalTime,reqSeat,Service time
std::vector<bool> client_taken; //holds for each client if it is served or not
std::queue<int> clients_q; // WAITING QUEUE FOR CLIENTS
int currentTime; 
int capacity; //Capacity of a given theater
bool A; //false if teller thread A is not busy
bool B; //false if teller thread B is not busy
bool C; //false if teller thread C is not busy
int currentTicketSaled; 
std::vector<bool> seats; //holds if seat is available(0) or taken(1)
std::vector<std::string> clientNames; //holds client names (index=client id)
std::ofstream outputFile;
int lastSeatReserved; //it will be used to not exceed total seat Number
int minSeatAvailable; //it keeps track of minimum seat available at moment
int capacity_oda=60; //Seat capacity for Oda ODA TIYATROSU
int capacity_uskudar=80; //Seat capacity for USKUDAR STUDYO SAHNE
int capacity_kucuk=200; //Seat capacity for KUCUK SAHNE
bool clientThreadsCompleted;

pthread_mutex_t client_mutex;  //mutex for client threads
pthread_mutex_t teller_mutex;  //mutex for teller threads

using namespace std;

//client thread takes the client id as threadarg,
//sleeps for its arrival time
//then push the client in the clients_q
void *func_clientThread(void *threadarg) {
    int a=*(int*)threadarg;
    int arrivalTime=clients[a][1];
    this_thread::sleep_for(chrono::milliseconds (arrivalTime));
    pthread_mutex_lock(&client_mutex);
    clients_q.push(a);
    pthread_mutex_unlock(&client_mutex);
    pthread_exit(0);
}

//I couldn't achive the desired properties just with one section 
//So I divided the tellerThread into three section and gave teller A the highest priority and C is the lowest
//tellerThread takes A/B/C as threadarg
void *func_tellerThread(void *threadarg) {
    string a=*(string*)threadarg;
    //wait until clients_q is not empty
    while(clients_q.empty()) {

    }
    //following for loop iterates until all clients are served
    for(;currentTicketSaled<numb_clients;) {
        //if The thread is Thread A and Thread A is not busy 
        if(a=="A" && A&& !clients_q.empty()) {
            int currentClient=clients_q.front();
            clients_q.pop();
            currentTicketSaled++;
            A=false;
            int givenSeatA;
            
            //following code snippet decides which seat will be given to client

            int reqSeatA=clients[currentClient][3];
            if(reqSeatA>capacity+1 || seats[reqSeatA-1]) {
                givenSeatA=minSeatAvailable+1;
                seats[givenSeatA-1]=true;
                while(seats[minSeatAvailable] && minSeatAvailable<capacity) {
                    minSeatAvailable++;
                }
            } else {
                givenSeatA=reqSeatA;
                seats[givenSeatA-1]=true;
                while(seats[minSeatAvailable] && minSeatAvailable<capacity) {
                    minSeatAvailable++;
                }
            }

            //after deciding the given seat thread A sleeps for currentClient's process time
            int currentProcessTime=clients[currentClient][2];
            if(givenSeatA==capacity+1) {
                lastSeatReserved++;
            }
            this_thread::sleep_for(chrono::milliseconds (currentProcessTime));

            pthread_mutex_lock(&teller_mutex);
            
            //if all seats are reserved
            if(lastSeatReserved>=1) {
                givenSeatA=-1;
                outputFile<<clientNames[currentClient] <<" requests seat " << reqSeatA <<", reserves None" <<". Signed by Teller A."<<endl;
            }
            else {
                outputFile << clientNames[currentClient] << " requests seat " << reqSeatA << ", reserves seat "
                           << givenSeatA << ". Signed by Teller A." << endl;
            }
            // cout<< clientNames[currentClient] <<"requested" << reqSeatA <<" given by A" << givenSeatA <<endl;
            A=true;

            pthread_mutex_unlock(&teller_mutex);

        }

        //if thread A is busy and B is available and this thread is teller B
        else if(a=="B" && B && !A&& !clients_q.empty()) {

            int currentClient=clients_q.front();
            clients_q.pop();
            currentTicketSaled++;
            B=false;
            int givenSeatB;

            //following code snippet decides which seat will be given to client
            int reqSeatB=clients[currentClient][3];
            int currentProcessTime=clients[currentClient][2];
            if(reqSeatB>capacity+1 || seats[reqSeatB-1]) {
                givenSeatB=minSeatAvailable+1;
                seats[givenSeatB-1]=true;
                while(seats[minSeatAvailable] && minSeatAvailable<capacity) {
                    minSeatAvailable++;
                }
            } else {
                givenSeatB=reqSeatB;
                seats[givenSeatB-1]=true;
                while(seats[minSeatAvailable] && minSeatAvailable<capacity) {
                    minSeatAvailable++;
                }
            }
            if(capacity+1==givenSeatB) {
                lastSeatReserved++;
            }

            //teller B sleeps for current client's process time
            this_thread::sleep_for(chrono::milliseconds (currentProcessTime));

            pthread_mutex_lock(&teller_mutex);

            //if all seats are already reserved
            if(lastSeatReserved>=1) {
                givenSeatB=-1;
                outputFile<<clientNames[currentClient] <<" requests seat " << reqSeatB <<", reserves None" <<". Signed by Teller B."<<endl;
            }
            else {
                outputFile << clientNames[currentClient] << " requests seat " << reqSeatB << ", reserves seat "
                           << givenSeatB << ". Signed by Teller B." << endl;
            }
           

            B=true;
            pthread_mutex_unlock(&teller_mutex);

        }

        //if A and B are busy and C is available and it is teller thread for C 
        else if(a=="C" && C && !A && !B && !clients_q.empty()){
            int currentClient=clients_q.front();
            clients_q.pop();
            currentTicketSaled++;
            C=false;
            int givenSeatC;
            int reqSeatC=clients[currentClient][3];
            int currentProcessTime=clients[currentClient][2];

            //following code snippet decides which seat will be given to client
            if(reqSeatC>capacity+1 || seats[reqSeatC-1]) {
                givenSeatC=minSeatAvailable+1;
                seats[givenSeatC-1]=true;
                while(seats[minSeatAvailable] && minSeatAvailable<capacity) {
                    minSeatAvailable++;
                }
            } else {
                givenSeatC=reqSeatC;
                seats[givenSeatC-1]=true;
                while(seats[minSeatAvailable] && minSeatAvailable<capacity) {
                    minSeatAvailable++;
                }
            }
            if(capacity+1==givenSeatC) {
                lastSeatReserved++;
            }

            //teller C thread sleeps for current client's service time
            this_thread::sleep_for(chrono::milliseconds (currentProcessTime));

            pthread_mutex_lock(&teller_mutex);

            //if all seats are already reserved
            if(lastSeatReserved>=1) {
                givenSeatC=-1;
                outputFile<<clientNames[currentClient] <<" requests seat " << reqSeatC <<", reserves None" <<". Signed by Teller C."<<endl;
            }
            else {

                outputFile<<clientNames[currentClient] <<" requests seat " << reqSeatC <<", reserves seat " << givenSeatC <<". Signed by Teller C."<<endl; }
            
            C=true;
            pthread_mutex_unlock(&teller_mutex);

        }

    }

    //exit thread
    pthread_exit(0);


}



int main(int argc, char* argv[]) {
    //set global vars to initial values
    lastSeatReserved=0;
    minSeatAvailable=0;
    A=true;
    B=true;
    C=true;
    currentTicketSaled=0;
    currentTime=-1;
    numb_clients=-1;
    clientThreadsCompleted=false;
    
    pthread_mutex_init(&teller_mutex,NULL); //initialize mutex for teller threads
    pthread_mutex_init(&client_mutex,NULL); //initialize mutex for client threads

    outputFile.open(argv[2]);
    std::ifstream inputFile(argv[1]);
    std::string input_str;

    //take first line and decides the capacity of given theater
    getline(inputFile,input_str);
    if(input_str=="OdaTiyatrosu")
        capacity=capacity_oda;
    else if(input_str=="ÜsküdarStüdyoSahne")
        capacity=capacity_uskudar;
    else
        capacity=capacity_kucuk;

    for(int i=0;i<capacity; i++) {
        seats.push_back(false);
    }

    //takes second line and set the number of clients given to given value
    getline(inputFile,input_str);
    numb_clients=std::stoi(input_str);

    //takes remaining input lines for numb_clients time and 
    //add client informations given into clients and client_names vector
    for(int i=1;i<=numb_clients;i++) {
        string current_clientName;
        std::vector<int> tmpVec;
        int tmpCount=0;
        getline(inputFile,input_str );
        std::istringstream iss(input_str);
        std::string token;
        while(std::getline(iss, token, ',')) {
            if(tmpCount==0) {
                current_clientName=token;
                tmpVec.push_back(i);
                tmpCount++;
            } else {
                tmpVec.push_back(std::stoi(token));
            }
        }
        clientNames.push_back(current_clientName);
        clients.push_back(tmpVec);
    }
    //end of taking inputs

    outputFile<<"Welcome to the Sync-Ticket!" <<std::endl;

    //3 threads for tellers
    pthread_t tellerThreads[3];
    std::vector<std::string> tellerNames;
    tellerNames.push_back("A");
    tellerNames.push_back("B");
    tellerNames.push_back("C");

    //creates teller threads
    for(int i=0;i<3;i++) {
        pthread_create( &tellerThreads[i], NULL, &func_tellerThread,(void*) &tellerNames[i]);
        outputFile<< "Teller "<< char(i+65)<<" has arrived." <<std::endl;
    }

    //a client_thread for each client
    pthread_t client_threads[numb_clients];

    vector<int> index(numb_clients,0);
    for(int i=0;i<numb_clients;i++) {
        client_taken.push_back(false);
    }


    //creates client threads, takes client ids as input
    for(int i=0;i<numb_clients;i++) {
        index[i]=i;

        pthread_create( &client_threads[i], NULL, &func_clientThread,(void*)&index[i]);
        if(i==numb_clients-1)
            clientThreadsCompleted=true;
    }





    //join all the threads

    for(int i=0; i<3; i++) {
        pthread_join( tellerThreads[i], NULL);
    }
    for(int i=0; i<numb_clients; i++) {
        pthread_join( client_threads[i], NULL);
    }

    //Destroy mutexes 
    pthread_mutex_destroy(&teller_mutex);
    pthread_mutex_destroy(&client_mutex);
    outputFile<<"All clients received service."<<endl;
    //close files
    inputFile.close();
    outputFile.close();



    return 0;
}





