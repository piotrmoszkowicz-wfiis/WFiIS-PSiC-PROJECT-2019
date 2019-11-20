#include <iostream>
#include <string>

#include "HttpServer.h"

 /*
	When receiving TCP data, a ZMQ_STREAM socket shall prepend a message part containing the identity of the originating peer to the message before passing it to the application. 
	Messages received are fair-queued from among all connected peers.
 */

int main()
{
	std::cout << "-------------------------------------- Start -----------------------------------------\n";

	HttpServer httpServer{};

	if(!httpServer.Initialize())
	{
		std::cout << "HttpServer Initialize failed \n";
		return 1;
	}

	if(!httpServer.Start())
	{
		std::cout << "HttpServer Start failed \n";
		return 1;
	}


	std::cout << "-------------------- Finish --------------------------" << std::endl;
    return 0;
}