#include "MyCurl.h"


MyCurl::MyCurl()
{
	curl = curl_easy_init();
	if (!curl)
		throw "Cannot initialize curl";
	headerChunk = NULL;
	requestType = HTTP_GET;

	uploadData = NULL;
}


MyCurl::~MyCurl()
{
	if(headerChunk != NULL)
		curl_slist_free_all(headerChunk);
	curl_easy_cleanup(curl);

	if (uploadData != NULL)
		delete[] uploadData;
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
	case HTTP_PUT:
		code = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		break;
	case HTTP_DELETE:
		code = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		break;
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

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

	if (code != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(code));
}

std::string& MyCurl::GetResponse() {
	return response;
}

static int progressCallback(void *clientptr, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	wxProgressDialog* progressDialog = (wxProgressDialog *)clientptr;
	progressDialog->Update(ulnow);
	return 0;
}

void MyCurl::setXferFunction(wxProgressDialog* progressDialog) {
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progressDialog);
}

void MyCurl::SetToSmallFile(wxString filePath) {
	//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */

	wxFileName finfo(filePath);
	wxFile file(filePath, wxFile::read);
	uploadData = new char[finfo.GetSize().GetValue()];
	auto uploadDataSize = file.Read(uploadData, finfo.GetSize().GetValue());

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, uploadData);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, uploadDataSize);
}

void MyCurl::SetData(wxString data) {
	uploadData = new char[data.Length()];
	memcpy(uploadData,data.c_str(),data.Len());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, uploadData);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.Len());
}

static int resumableProgressCallback(void *clientptr, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	OneDriveUploader* uploader = (OneDriveUploader *)clientptr;
	uploader->SetProgressBar(ulnow);
	return 0;
}

//void MyCurl::setResumableXferFunction(OneDriveUploader* uploader) {
//	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
//	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, resumableProgressCallback);
//	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, uploader);
//}
