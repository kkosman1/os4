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

using namespace std;

int PERIOD_FETCH=180;
int NUM_FETCH=1;
int NUM_PARSE=1;
string SEARCH_FILE="Search.txt";
string SITE_FILE="Site.txt";
int BATCH=1;
queue<string> SITES;
vector<string> SEARCHWORDS;
queue<string> FETCH;
queue<struct parseStruct> PARSE;
pthread_cond_t fetchCond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t parseCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t fetchMutex = PTHREAD_MUTEX_INITIALIZER;

void parseString(string);
void getSearchTerms(string);
void getSiteTerms(string);

struct parseStruct{
	string site;
	string siteData;
	string searchWord;
	string date;
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
	initializeFile(); //intiailize columns in file	
	//Continuously run the program until control-C
	signal(SIGALRM, &startThreads, NULL);
	while(1){
		alarm(PERIOD_FETCH);
		BATCH++;
	}
	return 0;
}

void startThreads(){
	FETCH=SITES; //reset queue
	initializeFile();

	pthread_mutex_lock(&fetchMutex);
        while(!fetch){
                pthread_cond_wait(&fetch, &fetchMutex);
        }
        pthread_mutex_unlock(&fetchMutex);







	allowFetch();
	siteCounter=0;
	pthread_t fetchThreads[NUM_FETCH];
	pthread_t parseThreads[NUM_PARSE];

	struct siteArgument args[siteTerms.size()];
	while(!sites.empty()){
		for (int i=0; i<NUM_FETCH; i++){
			args[siteCounter].site=sites.pop_front();
			pthread_create(&fetchThreads[i], NULL, getSiteData, (void *));
			siteCounter++;
		}
		for (int i=0; i<NUM_FETCH; i++){
			pthread_join(fetchThreads[i], NULL);
		}
	}
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
                
void initializeFile(){
	ofstream myfile;
	myfile.open(BATCH+".csv");
	myfile << "Time" << "," << "Phrase" << "," << "Website" << "," << "Count" << endl;
	myfile.close();
}

void getSiteData(void *){
	string site=FETCH.pop_front();
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

	for (i=0;i<SEARCHWORDS.size();i++){
		args.searchWord=SEARCHWORDS[i];
		PARSE.append(args);
	}
}

void getSearchData(){
	struct parseStruct args=PARSE.pop_front();
	int counter=0;

	size_t nPos = args.siteData.find(args.searchWord, 0); // first occurrence
	while(nPos != string::npos){
		counter++;
		nPos = args.siteData.find(args.searchWord, nPos+1);
	}
	myfile << args.date << "," << args.searchWord << "," << args.site << "," << counter << endl;
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

vector<string> getSearchTerms(string filename){
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
				SITES.append(word);
			}
		}
	}
}
