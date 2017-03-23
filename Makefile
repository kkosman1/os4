all: site-tester

site-tester: site-tester.cpp curl.cpp
	g++ -Wall -std=c++11 -lcurl curl.cpp site-tester.cpp -o site-tester

clean:
	rm -f site-tester
