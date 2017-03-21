#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

vector<string> convertToVector(string);

int main(int argc, char* argv[]){
	        string outputFile; //taken in to convert to html
		string htmlFile; //output html file 
		string header="    <th>";
		string headerEnd="</th>";

		if (argc==3){
			outputFile=argv[1]; //first argument, output file from project
			htmlFile=argv[2]; //second argument, html file
		}
		else {
			cout << "Error: no file specified." << endl;
			return 1;		        
		}
		vector<string> vect;
		string line;
		string head="<table style=\"width:100%\">";
		ifstream file(outputFile.c_str());
		ofstream y;
		y.open(htmlFile.c_str());
		
		y << head << endl;
		
		//Read the comma separated file, then output in html format
		//Use "<th>" for the first header line to bold the top entry of the table 
		if (file.is_open()){ //check that we can open the file
			while (getline(file,line)){
				y << "  <tr>" << endl;
				vect = convertToVector(line);
			
				for (int i=0; i<4; i++){
					y << header;
					y << vect.at(i);
					y << headerEnd << endl;
				}	
				y << "</tr>" << endl;
				header="    <td>";
				headerEnd="</td>";
			}
		}
		y.close();
		return 0;
}

vector<string> convertToVector(string str){
	istringstream ss(str);
	string token;
	vector<string> tokens;

	while(getline(ss, token, ',')) {
		tokens.push_back(token);
	}
	return tokens;
}
