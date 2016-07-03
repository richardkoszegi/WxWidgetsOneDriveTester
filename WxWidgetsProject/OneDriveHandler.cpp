#include "OneDriveHandler.h"
#include "WebLogin.h"
#include "cURL_Thread.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "RapidJson/document.h"
#include "RapidJson/memorystream.h"

//Upload Directory
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "MyCurl.h"
#include <iostream>

#include <wx/filename.h>

//UploadLargeFile
#include "OneDriveUploader.h"

//HeaderTokenizer in download
#include <wx/tokenzr.h>

using namespace rapidjson;
//using namespace Provider;
OneDriveHandler::OneDriveHandler()
{
	uploadDirectoryName = "HelloDirectory";
	uploadToPath = false;
}


OneDriveHandler::~OneDriveHandler()
{
}

bool OneDriveHandler::Login() {
	WebLogin* wl = new WebLogin("https://login.live.com/oauth20_authorize.srf?\
client_id=000000004818EA82\
&scope=onedrive.readwrite%2Cwl.signin%2Cwl.offline_access\
&response_type=code\
&redirect_uri=https%3A%2F%2Flogin.live.com%2Foauth20_desktop.srf", "code=", wxSize(400, 550));
	wl->ShowModal();
	wxString url = wl->m_browser->GetCurrentURL();
	delete wl;

	int acode_start = url.First("code=");
	if (acode_start == -1) return false;
	wxString auth_code = url.SubString(acode_start + 5, url.Length());
	int acode_end = auth_code.First('&');
	if (acode_end != -1) auth_code = auth_code.SubString(0, acode_end - 1);
	//wxLogDebug(wxString("AuthCode: ") + auth_code);

	// redirect_uri, cliend_id, client_secret is from onedrive dev app registration site.
	// redirect_uri needs to be the same as sent with user login.
	Network::Request* req = new Network::Request("https://login.live.com/oauth20_token.srf", OneDrive_refresh_token_rx_handler, 0);
	req->AddPOST("client_id", "000000004818EA82");
	req->AddPOST("redirect_uri", "https://login.live.com/oauth20_desktop.srf");
	req->AddPOST("code", auth_code); // auth_code.c_str()
	req->AddPOST("grant_type", "authorization_code");
	req->SetUserData(this); // Set user data to self so we can get the instance from the callback.
	this->RefreshToken("aa", "bb", 11);
	req->DontSendChunks();
	req->TerminateString();
	//req->DontFreeRXBuf();
	Network::cURL_Thread::Get().AddRequest(req);

	// Revoke authorization rights here:
	// (will search for it later)

	//GetRootDirId();

	wxMessageBox("Sikerült bejelentkeznie a OneDrive-ra!",
		"Sikerült!", wxOK | wxICON_INFORMATION);
	return true;
}

size_t OneDriveHandler::OneDrive_refresh_token_rx_handler(char *buf, size_t n, size_t l, Network::Request* self)
{
	wxLogDebug("%s", buf);

	try {
		Document d;
		if (d.ParseInsitu(buf).HasParseError()) {
			//if(d.ParseStream(MemoryStream(buf,n*l)).HasParseError()) {
			fprintf(stderr, "Error\n");
			//throw rapidjson_exception();
		}
		if (!d.HasMember("access_token") || !d.HasMember("token_type") || !d.HasMember("expires_in") || !d.HasMember("token_type"))
			throw std::string("bad_members");

		Value& acctv = d["access_token"];
		Value& expirev = d["expires_in"];
		Value& reftv = d["refresh_token"];
		const char* access_token = acctv.GetString();
		const char* refresh_token = reftv.GetString();
		int expires_sec = expirev.GetInt();

		// Call OAuthProvider base class' token updater method.
		((OneDriveHandler*)(self->user_data))->RefreshToken(access_token, refresh_token, expires_sec);

		wxLogDebug("OneDrive Auth OK");
	}
	//catch (rapidjson_exception& e) { printf("rapidjson exception\n"); }
	catch (...) { printf("other exception\n"); }
	return n*l;
}

void OneDriveHandler::UpdateCache()
{

}

void OneDriveHandler::StartRefreshToken()
{
	Network::Request* req = new Network::Request("https://login.live.com/oauth20_token.srf", OneDrive_refresh_token_rx_handler, 0);
	req->AddPOST("client_id", "000000004818EA82");
	req->AddPOST("redirect_uri", "https://login.live.com/oauth20_desktop.srf");
	req->AddPOST("refresh_token", GetRefreshToken()); // refresh_token.c_str()
	req->AddPOST("grant_type", "refresh_token");
	req->SetUserData(this); // Set user data to self so we can get the instance from the callback.
	req->DontSendChunks();
	req->TerminateString();
	//req->DontFreeRXBuf();
	Network::cURL_Thread::Get().AddRequest(req);
}

size_t OneDriveHandler::OneDrive_HelloFile_Callback(char *d, size_t n, size_t l, Network::Request* self) {
	wxMessageBox((char *)self,
		"File Uploaded", wxOK | wxICON_INFORMATION);
	return n*l;
}

char * wxStringToChar(wxString str) {
	return const_cast<char *>(str.c_str().AsChar());
}

void OneDriveHandler::UploadSmallFile(wxString filePath) {
	MyCurl curl;
	wxString authHeader = wxString("Authorization: bearer ") + this->GetAccessToken();
	curl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));
	curl.AddHeader("Content-Type: text/plain");

	wxString url;
	if (uploadToPath) {
		wxString modifiedFilePath = filePath.SubString(filePath.First("\\"), filePath.size());
		if (modifiedFilePath.Contains(" ")) {
			modifiedFilePath.Replace(" ", "%20", true);
		}
		modifiedFilePath.Replace("\\", "/", true);
		url = wxString("https://api.onedrive.com/v1.0/drive/special/approot:") + modifiedFilePath + wxString(":/content");
	}
	else {
		wxString uploadFileName = filePath.SubString(filePath.find_last_of("\\") + 1, filePath.size());
		if (uploadFileName.Contains(" ")) {
			uploadFileName.Replace(" ", "%20", true);
		}
		url = wxString("https://api.onedrive.com/v1.0/drive/special/approot:/") + uploadFileName + wxString(":/content");
	}

	curl.SetUrl(const_cast<char *>(url.c_str().AsChar()));

	curl.SetRequestType(HTTP_PUT);
	curl.SetToSmallFile(filePath);
	auto uploadFileInfo = wxFileName(filePath);

	wxProgressDialog* progressDialog = new wxProgressDialog("Upload progress", "Your upload in progress:", uploadFileInfo.GetSize().GetValue());
	progressDialog->ShowModal();
	curl.setXferFunction(progressDialog);

	curl.DoIt();
	delete progressDialog;

	if (curl.GetResponseCode() == 201 || curl.GetResponseCode() == 200) {
		Document d;
		char* data = const_cast<char *>(curl.GetResponse().c_str());
		d.ParseInsitu(data);
		Value& nameValue = d["name"];
		wxString message = nameValue.GetString() + wxString(" file uploaded successfully!");
		wxMessageBox(message, "File Uploaded!", wxOK | wxICON_INFORMATION);
	}
	else {
		std::string message = "Error code: ";
		message+= std::to_string( curl.GetResponseCode());
		wxMessageBox(message, "Error!", wxOK | wxICON_ERROR);
	}
}

void OneDriveHandler::GetRootDirId() {
	MyCurl curl;
	wxString authHeader = wxString("Authorization: bearer ") + this->GetAccessToken();
	curl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));
	curl.SetRequestType(HTTP_GET);
	curl.SetUrl("https://api.onedrive.com/v1.0/drive/special/approot");

	curl.DoIt();

	try {
		Document d;
		if (d.ParseInsitu(wxStringToChar(curl.GetResponse())).HasParseError()) {
			fprintf(stderr, "Error\n");
		}
		if (!d.HasMember("id") )
			throw std::string("bad_members");

		Value& idValue = d["id"];

		this->rootDirId = wxString(idValue.GetString());

		wxLogDebug(rootDirId);
	}
	catch (...) { printf("other exception\n"); }
}

wxString createDirectoryJson(wxString directoryName) {
	StringBuffer s;
	PrettyWriter<StringBuffer> writer(s);
	writer.StartObject();
	writer.String("name");
	writer.String(directoryName);
	writer.String("folder");
	writer.StartObject();
	writer.EndObject();
	writer.EndObject();
	wxLogDebug(s.GetString());
	return wxString(s.GetString());
}

void OneDriveHandler::CreateDirectory() {
	if (rootDirId == "")
		GetRootDirId();

	MyCurl curl;
	wxString authHeader = wxString("Authorization: bearer ") + this->GetAccessToken();
	curl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));
	curl.AddHeader("Content-Type: application/json");

	curl.SetRequestType(HTTP_POST);
	
	wxString jsondata = createDirectoryJson(uploadDirectoryName);
	curl.SetData(jsondata);

	wxProgressDialog* progressDialog = new wxProgressDialog("Upload progress", "Your upload in progress:");
	progressDialog->ShowModal();
	//curl.setXferFunction(progressDialog);
	wxString url = wxString("https://api.onedrive.com/v1.0/drive/items/") + rootDirId + wxString("/children");

	wxLogDebug(url);
	curl.SetUrl(const_cast<char *>(url.c_str().AsChar()));

	curl.DoIt();
	delete progressDialog;
	wxMessageBox(curl.GetResponse(),
		"File Uploaded", wxOK | wxICON_INFORMATION);
}

bool OneDriveHandler::IsLoggedIn() {
	if (this->GetAccessToken() == "")
		return false;
	else
		return true;
}


void OneDriveHandler::UploadLargeFile(wxString filePath) {
	OneDriveUploader uploader(filePath, this);
	uploader.CreateUploadSession();
	uploader.UploadFragments();
}

void OneDriveHandler::UploadFile(wxString filePath) {
	auto uploadFileInfo = wxFileName(filePath);
	if (uploadFileInfo.GetSize().GetValue() > (10 * 1024 * 1024))
		UploadLargeFile(filePath);
	else
		UploadSmallFile(filePath);
}

void OneDriveHandler::ListFolder() {
	MyCurl curl;
	wxString authHeader = wxString("Authorization: bearer ") + this->GetAccessToken();
	curl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));
	curl.SetRequestType(HTTP_GET);
	//curl.SetUrl("https://api.onedrive.com/v1.0/drive/special/approot:/Egyetem:/children");
	curl.SetUrl("https://api.onedrive.com/v1.0/drive/special/approot/children");
	curl.DoIt();

	Document d;
	if (d.ParseInsitu(const_cast<char*>(curl.GetResponse().c_str())).HasParseError()) {
		wxLogDebug("Error!");
	}

	Value& valuev = d["value"];
	for (SizeType i = 0; i < d["value"].Size(); i++) {
		wxString item = wxString("name: ") + d["value"][i]["name"].GetString();
		if (d["value"][i].HasMember("folder"))
			item += wxString(" (folder)");
		wxLogDebug(item);
	}
	wxMessageBox(curl.GetResponse(), "Listing Done!", wxOK | wxICON_INFORMATION);
}

int GetFileSize(wxString header) {
	wxStringTokenizer tkz(header, wxT("\n"));
	int fileSize;
	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken();
		if (token.StartsWith("Content-Range")) {
			wxString fileSizeStr = token.SubString(token.Find("/") + 1, token.Length());
			wxLogDebug(fileSizeStr);
			fileSize = std::stoi(fileSizeStr.c_str().AsChar());
			break;
		}
	}
	return fileSize;
}

int PACKET_SIZE = 1048576; //1 MiB

void DownloadLargeFile(FILE* file, wxString& url, int fileSize, wxProgressDialog* progressDialog) {
	int downloadedSize = PACKET_SIZE;
	int packetNr = 1 + ((fileSize - 1) / PACKET_SIZE);
	wxLogDebug(wxString("Packet nr:") + wxString(std::to_string(packetNr)));
	for (int currentPacketNr = 1; currentPacketNr < packetNr; currentPacketNr++) {
		wxLogDebug(std::to_string(currentPacketNr + 1) + wxString(". package"));
		MyCurl curl;
		curl.SetUrl(const_cast<char *>(url.mb_str().data()));

		int currentSize;
		if (fileSize - downloadedSize < PACKET_SIZE) {
			std::string range = "Range: bytes=" + std::to_string(currentPacketNr * PACKET_SIZE) + "-" + std::to_string(fileSize - 1);
			curl.AddHeader(const_cast<char*>(range.c_str()));
			currentSize = fileSize - downloadedSize;
		}
		else {
			std::string range = "Range: bytes=" + std::to_string(currentPacketNr * PACKET_SIZE) + "-" + std::to_string(((currentPacketNr + 1) * PACKET_SIZE) - 1);
			curl.AddHeader(const_cast<char*>(range.c_str()));
			currentSize = PACKET_SIZE;
		}

		curl.DoIt();
		std::string firstData = curl.GetResponse();
		fwrite(firstData.c_str(), sizeof(char), firstData.length(), file);
		downloadedSize += currentSize;
		progressDialog->Update(downloadedSize);
		wxLogDebug(wxString("RespCode: ") + wxString(std::to_string(curl.GetResponseCode())));
		wxLogDebug(wxString("Curr dwsize:") + wxString(std::to_string(downloadedSize)));
	}
	wxLogDebug(wxString(std::to_string(downloadedSize)) + "_" + wxString(std::to_string(fileSize)));
}

void OneDriveHandler::DownloadFile() {
	wxString filename("InfoSecIntro2014.pdf");

	MyCurl locationCurl;
	wxString authHeader = wxString("Authorization: bearer ") + this->GetAccessToken();
	locationCurl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));
	locationCurl.SetRequestType(HTTP_GET);
	wxString url = wxString("https://api.onedrive.com/v1.0/drive/special/approot:/") + filename + wxString(":/content");
	locationCurl.SetUrl(const_cast<char *>(url.mb_str().data()));
	locationCurl.SetHeaderData();
	locationCurl.DoIt();

	wxString r(locationCurl.GetResponseHeader());
	wxString downloadUrl = r.SubString(r.Find("https://"), r.Find(filename.c_str()) + (filename.size() - 1));

	MyCurl firstCurl;
	wxString rangeHeader = wxString("Range: bytes=0-") + wxString(std::to_string(PACKET_SIZE-1));
	firstCurl.AddHeader(const_cast<char*>(rangeHeader.mb_str().data()));
	firstCurl.SetUrl(const_cast<char *>(downloadUrl.mb_str().data()));
	firstCurl.SetHeaderData();
	firstCurl.DoIt();

	FILE* file;
	file = fopen(filename.c_str(), "wb");
	std::string firstData = firstCurl.GetResponse();
	fwrite(firstData.c_str(), sizeof(char), firstData.length(), file);

	wxString firstCurlHeader(firstCurl.GetResponseHeader());
	int fileSize = GetFileSize(firstCurlHeader);
	if (fileSize > PACKET_SIZE) {
		if (firstCurl.GetResponseCode() == 206) {
			wxLogDebug("Large Download");
			wxProgressDialog* progressDialog = new wxProgressDialog("Upload progress", "Your upload in progress:", fileSize);
			progressDialog->ShowModal();
			DownloadLargeFile(file, downloadUrl, fileSize, progressDialog);
			delete progressDialog;
		}
	}
	fclose(file);
	wxMessageBox("Downloaded!", "DownLoad Done!", wxOK | wxICON_INFORMATION);
}

