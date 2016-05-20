#pragma once

#include "OAuthProvider.h"
#include "cURL_Thread.h"

class OneDriveHandler : public Provider::OAuthProvider
{
public:
	OneDriveHandler();
	~OneDriveHandler();

	bool Login();
	void Upload();
	void Download();

	static size_t OneDrive_refresh_token_rx_handler(char *d, size_t n, size_t l, Network::Request* self);

	void UpdateCache();

	virtual void StartRefreshToken();

	static size_t OneDrive_HelloFile_Callback(char *d, size_t n, size_t l, Network::Request* self);

	void UploadHelloFile();

	void UploadSmallFile();
private:
	wxString rootDirId;

	void GetRootDirId();

	wxString uploadFileName;
	wxString uploadFilePath;
};

