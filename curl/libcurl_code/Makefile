##
## sample code for O'ReillyNet article on libcURL programming
##

##
## NOTE: 
##
## The values of "CXXFLAGS_CURL" and "LDFLAGS_CURL" are generated from
## the command "curl-config," which should ship with the libcURL
## development kit.
## 
## If these examples fail to build on your system, first replace the
## values of those macros:
## 
## 	"CXXFLAGS_CURL" is "curl-config --cflags"
## 	"LDFLAGS_CURL" is "curl-config --libs"
## 
## 

CXXFLAGS_BASIC = -g -I. -Wall

CXXFLAGS_CURL =

CXXFLAGS = ${CXXFLAGS_BASIC} ${CXXFLAGS_CURL}

CXX = c++

LDFLAGS_BASIC = -g

LDFLAGS_CURL = -lcurl -L/usr/kerberos/lib -lssl -lcrypto -lgssapi_krb5 -lkrb5 -lcom_err -lk5crypto -lresolv -ldl -lz -L/usr/kerberos/lib -lz -lssl -lcrypto -lssl -lcrypto -lgssapi_krb5 -lkrb5 -lcom_err -lk5crypto -lresolv -ldl -lz -lz

LDFLAGS = ${LDFLAGS_BASIC} ${LDFLAGS_CURL} ${LDFLAGS_EXPAT}

##

PROGRAMS = step1 step2 step3 step4

all: warning ${PROGRAMS}

warning:
	@echo
	@echo "NOTE:"
	@echo " See the note in the Makefile if this fails to build."
	@echo " You may have to rerun curl-config to generate compiler/linker flags."
	@echo

step1: step1-http-get.cc
	${CXX} ${CXXFLAGS} -o ${@} ${?} ${LDFLAGS}

step2: step2-ftp-anon-download.cc
	${CXX} ${CXXFLAGS} -o ${@} ${?} ${LDFLAGS}

step3: step3-ftp-auth-upload.cc
	${CXX} ${CXXFLAGS} -o ${@} ${?} ${LDFLAGS}

step4: step4-http-post.cc
	${CXX} ${CXXFLAGS} -o ${@} ${?} ${LDFLAGS}


clean:
	-rm -f ${PROGRAMS}
