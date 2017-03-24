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

void getResults(vector<string> searchWords, vector<string> siteTerms){
	int counter=0;
	CURL *curl;
	string readBuffer;
	CURLcode res;

	for (unsigned int k=0; k<siteTerms.size(); k++){	
		readBuffer.clear();
		curl = curl_easy_init();
		if(curl) {
			curl_easy_setopt(curl, CURLOPT_URL, siteTerms[k].c_str());
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

				myfile << date << "," << searchWords[j] << "," << siteTerms[k] << "," << counter << endl;
				counter=0;
			}
		}
	}
	myfile.close();
}
