#pragma once

#include "OAuthProvider.h"
#include "cURL_Thread.h"

class OneDriveHandler : public Provider::OAuthProvider
{
public:
	OneDriveHandler();
	~OneDriveHandler();

	bool Login();
	void UploadFile(wxString filePath);
	void DownloadFile();

	static size_t OneDrive_refresh_token_rx_handler(char *d, size_t n, size_t l, Network::Request* self);

	void UpdateCache();

	virtual void StartRefreshToken();

	static size_t OneDrive_HelloFile_Callback(char *d, size_t n, size_t l, Network::Request* self);

	void UploadSmallFile(wxString filePath);

	void CreateDirectory();

	void UploadLargeFile(wxString filePath);

	bool IsLoggedIn();

	void ListFolder();
private:
	wxString rootDirId;

	void GetRootDirId();

	bool uploadToPath;

	//Test Upload Directory
	wxString uploadDirectoryName;

};

