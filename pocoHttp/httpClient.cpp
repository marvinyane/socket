#include <iostream>
#include <string>

#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPClientSession.h"


int main()
{
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, "/");
    request.setKeepAlive(true);
    Poco::Net::HTTPResponse response;
    Poco::Net::HTTPClientSession session("127.0.0.1", 9090);

    session.setKeepAlive(false);
    session.sendRequest(request);

    std::istream &is = session.receiveResponse(response);

    int length = response.getContentLength();
    Poco::Net::HTTPResponse::HTTPStatus status = response.getStatus();

    std::cout << "length is " << length << "\n";
    std::cout << "status is " << status << "\n";

#if 0
    while (!is.eof())
    {
        std::string s;
        is >> s;

        std::cout << "received : " << s << "\n";
    }
#endif

    //std::string str(std::istreambuf_iterator<char>(is),  std::istreambuf_iterator<char>());
    std::string str((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    std::cout << "length of str is " << str.length() << "\n";
    std::cout << str;

    return 0;
}
