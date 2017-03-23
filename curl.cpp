#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ctime>
#include <chrono>
#include <fstream>
#include <cstring>
#include <vector>

using namespace std;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
	((string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void print(vector<string> searchWords, vector<string> siteTerms){
	string searchTerm="Notre";
	int counter=0;
	CURL *curl;
	string readBuffer;
	curl = curl_easy_init();
	string website="https://www.nd.edu/faith-and-service/faith-in-the-academy/"; 
	ofstream myfile;
	myfile.open("example.csv");
	myfile << "Time" << "," << "Phrase" << "," << "Website" << "," << "Count" << endl;

	for (unsigned int k=0; k<siteTerms.size(); k++){	
		for(unsigned int j=0; j<searchWords.size(); j++){
			if(curl) {
				curl_easy_setopt(curl, CURLOPT_URL, siteTerms[k].c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
				res = curl_easy_perform(curl);
				curl_easy_cleanup(curl);
				size_t nPos = readBuffer.find(searchTerm, 0); // first occurrence
				while(nPos != string::npos){
					counter++;
					nPos = readBuffer.find(searchWords[j], nPos+1);
				}
		
				char *date;
				time_t timer;
				timer=time(NULL);	
				date = asctime(localtime(&timer));
				date[strlen(date) - 1] = '\0';	

				myfile << date << "," << searchWords[j] << "," << siteTerms[k] << "," << counter << endl;
				myfile.close();
			}
		}		
	}
}
