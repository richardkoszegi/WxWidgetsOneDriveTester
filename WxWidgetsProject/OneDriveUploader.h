#pragma once

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "OneDriveHandler.h"
#include "MyCurl.h"

class OneDriveUploader
{
private:
	int totalSize;
	int uploadedBytesNr;
	int packetSize;	//Max 60 MiB, n*320 KiB
	wxString uploadFilePath;
	OneDriveHandler* handler;
	wxString uploadUrl;
public:
	OneDriveUploader(wxString filePath, OneDriveHandler* odh);
	~OneDriveUploader();
	void CreateUploadSession();
	void UploadFragments();
};

