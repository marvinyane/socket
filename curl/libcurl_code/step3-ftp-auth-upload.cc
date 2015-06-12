/*

sample for O'ReillyNet article on libcURL:
	{TITLE}
	{URL}
	AUTHOR: Ethan McCallum

auth ftp/upload (scenario: send file to remote host)

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
#include<sstream>
#include<string>
#include<sstream>
#include<algorithm>
#include<stdexcept>


#include<fstream>

extern "C" {
	#include<curl/curl.h>
}

extern "C" {
	#include<time.h> // time()
}

// - - - - - - - - - - - - - - - - - - - -

/*

A template class that acts as a pass-through.  This lets us use
a true object as a handler.

I could have achieved the same effect using inheritance:

	// ... inside UploadHandler::execute() ...
	
	Handler* h = static_cast< Handler* >( userData ) ;
	return( h->execute( ... ) ) ;


Neither one is "right" or "wrong"; when possible, though, I personally prefer
the flexibility of templates to explicit attachment via inheritance.
*/

template< typename T >
class UploadHandler {

	public:
	static size_t execute( char* buffer , size_t size , size_t nitems , void* userData ){

		// buffer: buffer we fill with data
		// size * nitems: max num of chars we can put in bufptr for this call
		// userData: state data, here an object that does the real work

		T* realHandler = static_cast< T* >( userData ) ;

		return( realHandler->execute( buffer , size , nitems ) ) ;
	
	} // execute()

} ; // UploadHandler

class UploadData {

	public:

	UploadData()
		:
		data_()
	{

		// load up the stream with data

		data_ << "[BEGIN: FTP data]" << std::endl ;

		for( int ix = 1 ; ix <= 1000 ; ++ix ){
			data_ << "line " << ix << ": pretend this data is generated on the fly, and not canned" << std::endl ;
		}

		data_ << "[END: FTP data]" << std::endl ;

  		std::cout << "data is " << data_.seekg( 0 , std::ios::end ).tellg() << " bytes" << std::endl ;

		// reset the read ("get") pointer to the beginning of the data stream
  		data_.seekg( 0 , std::ios::beg );

		return ;

	} // ctor

	size_t execute( char* buffer , size_t size , size_t nitems ){


		const int bufferLength = ( size * nitems ) ;

		const int bytesRead = data_.readsome( buffer , bufferLength ) ;
		// std::cout.write( buffer , bytesRead ) ;

		std::cout << "[UploadData::execute() called: buffer size: " << bufferLength
			<< " bytes; written to buffer: " << bytesRead << " bytes]"
			<< std::endl
		; 

		// return values:
		//   0 --> done with upload
		//   greater than 0 --> number of bytes put into the buffer
		//   less than 0 --> fatal error

		if( data_.eof() ){
			return( 0 ) ;
		}else{
			return( bytesRead ) ; 
		}

	} // execute()

	private:
	std::stringstream data_ ;

	// not implemented
	UploadData( const UploadData& other ) ;
	UploadData& operator=( const UploadData& other ) ;

} ; // class UploadData


// - - - - - - - - - - - - - - - - - - - -



enum {
	ERROR_ARGS = 1 ,
	ERROR_CURL_INIT = 2
} ;

enum {
	OPTION_FALSE = 0 ,
	OPTION_TRUE = 1
} ;


// - - - - - - - - - - - - - - - - - - - -

std::string timeAsString(){

	std::ostringstream buf ;
	buf << time( NULL ) << std::flush ;
	return( buf.str() ) ;

} // timeAsString()


// - - - - - - - - - - - - - - - - - - - -


int main( int argc , char** argv ){

	if( argc != 4 ){
		std::cerr << "test of libcURL: perform FTP upload" << std::endl ;
		std::cerr << " Usage: " << argv[0] << " {host} {login} {password}" << std::endl ;
		std::exit( ERROR_ARGS ) ;
	}

	const char* host = argv[1] ;
	const char* login = argv[2] ;
	const char* pass = argv[3] ;


	// setup login info as a string "{login}:{password}"

	// could also specify the login/password in the URL itself,
	// e.g. "ftp://{login}:{password}@{host}"; doing this here
	// doesn't present the same security issues as placing the
	// info in a URL bar or on the commandline, as no one can
	// see the URL that libcURL uses via "ps" or standard
	// over-the-shoulder nosiness.
	std::ostringstream loginInfoBuf ;
	loginInfoBuf << login << ':' << pass << std::flush ;
	const std::string loginInfo = loginInfoBuf.str() ;

	const std::string now = timeAsString() ;

	// set path for remote upload.  Creates a new directory
	// in which to put the target file (imagine a nightly upload)
	// so the URL is of the form
	//
	//   ftp://{host}/tmp/{dated dir}/data.txt

	std::ostringstream remotePathBuf ;
	remotePathBuf << "/tmp/" << now << std::flush ;
	const std::string remotePath = remotePathBuf.str() ;

	std::ostringstream urlBuf ;
	urlBuf << "ftp://" << host << remotePath << "/data.txt" << std::flush ;
	const std::string url = urlBuf.str() ;

	std::cout << "The roundup: " << std::endl ;
	std::cout << "login info:  \""  << loginInfo << "\"" << std::endl ;
	std::cout << "remote path: \""  << remotePath << "\"" << std::endl ;
	std::cout << "url:         \""  << url << "\"" << std::endl ;


	// - - - - - - - - - - - - - - - - - - - -

	curl_global_init( CURL_GLOBAL_ALL ) ;


	CURL* ctx = curl_easy_init() ;

	if( NULL == ctx ){
		std::cerr << "Unable to initialize CURL interface" << std::endl ;
		return( ERROR_CURL_INIT ) ;
	}

	/* BEGIN: configure the handle: */

	// handy for debugging: see *everything* that goes on
	// curl_easy_setopt( ctx , CURLOPT_VERBOSE, OPTION_TRUE ) ;

	// target url:
	curl_easy_setopt( ctx , CURLOPT_URL,  url.c_str() ) ;

	// no progress bar:
	curl_easy_setopt( ctx , CURLOPT_NOPROGRESS , OPTION_TRUE ) ;

	// login/password:
	curl_easy_setopt( ctx , CURLOPT_USERPWD , loginInfo.c_str() ) ;

	// use passive FTP, if it's available
	curl_easy_setopt( ctx , CURLOPT_FTP_USE_EPSV , OPTION_TRUE ) ;

	// we don't have to explictly create the target dir path; set this option
	// instead:
	// curl_easy_setopt( ctx , CURLOPT_FTP_CREATE_MISSING_DIRS , OPTION_TRUE ) ;
	// I manually create the path in this example to demonstrate
	// how to execute arbitrary FTP commands using CURLOPT_QUOTE.



	/*
	// error buffer -- used for libcURL versions earlier than
	// v7.12.x; newer versions use curl_easy_strerror()
	char errorBuf[ CURL_ERROR_SIZE ] ;
	std::fill( errorBuf , errorBuf + CURL_ERROR_SIZE , '\0' ) ;
	curl_easy_setopt( ctx , CURLOPT_ERRORBUFFER , errorBuf ) ;
	*/

	// BEGIN: list of commands to execute before uploading file
	curl_slist* commands = NULL ;

	std::ostringstream commandBuf ;

	// yes, it's "mkd" and not "mkdir" (many clients translate it for you)
	// see RFC 959 for details:
	//
	//    http://www.ietf.org/rfc/rfc959.txt
	//
	// NOTE: creating/changing to the dir isn't necessary here; it's done as
	// part of the example of using CURLOPT_QUOTE.  You could simply have
	// specified CURLOPT_FTP_CREATE_MISSING_DIRS.

	commandBuf << "mkd " << remotePath << std::flush ;
	const std::string mkdirCommand = commandBuf.str() ;
	commands = curl_slist_append( commands , mkdirCommand.c_str() ) ;

	// NOTE: cURL will try to change dirs (based on the URL)
	// from the present directory (by default, your home dir
	// on the remote system).

	// To upload data to an absolute path on the remote host,
	// you must change to the root dir first.
	commands = curl_slist_append( commands , "cwd /" ) ;
	// from here, cURL will "cwd tmp" and "cwd {timestamp dir}"


	curl_easy_setopt( ctx , CURLOPT_QUOTE , commands ) ;
	// END: list of commands to execute before uploading file




	// specify that we're uploading data:
	curl_easy_setopt( ctx , CURLOPT_UPLOAD , OPTION_TRUE );


	// how to upload data.  The choices here are:
	//
	// 1/ specify a FILE* as CURLOPT_READDATA (upload an existing file)
	//
	// 2/ specify custom CURLOPT_READDATA and a function CURLOPT_READFUNCTION
	//    Similar to CURLOPT_WRITEFUNCTION, this may be called several
	//    times so it's up to the function to keep track of what data it
	//    has already given to libcURL in previous calls

	UploadData uploadData ;

	curl_easy_setopt( ctx , CURLOPT_READFUNCTION , UploadHandler< UploadData >::execute );
	curl_easy_setopt( ctx , CURLOPT_READDATA , &uploadData );

	/* END: configure the handle: */



	// action!

	const CURLcode rc = curl_easy_perform( ctx ) ;

	// for curl v7.11.x and earlier, look into
	// curl_easy_setopt( ctx , CURLOPT_ERRORBUFFER , /* char array */ ) ;
	if( CURLE_OK != rc ){
		std::cerr << "Error from cURL: " << curl_easy_strerror( rc ) << std::endl ;
	}


	// cleanup
	curl_slist_free_all( commands ) ;
	curl_easy_cleanup( ctx ) ;
	curl_global_cleanup() ;

	std::exit( 0 ) ;


} // main()
