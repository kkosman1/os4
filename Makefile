all: site-tester curl

site-tester: site-tester.cpp
	g++ -Wall site-tester.cpp -o site-tester

curl: curl.cpp
	g++ -Wall -std=c++11 -lcurl curl.cpp

clean:
	rm -f site-tester
