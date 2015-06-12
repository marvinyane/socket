/*

sample for O'ReillyNet article on libcURL:
	{TITLE}
	{URL}
	AUTHOR: Ethan McCallum

Scenario: use http/GET to fetch a webpage


This code was built/tested under Fedora Core 3,
libcURL version 7.12.3.  As libcURL is under 
active development, features may be added or
removed between releases.  Be sure to check your
library version, and its corresponding documentation,
if these examples fail to build on your system.

*/

#include<iostream>

extern "C" {
	#include<curl/curl.h>
}

// - - - - - - - - - - - - - - - - - - - -

enum {
	ERROR_ARGS = 1 ,
	ERROR_CURL_INIT = 2
} ;

enum {
	OPTION_FALSE = 0 ,
	OPTION_TRUE = 1
} ;

enum {
	FLAG_DEFAULT = 0 
} ;

// - - - - - - - - - - - - - - - - - - - -


int main( const int argc , const char** argv ){

	if( argc != 2 ){
		std::cerr << "test of libcURL: fetch data via HTTP." << std::endl ;
		std::cerr << "This sends header data to stderr," << std::endl ;
		std::cerr << "and body data to stdout." << std::endl ;
		std::cerr << " Usage: " << argv[0] << " {url} [debug]" << std::endl ;
		return( ERROR_ARGS ) ;
	}

	const char* url = argv[1] ;

	// global libcURL init
	curl_global_init( CURL_GLOBAL_ALL ) ;

	// create a context, sometimes known as a handle.
	// Think of it as a lookup table, or a source of config data.
	CURL* ctx = curl_easy_init() ;

	if( NULL == ctx ){
		std::cerr << "Unable to initialize cURL interface" << std::endl ;
		return( ERROR_CURL_INIT ) ;
	}

	/* BEGIN: configure the handle: */

	// handy for debugging: see *everything* that goes on
	// curl_easy_setopt( ctx , CURLOPT_VERBOSE, OPTION_TRUE ) ;

	// target url:
	curl_easy_setopt( ctx , CURLOPT_URL,  url ) ;

	// no progress bar:
	curl_easy_setopt( ctx , CURLOPT_NOPROGRESS , OPTION_TRUE ) ;

	/*
	// for curl 7.11 and earlier
	char errorBuf[ CURL_ERROR_SIZE ] ;
	std::fill( errorBuf , errorBuf + CURL_ERROR_SIZE , '\0' ) ;
	curl_easy_setopt( ctx , CURLOPT_ERRORBUFFER , errorBuf ) ;
	*/


	// what to do with returned data

	// received headers:
	/*
	By default, headers are stripped from the output.
	They can be:

	- passed through a separate FILE* (CURLOPT_WRITEHEADER)

	- included in the body's output (CURLOPT_HEADER -> nonzero value)
		(here, the headers will be passed to whatever function
		 processes the body, along w/ the body)

	- handled with separate callbacks (CURLOPT_HEADERFUNCTION)
		(in this case, set CURLOPT_WRITEHEADER to a
		 matching struct for the function)

	*/
	
	// (sending response headers to stderr)
	curl_easy_setopt( ctx , CURLOPT_WRITEHEADER , stderr ) ;


	// response content: same choices as headers
	// send it to stdout to prove that libcURL differentiates the two
	curl_easy_setopt( ctx , CURLOPT_WRITEDATA , stdout ) ;

	/* END: configure the handle */


	// action!

	const CURLcode rc = curl_easy_perform( ctx ) ;

	// for curl v7.11.x and earlier, look into
	// curl_easy_setopt( ctx , CURLOPT_ERRORBUFFER , /* char array */ ) ;
	if( CURLE_OK != rc ){

		std::cerr << "Error from cURL: " << curl_easy_strerror( rc ) << std::endl ;

	}else{

		// get some info about the xfer:
		double statDouble ;
		long statLong ;
		char* statString = NULL ;

		// known as CURLINFO_RESPONSE_CODE in later curl versions
		if( CURLE_OK == curl_easy_getinfo( ctx , CURLINFO_HTTP_CODE , &statLong ) ){
			std::cout << "Response code:  " << statLong << std::endl ;
		}

		if( CURLE_OK == curl_easy_getinfo( ctx , CURLINFO_CONTENT_TYPE , &statString ) ){
			std::cout << "Content type:   " << statString << std::endl ;
		}

		if( CURLE_OK == curl_easy_getinfo( ctx , CURLINFO_SIZE_DOWNLOAD , &statDouble ) ){
			std::cout << "Download size:  " << statDouble << "bytes" << std::endl ;
		}

		if( CURLE_OK == curl_easy_getinfo( ctx , CURLINFO_SPEED_DOWNLOAD , &statDouble ) ){
			std::cout << "Download speed: " << statDouble << "bytes/sec" << std::endl ;
		}

	}

	// cleanup
	curl_easy_cleanup( ctx ) ;
	curl_global_cleanup() ;

	return( 0 ) ;

} // main()
