all: site-tester

site-tester: site-tester.cpp
	g++ -Wall -std=c++11 -lcurl site-tester.cpp -o site-tester

clean:
	rm -f site-tester
