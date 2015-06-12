#include <vector>
#include <string>
#include <iostream>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServer.h"

#include "Poco/Net/ServerSocket.h"

#include "Poco/Util/ServerApplication.h"


using namespace std;

class MyRequestHandler : public Poco::Net::HTTPRequestHandler
{
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest &req, Poco::Net::HTTPServerResponse & resp)
        {
            std::cout << "here??????????\n";
            resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            resp.setContentType("text/html");
            resp.setContentLength(100);
            ostream& out = resp.send();

            out << "this is a message? \n";
            out.flush();
            
            out << "this is a message? \n";
            out.flush();
            sleep(1);

            out << "this is a message? \n";
            out.flush();
            sleep(1);

            //out << "this is a message? \n";
            //out.flush();
            //sleep(1);

            //out << "this is a message? \n";
            //while (1)
            {
                //sleep(1);
            }
            out.flush();
        }
};

class MyRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
    public:
        virtual Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &)
        {
            return new MyRequestHandler;
        }
};

class MyServerApp : public Poco::Util::ServerApplication
{
    protected:
        int main(const vector<string> &)
        {
            Poco::Net::HTTPServer s(new MyRequestHandlerFactory, 
                    Poco::Net::ServerSocket(9090), new Poco::Net::HTTPServerParams);

            s.start();

            cout << "Server started \n";
            waitForTerminationRequest();

            cout << "\n shutdown....\n";

            return Application::EXIT_OK;
        }
};

int main(int argc, char** argv)
{
    MyServerApp app;
    return app.run(argc, argv);
}
