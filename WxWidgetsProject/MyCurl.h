#pragma once

#include "curl/curl.h"
#include <string>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/progdlg.h>


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
	int responseCode;
public:
	MyCurl();
	~MyCurl();

	void AddHeader(char* header);
	void DoIt();
	void SetRequestType(RequestType);
	void SetUrl(char* url);
	char* GetResponse();
	void setXferFunction(wxProgressDialog* progressDialog);

	void SetToSmallFile(wxString filePath);

	void SetData(wxString data);

	int GetResponseCode() {
		return responseCode;
	}
};

