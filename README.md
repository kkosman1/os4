Partner: Kim Kosman

To build the code, use make.
To run, use: site-tester CONFIG.TXT
The configuration file, CONFIG.TXT, should contain the parameters specified below (unless the defaults are desired):
	Parameter	Description							Default
	PERIOD_FETCH	The time (in seconds) between fetches of the various sites	180
	NUM_FETCH	Number of fetch threads (1-8)					1
	NUM_PARSE	Number of parsing threads (1-8)					1
	SEARCH_FILE	File containing the search strings				Search.txt
	SITE_FILE	File containing the sites to query				Sites.txt
