#pragma once

#include "curl/curl.h"
#include <string>
#include <wx/wx.h>
#include <wx/filename.h>


enum RequestType {
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE
};

class MyCurl
{
private:
	CURL* curl;
	struct curl_slist *headerChunk;
	RequestType requestType;
	char* requestUrl;
	std::string response;
	char* uploadData;
public:
	MyCurl();
	~MyCurl();

	void AddHeader(char* header);
	void DoIt();
	void SetRequestType(RequestType);
	void SetUrl(char* url);
	char* GetResponse();

	void SetToHello() {
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "Hello From OneDriveTest"); /* data goes here */
	}

	void SetToSmallFile(wxString fileName);
};

