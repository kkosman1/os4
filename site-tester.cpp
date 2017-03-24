#include <signal.h> 
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ctime>
#include <chrono>
#include <fstream>
#include <cstring>
#include <vector>
#include <queue> 
#include <pthread.h>
#include <unistd.h>

using namespace std;

// Global Variables
int cont=1;
int PERIOD_FETCH=180;
int NUM_FETCH=1;
int NUM_PARSE=1;
string SEARCH_FILE="Search.txt";
string SITE_FILE="Site.txt";
int BATCH=0;
fstream MYFILE;
queue<string> SITES;
vector<string> SEARCHWORDS;
queue<string> FETCH;
queue<struct parseStruct> PARSE;
pthread_cond_t fetchCond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t parseCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t fetchMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t parseMutex = PTHREAD_MUTEX_INITIALIZER;
vector<pthread_t> fetchThreads;
vector<pthread_t> parseThreads;

void interruptHandler(int);
void fillFetchQueue(int);
void initializeFile();
static size_t WriteCallback(void *, size_t, size_t, void *);
void* getSiteData(void *);
void* getSearchData(void *);
void parseString(string);
void getSearchTerms(string);
void getSiteTerms(string);

struct parseStruct{
	string site;
	string siteData;
	string searchWord;
	char* date;
};

int main(int argc, char* argv[]){
	string configFile;
	if (argc==2){
		configFile=argv[1];
	}
	else {
		cout << "Error: no configuration file specified." << endl;
		return 1;
	}
	string line;
	ifstream file(configFile.c_str());
	if (file.is_open()){ //check that we can open the file
		while (getline(file,line)){
			parseString(line);
		}
		file.close();
	}
	else {
		cout << "Error: could not open configuration file." << endl;
		return 1;
	}

	getSearchTerms(SEARCH_FILE);
	if (SEARCHWORDS.empty()){
		cout << "Error: invalid search file." << endl;
		return 1;
	}
	getSiteTerms(SITE_FILE);
	if (SITES.empty()){
		cout << "Error: invalid site file." << endl;
		return 1;
	}
	
	pthread_t temp;

	//Create Threads
	for (int i=0;i<NUM_FETCH; i++){
		fetchThreads.push_back(temp);
		pthread_create(&fetchThreads[i], NULL, &getSiteData, NULL);
	}
	for (int i=0;i<NUM_PARSE; i++){
		parseThreads.push_back(temp);
		pthread_create(&parseThreads[i], NULL, &getSearchData, NULL);
	}
	
	//Continuously run the program until control-C
	signal(SIGALRM, fillFetchQueue);
	signal(SIGHUP, interruptHandler);
	while(cont){
		alarm(PERIOD_FETCH);

        	pthread_mutex_lock(&fetchMutex);
        	pthread_cond_broadcast(&fetchCond);
		pthread_mutex_unlock(&fetchMutex);

        	pthread_mutex_lock(&parseMutex);
        	pthread_cond_broadcast(&parseCond);
             	pthread_mutex_unlock(&fetchMutex);	
	}
	return 0;
}

void interruptHandler(int sig){
	for (int i=0;i<NUM_FETCH; i++)
		pthread_join(fetchThreads[i], NULL);
	for (int i=0;i<NUM_PARSE; i++)
		pthread_join(parseThreads[i], NULL);
	
	cont=0;
}

void fillFetchQueue(int sig){
	BATCH++;
	FETCH=SITES;
	initializeFile();
}

//Convert to Cstring
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
                
void initializeFile(){
	string filename = to_string(BATCH) + ".csv";
	MYFILE.open(filename, fstream::out | fstream::app);
	MYFILE << "Time" << "," << "Phrase" << "," << "Website" << "," << "Count" << endl;
}

void* getSiteData(void *){
	while(FETCH.empty()){
		pthread_cond_wait(&fetchCond, &fetchMutex);
		cout << "hello yo" << endl;
	}
	cout <<"hi"<<endl;
	string site=FETCH.front();
	FETCH.pop();
	struct parseStruct args;

        time_t timer;
        timer=time(NULL);
        args.date = asctime(localtime(&timer));
        args.date[strlen(args.date) - 1] = '\0';

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl) {	
		curl_easy_setopt(curl, CURLOPT_URL, site.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(args.siteData));

		res=curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		curl_easy_cleanup(curl);
	}
	args.site=site;
	cout << "hello" << endl;
	//notifiy parsers when fetch is done, know they can proceed
	for (unsigned int i=0;i<SEARCHWORDS.size();i++){
		args.searchWord=SEARCHWORDS[i];
		PARSE.push(args);
	}
	return NULL;
}

void* getSearchData(void *){
	while(PARSE.empty()){
		pthread_cond_wait(&parseCond, &parseMutex);
	}
	struct parseStruct args=PARSE.front();
	PARSE.pop();
	int counter=0;

	size_t nPos = args.siteData.find(args.searchWord, 0); // first occurrence
	while(nPos != string::npos){
		counter++;
		nPos = args.siteData.find(args.searchWord, nPos+1);
	}
	cout << "hello" << args.searchWord << endl;
	MYFILE << args.date << "," << args.searchWord << "," << args.site << "," << counter << endl;
	return NULL;
}

void parseString(string line){
	string delimiter="=";

	size_t pos=line.find(delimiter);
	string token = line.substr(0,pos);
	line.erase(0,pos+1);
	if (strcmp(token.c_str(), "PERIOD_FETCH")==0)
		PERIOD_FETCH=stoi(line);
	else if (strcmp(token.c_str(), "NUM_FETCH")==0 && line.length()==1 && ((int)line[0]-'0')>=1 && ((int)line[0]-'0')<=8)
		NUM_FETCH=((int)line[0]-'0');
	else if (strcmp(token.c_str(), "NUM_PARSE")==0 && line.length()==1 && ((int)line[0]-'0')>=1 && ((int)line[0]-'0')<=8)
		NUM_PARSE=((int)line[0]-'0');
	else if (strcmp(token.c_str(), "SEARCH_FILE")==0)
		SEARCH_FILE=line;
	else if (strcmp(token.c_str(), "SITE_FILE")==0)
		SITE_FILE=line;
	else
		cout << "Warning: unknown parameter." << endl;
}

void getSearchTerms(string filename){
	string word;

	ifstream file(filename.c_str());
	if(file.is_open()){
		while(getline(file, word)){
			if (word.find(",")==string::npos && !word.empty()){
				SEARCHWORDS.push_back(word);
			}
		}
	}
}

void getSiteTerms(string filename){
	string word;
	string substring; //look at the first 7 letters to see if they are http
                
	ifstream file(filename.c_str());
	if(file.is_open()){
		while(getline(file, word)){
			string hello = word.substr(0, 7).c_str();
			if (word.length() > 7 && strcmp(word.substr(0, 7).c_str(), "http://")==0){
				SITES.push(word);
			}
		}
	}
}
