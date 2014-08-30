#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#include "logger_stdout.h"
#include "networklayer.h"
#include "ua_application.h"
#include "ua_channel_manager.h"
#include "ua_session_manager.h"
#include "ua_server.h"

UA_Boolean running = UA_TRUE;

void stopHandler(int sign) {
	running = UA_FALSE;
}

UA_Int32 serverCallback(void * arg) {
	char *name = (char *) arg;
	printf("%s does whatever servers do\n",name);

	Namespace* ns0 = (Namespace*)UA_indexedList_find(appMockup.namespaces, 0)->payload;
	UA_Int32 retval;
	const UA_Node * node;
	UA_ExpandedNodeId serverStatusNodeId; NS0EXPANDEDNODEID(serverStatusNodeId, 2256);
	retval = Namespace_get(ns0, &serverStatusNodeId.nodeId, &node);
	if(retval == UA_SUCCESS){
		((UA_ServerStatusDataType*)(((UA_VariableNode*)node)->value.data))->currentTime = UA_DateTime_now();
	}

	return UA_SUCCESS;
}


int main(int argc, char** argv) {
	UA_Int32 retval;
	/* gets called at ctrl-c */
	signal(SIGINT, stopHandler);
	
	appMockup_init();
	NL_data* nl = NL_init(&NL_Description_TcpBinary, 16664);
	UA_String endpointUrl;
	UA_String_copycstring("no endpoint url",&endpointUrl);
	SL_ChannelManager_init(10,36000,244,2,&endpointUrl);
	UA_SessionManager_init(10,3600000,25);
	struct timeval tv = {1, 0}; // 1 second

	retval = NL_msgLoop(nl, &tv, serverCallback, argv[0], &running);

	return retval == UA_SUCCESS ? 0 : retval;
}
