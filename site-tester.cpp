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

string PERIOD_FETCH="180";
string NUM_FETCH="1";
string NUM_PARSE="1";
string SEARCH_FILE="Search.txt";
string SITE_FILE="Site.txt";
pthread_cond_t fetch = PTHREAD_COND_INITIALIZER; 
pthread_cond_t parse = PTHREAD_COND_INITIALIZER;
pthread_mutex_t fetchMutex = PTHREAD_MUTEX_INITIALIZER;

void parseString(string);
vector<string> getSearchTerms(string);
vector<string> getSiteTerms(string);

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

	vector<string> searchTerms=getSearchTerms(SEARCH_FILE);
	if (searchTerms.size()==0){
		cout << "Error: invalid search file." << endl;
		return 1;
	}
	vector<string> siteTerms=getSiteTerms(SITE_FILE);
	if (siteTerms.size()==0){
		cout << "Error: invalid site file." << endl;
		return 1;
	}
	pthread_t fetchThreads[NUM_FETCH];
	pthread_t parseThreads[NUM_PARSE];
	initializeFile(); //intiailize columns in file	
	//Continuously run the program until control-C
	while(1){
		signal(SIGALRM, &startThreads, NULL); 
		alarm(PERIOD_FETCH); 
	}
	return 0;
}

void startThreads(vector<string> siteTerms, vector<string> searchTerms){
	allowFetch();
	availableThread=NUM_THREAD; 	
	
	for (int i=0; i< siteTerms.size(); i++){
		if (availalbeThread>0){
			pthread_create(&fetch[i], NULL, getResults(searchTerms, siteTerms[i]));
			availableThread--;
		}
	}
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
                
void initializeFile(){
	ofstream myfile;
	myfile.open("example.csv");
	myfile << "Time" << "," << "Phrase" << "," << "Website" << "," << "Count" << endl;
}

void allowFetch(){
	pthread_mutex_lock(&fetchMutex);
	while(!fetch){
		pthread_cond_wait(&fetch, &fetchMutex);
	}
	pthread_mutex_unlock(&fetchMutex);
}


void getResults(vector<string> searchWords, string siteTerms){
	int counter=0;
	CURL *curl;
	string readBuffer;
	CURLcode res;

	readBuffer.clear();
	curl = curl_easy_init();
	if(curl) {	
		curl_easy_setopt(curl, CURLOPT_URL, siteTerms.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		res=curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		for (unsigned int j=0; j<searchWords.size(); j++){
			size_t nPos = readBuffer.find(searchWords[j], 0); // first occurrence
			while(nPos != string::npos){
				counter++;
				nPos = readBuffer.find(searchWords[j], nPos+1);
			}
			char *date;
			time_t timer;
			timer=time(NULL);
			date = asctime(localtime(&timer));
			date[strlen(date) - 1] = '\0';
			myfile << date << "," << searchWords[j] << "," << siteTerms << "," << counter << endl;
			counter=0;
		}
	}
}

void parseString(string line){
	string delimiter="=";

	size_t pos=line.find(delimiter);
	string token = line.substr(0,pos);
	line.erase(0,pos+1);
	if (strcmp(token.c_str(), "PERIOD_FETCH")==0)
		PERIOD_FETCH=line;
	else if (strcmp(token.c_str(), "NUM_FETCH")==0)
		NUM_FETCH=line;
	else if (strcmp(token.c_str(), "NUM_PARSE")==0)
		NUM_PARSE=line;
	else if (strcmp(token.c_str(), "SEARCH_FILE")==0)
		SEARCH_FILE=line;
	else if (strcmp(token.c_str(), "SITE_FILE")==0)
		SITE_FILE=line;
	else
		cout << "Warning: unknown parameter." << endl;
}

vector<string> getSearchTerms(string filename){
	string word;
	vector<string> terms;

	ifstream file(filename.c_str());
	if(file.is_open()){
		while(getline(file, word)){
			if (word.find(",")==string::npos && !word.empty()){
				terms.push_back(word);
			}
		}
	}
	return terms;
}

vector<string> getSiteTerms(string filename){
	string word;
	vector<string> terms;
	string substring; //look at the first 7 letters to see if they are http
                
	ifstream file(filename.c_str());
	if(file.is_open()){
		while(getline(file, word)){
			string hello = word.substr(0, 7).c_str();
			if (word.length() > 7 && strcmp(word.substr(0, 7).c_str(), "http://")==0){
				terms.push_back(word);
			}
		}
	}
	return terms;
}
