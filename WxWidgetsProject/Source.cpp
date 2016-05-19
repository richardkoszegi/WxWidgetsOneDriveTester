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
	void UploadHello(wxCommandEvent& event);
	void UploadSmallFile(wxCommandEvent& event);
	wxDECLARE_EVENT_TABLE();
};


enum {
	ID_Login = 1,
	ID_UploadHello = 2,
	ID_UploadSmallFile = 3
};


wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(wxID_EXIT, MyFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
EVT_MENU(ID_Login, MyFrame::OnLogin)
EVT_MENU(ID_UploadHello, MyFrame::UploadHello)
EVT_MENU(ID_UploadSmallFile, MyFrame::UploadSmallFile)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(MyApp);
bool MyApp::OnInit()
{
	MyFrame *frame = new MyFrame("One Drive Test", wxPoint(50, 50), wxSize(600, 600));
	frame->Show(true);
	return true;
}
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size)
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(wxID_EXIT);

	wxMenu *menuODLogin = new wxMenu;
	menuODLogin->Append(ID_Login, "&OneDriveLogin", "Login to OneDrive");
	menuODLogin->Append(ID_UploadHello, "&UploadHello", "Upload Hello file");
	menuODLogin->Append(ID_UploadSmallFile, "&UploadSmallFile", "Upload Small File");

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuODLogin, "&Login");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);
	CreateStatusBar();
	SetStatusText("Welcome to wxWidgets!");
}
void MyFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("This is a wxWidgets app for testing OneDrive Cloud.",
		"About OneDriveLoginTest", wxOK | wxICON_INFORMATION);
}

OneDriveHandler* odh;

void MyFrame::OnLogin(wxCommandEvent& event) {
	odh = new OneDriveHandler();
	this->SetStatusText("Log in to OneDrive");
	odh->Login();
	this->SetStatusText("Log in done!");
}

void MyFrame::UploadHello(wxCommandEvent& event) {
	odh->UploadHelloFile();
	this->SetStatusText("Hello file uploaded!");
}

void MyFrame::UploadSmallFile(wxCommandEvent& event) {
	this->SetStatusText("Log in done!");
	odh->UploadSmallFile();
	this->SetStatusText("File uploaded!");
}
