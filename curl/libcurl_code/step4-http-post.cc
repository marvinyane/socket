/*

sample for O'ReillyNet article on libcURL:
	{TITLE}
	{URL}
	AUTHOR: Ethan McCallum

HTTP POST (e.g. form processing or REST web services)

This code was built/tested under Fedora Core 3,
libcURL version 7.12.3.  As libcURL is under 
active development, features may be added or
removed between releases.  Be sure to check your
library version, and its corresponding documentation,
if these examples fail to build on your system.

Some concepts that are introduced in earlier examples are
not explained in detail here.


*/

#include<cstdio>
#include<iostream>
#include<string>
#include<sstream>

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

const char* targetUrl ;

// - - - - - - - - - - - - - - - - - - - -


int main( int argc , char** argv ){

	if( argc != 2 ){
		std::cerr << "test of libcURL: test an HTTP post" << std::endl ;
		std::cerr << "(post data is canned)" << std::endl ;
		std::cerr << " Usage: " << argv[0] << " {post url}" << std::endl ;
		std::exit( ERROR_ARGS ) ;
	}

	targetUrl = argv[1] ;

	curl_global_init( CURL_GLOBAL_ALL ) ;


	CURL* ctx = curl_easy_init() ;

	if( NULL == ctx ){
		std::cerr << "Unable to initialize cURL interface" << std::endl ;
		return( ERROR_CURL_INIT ) ;
	}

	/* BEGIN: configure the handle: */

	// handy for debugging: see *everything* that goes on.
	// Enable (uncomment) if you're having trouble and want
	// more detail
	// curl_easy_setopt( ctx , CURLOPT_VERBOSE, OPTION_TRUE ) ;

	// target url:
	curl_easy_setopt( ctx , CURLOPT_URL,  targetUrl ) ;

	// no progress bar:
	curl_easy_setopt( ctx , CURLOPT_NOPROGRESS , OPTION_TRUE ) ;

	/*
	// error buffer -- used for libcURL versions earlier than
	// v7.12.x; newer versions use curl_easy_strerror()
	char errorBuf[ CURL_ERROR_SIZE ] ;
	std::fill( errorBuf , errorBuf + CURL_ERROR_SIZE , '\0' ) ;
	curl_easy_setopt( ctx , CURLOPT_ERRORBUFFER , errorBuf ) ;
	*/


	// send response content to standard output
	// send it to stdout to prove that libcURL differentiates the two
	curl_easy_setopt( ctx , CURLOPT_WRITEDATA , stdout ) ;

	// custom HTTP headers: create a linked list and assign
	// it to context option CURLOPT_HTTPHEADER
	curl_slist* responseHeaders = NULL ;
	
	responseHeaders = curl_slist_append( responseHeaders , "Expect: 100-continue" ) ;
	responseHeaders = curl_slist_append( responseHeaders , "User-Agent: Some Custom App" ) ;
	curl_easy_setopt( ctx , CURLOPT_HTTPHEADER , responseHeaders ) ;

	// setup the POST data

	// notice the URL-unfriendly characters (such as "%" and "&")
	// in the POST params.  We'll call curl_escape() to produce
	// HTML-escaped versions.

	const char* postParams[] = {
		"One"      , "this has % and & symbols" ,
		"Dos"      , "value with spaces" ,
		"Trois"    , "plus+signs+will+be+escaped" ,
		"Chetirye" , "fourth param..." ,
		NULL
	} ; 


	// buffer for the POST params
	std::ostringstream postBuf ;

	const char** postParamsPtr = postParams ;

	while( NULL != *postParamsPtr ){


		// curl_escape( {string} , 0 ): replace special characters
		// (such as space, "&", "+", "%") with HTML entities.
		// ( 0 => "use strlen to find string length" )
		// remember to call curl_free() on the strings on the way out
		char* key = curl_escape( postParamsPtr[0] , FLAG_DEFAULT ) ;
		char* val = curl_escape( postParamsPtr[1] , FLAG_DEFAULT )  ;

		std::cout << "Setting POST param: \"" << key << "\" => \"" << val << "\"" << std::endl ;
		postBuf << key << "=" << val << "&" ;

		postParamsPtr += 2 ;

		// the cURL lib allocated the escaped versions of the
		// param strings; we must free them here
		curl_free( key ) ;
		curl_free( val ) ;

	}

	postBuf << std::flush ;

	// We can't really call "postBuf.str().c_str()" here, because
	// the std::string created in the middle is a temporary.  In turn,
	// the char* buf from its c_str() operation isn't guaranteed to
	// be around after the function call.
	// The solution: explicitly create the string.

	// Larger (and/or better) code would use std::string::copy() to create
	// a const char* pointer to pass to cURL, then clean it up later.
	// e.g.:
	//	const char* postData = new char*[ 1 + postBuf.tellg() ] ;
	// 	postBuf.str().copy( postData , std::string::npos ) ;
	//  postData[ postBuf.tellg() ] == '\0' ;

	const std::string postData = postBuf.str() ;


	std::cout << "post data: " << postData << std::endl ;

	curl_easy_setopt( ctx , CURLOPT_POSTFIELDS , postData.c_str() ) ;

	// do a standard HTTP POST op
	// in theory, this is automatically set for us by setting
	// CURLOPT_POSTFIELDS...
	curl_easy_setopt( ctx , CURLOPT_POST , OPTION_TRUE ) ;


	/* END: configure the handle */

	// action!

	// std::ios_base::sync_with_stdio() ;

	std::cout << "- - - BEGIN: response - - -" << std::endl ;
	CURLcode rc = curl_easy_perform( ctx ) ;
	std::cout << "- - - END: response - - -" << std::endl ;

	// "curl_easy_strerror()" available in curl v7.12.x and later
	if( CURLE_OK != rc ){
		std::cerr << '\t' << "Error from cURL: " << curl_easy_strerror( rc ) << std::endl ;
	}


	// cleanup

	curl_slist_free_all( responseHeaders ) ;
	curl_easy_cleanup( ctx ) ;
	curl_global_cleanup() ;

	std::exit( 0 ) ;


} // main()
