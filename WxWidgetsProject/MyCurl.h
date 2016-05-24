#pragma once

#include "curl/curl.h"
#include <string>
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/progdlg.h>
#include "OneDriveUploader.h"



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

	//void setResumableXferFunction(OneDriveUploader* uploader);

	int GetResponseCode() {
		return responseCode;
	}

	//Assertion error miatt
	void SetToLargeUpload() {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	}

	void SetData(wxFile& file, size_t nCount, int offset) {
		file.Seek(offset, wxFromStart);
		uploadData = new char[nCount];
		auto readBytes = file.Read(uploadData, nCount);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, uploadData);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, readBytes);
	}

};

