all: site-tester

site-tester: site-tester.cpp
	g++ -Wall site-tester.cpp -o site-tester

clean:
	rm -f site-tester
