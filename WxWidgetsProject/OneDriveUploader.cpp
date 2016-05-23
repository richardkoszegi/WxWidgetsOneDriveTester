#include "OneDriveUploader.h"
#include "RapidJson/document.h"
#include "RapidJson/memorystream.h"

OneDriveUploader::OneDriveUploader(wxString filePath, OneDriveHandler* odh)
{
	uploadFilePath = filePath;
	handler = odh;

	uploadedBytesNr = 0;

	packetSize = 5 * 1024 * 1024;

	wxFileName fileInfo(uploadFilePath);
	totalSize = fileInfo.GetSize().GetValue();
}


OneDriveUploader::~OneDriveUploader()
{
}

void OneDriveUploader::CreateUploadSession() {
	MyCurl curl;
	curl.SetRequestType(HTTP_POST);
	wxString authHeader = wxString("Authorization: bearer ") + handler->GetAccessToken();
	curl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));
	wxString url = wxString("https://api.onedrive.com/v1.0/drive/special/approot:") + uploadFilePath.SubString(uploadFilePath.First(":") + 1, uploadFilePath.Length()) + wxString(":/upload.createSession");
	curl.DoIt();
	if (curl.GetResponseCode() != 200)
		throw "Cannot create upload session";

	rapidjson::Document d;
	d.ParseInsitu(curl.GetResponse());

	rapidjson::Value& uploadUrlValue = d["uploadUrl"];
	uploadUrl = uploadUrlValue.GetString();
}

void OneDriveUploader::UploadFragments() {
	
}