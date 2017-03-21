#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <vector>
using namespace std;

string PERIOD_FETCH="180";
string NUM_FETCH="1";
string NUM_PARSE="1";
string SEARCH_FILE="Search.txt";
string SITE_FILE="Site.txt";

void parseString(string);
vector<string> getTerms(string);


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

	vector<string> searchTerms=getTerms(SEARCH_FILE);
	if (searchTerms.size()==0){
		cout << "Error: invalid search file." << endl;
		return 1;
	}
	vector<string> siteTerms=getTerms(SITE_FILE);
	if (siteTerms.size()==0){
		cout << "Error: invalid site file." << endl;
		return 1;
	}
	return 0;
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

vector<string> getTerms(string filename){
	string word;
	vector<string> terms;

	ifstream file(filename.c_str());
	if(file.is_open()){
		while(getline(file, word)){
			if (word.find(",")==string::npos){
				terms.push_back(word);
			}
		}
	}
	return terms;
}
