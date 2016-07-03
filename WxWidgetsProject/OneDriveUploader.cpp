#include "OneDriveUploader.h"
#include "RapidJson/document.h"
#include "RapidJson/memorystream.h"

OneDriveUploader::OneDriveUploader(wxString filePath, OneDriveHandler* odh)
{
	uploadFilePath = filePath;
	handler = odh;

	uploadedBytesNr = 0;

	//packetSize = 5 * 1024 * 1024;
	packetSize = 20 * 320 * 1024;

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

	wxString modifiedFilePath = uploadFilePath.SubString(uploadFilePath.find_last_of("\\") +1, uploadFilePath.size());
	if (modifiedFilePath.Contains(" ")) {
		modifiedFilePath.Replace(" ", "%20", true);
	}
	modifiedFilePath.Replace("\\", "/", true);
	wxString url = wxString("https://api.onedrive.com/v1.0/drive/special/approot:/") + modifiedFilePath + wxString(":/upload.createSession");
	//wxString url = wxString("https://api.onedrive.com/v1.0/drive/root:/") + modifiedFilePath + wxString(":/upload.createSession");
	curl.SetUrl(const_cast<char *>(url.mb_str().data()));
	curl.SetToLargeUpload();
	curl.DoIt();
	if (curl.GetResponseCode() != 200) {
		std::string message = "Cannot start upload! (Error code: " + std::to_string(curl.GetResponseCode()) + ")";
		wxMessageBox( message, "Error!", wxOK | wxICON_ERROR);
		return;
	}

	rapidjson::Document d;
	d.ParseInsitu(const_cast<char*>(curl.GetResponse().c_str()));

	rapidjson::Value& uploadUrlValue = d["uploadUrl"];
	uploadUrl = uploadUrlValue.GetString();
}

void OneDriveUploader::UploadFragments() {
	int nrOfPackets = 1 + ((totalSize - 1) / packetSize);
	int httpCode;
	wxFile file(uploadFilePath, wxFile::read);

	/*progressDialog = new wxProgressDialog("Upload progress", "Your upload in progress:", totalSize);
	progressDialog->ShowModal();*/
	for (int currentPacketNr = 0; currentPacketNr < nrOfPackets; currentPacketNr++) {
		MyCurl curl;
		curl.SetUrl(const_cast<char *>(uploadUrl.mb_str().data()));
		curl.SetRequestType(HTTP_PUT);

		wxString authHeader = wxString("Authorization: bearer ") + handler->GetAccessToken();
		curl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));

		if (totalSize - uploadedBytesNr < packetSize) {
			std::string lengthHeader = "Content-Length: " + std::to_string(totalSize - uploadedBytesNr);
			curl.AddHeader(const_cast<char *>(lengthHeader.c_str()));
			std::string rangeHeader = "Content-Range: bytes " + std::to_string(currentPacketNr * packetSize) + "-" + std::to_string(totalSize - 1) + "/" + std::to_string(totalSize);
			curl.AddHeader(const_cast<char *>(rangeHeader.c_str()));

			curl.SetData(file, totalSize - uploadedBytesNr, currentPacketNr * packetSize);
		}
		else {
			std::string lengthHeader = "Content-Length: " + std::to_string(packetSize);
			curl.AddHeader(const_cast<char *>(lengthHeader.c_str()));
			std::string rangeHeader = "Content-Range: bytes " + std::to_string(currentPacketNr * packetSize) + "-" + std::to_string(((currentPacketNr + 1) * packetSize) - 1) + "/" + std::to_string(totalSize);
			curl.AddHeader(const_cast<char *>(rangeHeader.c_str()));

			curl.SetData(file, packetSize, currentPacketNr * packetSize);
		}
		//curl.setResumableXferFunction(this);

		curl.DoIt();
		uploadedBytesNr += packetSize;
		httpCode = curl.GetResponseCode();
		wxLogDebug(std::to_string(httpCode).c_str());
		if ((nrOfPackets - currentPacketNr) < 1 && !(httpCode == 202)) {
			CancelUpload();
			std::string message = "Upload cancelled! (Error code: " + std::to_string(httpCode) + ")";
			wxMessageBox( message, "Error!", wxOK | wxICON_ERROR);
		}
	}
	//delete progressDialog;
	if (httpCode == 201 || httpCode == 200) {
		wxMessageBox("File uploaded successfully!", "File Uploaded!", wxOK | wxICON_INFORMATION);
	}
	else
		if (httpCode == 409)
			wxMessageBox("Manual commit needed!", "Warning!", wxOK | wxICON_WARNING);
		else
			CancelUpload();
}

void OneDriveUploader::CancelUpload() {
	MyCurl curl;
	wxString authHeader = wxString("Authorization: bearer ") + handler->GetAccessToken();
	curl.AddHeader(const_cast<char *>(authHeader.mb_str().data()));
	curl.SetRequestType(HTTP_DELETE);
	curl.SetUrl(const_cast<char *>(uploadUrl.mb_str().data()));
	curl.DoIt();

	if (curl.GetResponseCode() == 204)
		wxLogDebug("Sikeres megszakitas");
}

void OneDriveUploader::SetProgressBar(curl_off_t ulnow) {
	progressDialog->Update(uploadedBytesNr + ulnow);
}