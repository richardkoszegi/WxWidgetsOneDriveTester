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
		d.ParseInsitu(curl.GetResponse());
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
		if (d.ParseInsitu(curl.GetResponse()).HasParseError()) {
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

