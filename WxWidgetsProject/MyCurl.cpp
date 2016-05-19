#include "MyCurl.h"
#include <wx/wx.h>
#include <wx/filename.h>


MyCurl::MyCurl()
{
	curl = curl_easy_init();
	if (!curl)
		throw "Cannot initialize curl";
	headerChunk = NULL;
	requestType = HTTP_GET;
}


MyCurl::~MyCurl()
{
	curl_easy_cleanup(curl);
}

void MyCurl::AddHeader(char* header) {
	headerChunk = curl_slist_append(headerChunk, header);
}

void MyCurl::SetRequestType(RequestType type) {
	this->requestType = type;

	CURLcode code;
	switch (requestType)
	{
	case HTTP_GET:
		code = curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		break;
	case HTTP_POST:
		code = curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
		break;
	//TODO: PUT, DELETE
	}
	if (code != CURLE_OK)
		throw "Failed to set HTTPGet";
}

void MyCurl::SetUrl(char* url) {
	this->requestUrl = url;
	CURLcode code;
	code = curl_easy_setopt(curl, CURLOPT_URL, url);
	if (code != CURLE_OK)
		throw "Failed to set url";
}

bool writerFailed;

size_t static writer(void *data, size_t size, size_t nmemb, std::string *writerData) {
	if (writerData == NULL) {
		writerFailed = true;
		return 0;
	}
	char *chardata = (char*)data;
	writerData->append(chardata, size*nmemb);
	return size * nmemb;
}

void MyCurl::DoIt() {
	CURLcode code;
	code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerChunk);
	if (code != CURLE_OK)
		throw "Failed to set HTTP header";

	//String writer callback
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	if (code != CURLE_OK)
		throw "Failed to set writer func";

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	if (code != CURLE_OK)
		throw "Failed to set writer data";

	code = curl_easy_perform(curl);

	if (code != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(code));
}

char* MyCurl::GetResponse() {
	return const_cast<char *>(response.c_str());
}

size_t static datawriter(char *bufptr, size_t size, size_t nitems, void *userp) {
	/*if (userp == NULL) {
		return 0;
	}*/
	/*FILE *fileptr = fopen("Hello2.txt", "rb");
	fread(bufptr, size, nitems, fileptr);
	wxLogDebug(bufptr);*/

	FILE* f = fopen("Hello2.txt", "rb");
	wxFileName finfo("Hello2.txt");
	auto stringlength = static_cast<size_t>(finfo.GetSize().GetValue());
	//char* bufptr = new char[stringlength];
	fseek(f, 0, SEEK_SET);
	fread(bufptr, sizeof(char), stringlength, f);
	wxLogDebug(wxString(bufptr));
	fclose(f);

	//return size * nitems;
	return stringlength;
}

void MyCurl::SetToSmallFile() {
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */

	FILE* f = fopen("Hello2.txt", "rb");
	wxFileName finfo("Hello2.txt");
	auto stringlength = static_cast<size_t>(finfo.GetSize().GetValue());
	byte* data = new byte[stringlength];
	fseek(f, 0, SEEK_SET);
	fread(data, sizeof(byte), stringlength, f);
	wxLogDebug(wxString(data));
	fclose(f);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

	delete[] data;
}

