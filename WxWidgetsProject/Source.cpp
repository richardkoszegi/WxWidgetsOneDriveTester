// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
//#include <wx/webview.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "OneDriveHandler.h"


class MyApp : public wxApp {
public:
	virtual bool OnInit();
};


class MyFrame : public wxFrame {
public:
	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
private:
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnLogin(wxCommandEvent& event);
	void UploadFile(wxCommandEvent& event);
	void CreateFolder(wxCommandEvent& event);
	wxDECLARE_EVENT_TABLE();
};


enum {
	ID_Login = 1,
	ID_UploadFile = 2,
	ID_CreateFolder = 3
};


wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(wxID_EXIT, MyFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
EVT_MENU(ID_Login, MyFrame::OnLogin)
EVT_MENU(ID_UploadFile, MyFrame::UploadFile)
EVT_MENU(ID_CreateFolder, MyFrame::CreateFolder)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
	MyFrame *frame = new MyFrame("One Drive Test", wxPoint(50, 50), wxSize(600, 600));
	frame->Show(true);
	return true;
}

OneDriveHandler* odh;

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size)
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(ID_Login, "&OneDriveLogin", "Login to OneDrive");
	menuFile->Append(ID_UploadFile, "&UploadFile", "Upload File");
	menuFile->Append(ID_CreateFolder, "&CreateFolder", "Create Folder");
	menuFile->Append(wxID_EXIT);

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);
	CreateStatusBar();
	SetStatusText("Welcome to wxWidgets!");

	odh = new OneDriveHandler();
}
void MyFrame::OnExit(wxCommandEvent& event)
{
	delete odh;
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This is a wxWidgets app for testing OneDrive Cloud.",
		"About OneDriveTest", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnLogin(wxCommandEvent& event) {
	this->SetStatusText("Log in to OneDrive");
	odh->Login();
	this->SetStatusText("Log in done!");
}

void MyFrame::CreateFolder(wxCommandEvent& event) {
	this->SetStatusText("Folder uploading...");
	odh->CreateDirectory();
	this->SetStatusText("Folder uploaded!");
}

void MyFrame::UploadFile(wxCommandEvent& event) {
	if (!odh->IsLoggedIn()) {
		wxMessageBox("Please log in to upload files!",
			"You're not logged in!", wxOK | wxICON_INFORMATION);
		return;
	}

	this->SetStatusText("File uploading...");
	wxFileDialog* OpenDialog = new wxFileDialog(this, "Choose a file to open");
	wxString filePath;
	// Creates a "open file" dialog with 4 file types
	if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "Cancel"
	{
		filePath = OpenDialog->GetPath();
		odh->UploadFile(filePath);

	}
	OpenDialog->Destroy();

	this->SetStatusText("File uploaded!");
}
