/*

sample for O'ReillyNet article on libcURL:
	{TITLE}
	{URL}
	AUTHOR: Ethan McCallum


anon ftp/download (scenario: fetch remote file)

This code was built/tested under Fedora Core 3,
libcURL version 7.12.3.  As libcURL is under 
active development, features may be added or
removed between releases.  Be sure to check your
library version, and its corresponding documentation,
if these examples fail to build on your system.


Some concepts that are introduced in earlier examples are
not explained in detail here.

*/

#include<iostream>
#include<string>
#include<sstream>
#include<list>

extern "C" {
	#include<curl/curl.h>
}

// - - - - - - - - - - - - - - - - - - - -

typedef std::list< std::string > FileList ;

enum {
	ERROR_ARGS = 1 ,
	ERROR_CURL_INIT = 2
} ;

enum {
	OPTION_FALSE = 0 ,
	OPTION_TRUE = 1
} ;

// - - - - - - - - - - - - - - - - - - - -

// custom data type to be used in callback function

class XferInfo {

	private:
	int bytesTransferred_ ;
	int invocations_ ;

	protected:
	// empty

	public:
	XferInfo(){

		reset() ;

	} // ctor

	/// reset counters
	void reset(){

		bytesTransferred_ = 0 ;
		invocations_ = 0 ;

		return ;

	} // reset()

	/// add the number of bytes transferred in this call
	void add( int more ){

		bytesTransferred_ += more ;
		++invocations_ ;

		return ;

	} // add()	

	/// get the amount of data transferred, in bytes
	int getBytesTransferred() const {
		return( bytesTransferred_ ) ;
	} // getBytesTransferred()

	/// get the number of times add() has been called
	int getTimesCalled(){
		return( invocations_ ) ;
	} // getTimesCalled()
} ;

// - - - - - - - - - - - - - - - - - - - -


// C++ programmers, take note of the "extern" call for C-style linkage
extern "C"
size_t showSize( void *source , size_t size , size_t nmemb , void *userData ){

	// this function may be called any number of times for even a single
	// transfer; be sure to write it accordingly.

	// source is the actual data fetched by libcURL; cast it to whatever
	// type you need (usually char*).  It has NO terminating NULL byte.

	// we don't touch the data here, so the cast is commented out
	// const char* data = static_cast< const char* >( source ) ;

	// userData is called "stream" in the docs, which is misleading:
	// that parameter can be _any_ data type, not necessarily a FILE*
	// Here, we use it to save state between calls to this function
	// and track number of times this callback is invoked.
	XferInfo* info = static_cast< XferInfo* >( userData ) ;

	const int bufferSize = size * nmemb ;

	std::cout << '\t' << "showSize() called: " << bufferSize << " bytes passed" << std::endl ;

	// ... pretend real data processing on *source happens here ...

	info->add( bufferSize ) ;

        
	/*
	return some number less than bufferSize to indicate an
	error (xfer abort)
	
	nicer code would also set a status var (in userData) for the
	calling function
	*/

	return( bufferSize ) ;

} // showSize()


// - - - - - - - - - - - - - - - - - - - -


int main( const int argc , const char** argv ){

	if( argc < 3 ){
		std::cerr << "test of libcURL: anonymous FTP" << std::endl ;
		std::cerr << " Usage: " << argv[0] << " {server} {file1} [{file2} ...]" << std::endl ;
		return( ERROR_ARGS ) ;
	}

	// remote FTP server
	const char* server = argv[1] ;

	const int totalTargets = argc - 2 ;

	std::cout << "Attempting to download " << totalTargets
		<< " files from " << server << std::endl
	;


	curl_global_init( CURL_GLOBAL_ALL ) ;


	CURL* ctx = curl_easy_init() ;

	if( NULL == ctx ){
		std::cerr << "Unable to initialize cURL interface" << std::endl ;
		return( ERROR_CURL_INIT ) ;
	}

	/* BEGIN: global handle options */

	// handy for debugging: see *everything* that goes on
	// curl_easy_setopt( ctx , CURLOPT_VERBOSE, OPTION_TRUE ) ;

	// no progress bar:
	curl_easy_setopt( ctx , CURLOPT_NOPROGRESS , OPTION_TRUE ) ;

	// what to do with returned data
	curl_easy_setopt( ctx , CURLOPT_WRITEFUNCTION , showSize ) ;

	/*
	// for curl 7.11 and earlier
	char errorBuf[ CURL_ERROR_SIZE ] ;
	std::fill( errorBuf , errorBuf + CURL_ERROR_SIZE , '\0' ) ;
	curl_easy_setopt( ctx , CURLOPT_ERRORBUFFER , errorBuf ) ;
	*/

	/* END: global handle options */

	std::ostringstream urlBuffer ;

	// here, we'll reuse the same CURL context. cURL will attempt to 
	// reuse the same connection.  We could also have called configured
	// a single prototype context with the common options, then called
	// curl_easy_duphandle() for each call to curl_easy_perform()

	XferInfo info ;

	for(
		int ix = 2 ;
		ix < argc ;
		++ix
	){

		const char* item = argv[ ix ] ;

		// zero counters for each file
		info.reset() ;


		// target url: concatenate the server and target file name
		urlBuffer.str( "" ) ;
		urlBuffer << "ftp://" << server << "/" << item << std::flush ;

		std::cout << "Trying " << urlBuffer.str() << std::endl ;

		const std::string url = urlBuffer.str() ;

		curl_easy_setopt( ctx , CURLOPT_URL,  url.c_str() ) ;

		// set the write function's user-data (state data)
		curl_easy_setopt( ctx , CURLOPT_WRITEDATA , &info ) ;


		// action!

		const CURLcode rc = curl_easy_perform( ctx ) ;

		// for curl v7.11.x and earlier, look into
		// curl_easy_setopt( ctx , CURLOPT_ERRORBUFFER , /* char array */ ) ;
		if( CURLE_OK == rc ){
			std::cout << '\t' << "xfer size: " << info.getBytesTransferred() << " bytes" << std::endl ;
			std::cout << '\t' << "Callback was invoked " << info.getTimesCalled() << " times for this file" << std::endl ;
		} else {
			std::cerr << "Error from cURL: " << curl_easy_strerror( rc ) << std::endl ;
		}

		std::cout << std::endl ;

	}


	// cleanup
	curl_easy_cleanup( ctx ) ;
	curl_global_cleanup() ;

	return( 0 ) ;

} // main()
