// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#include <wx/webview.h>
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
	void OnShowODToken(wxCommandEvent& event);
	void UploadHello(wxCommandEvent& event);
	void UploadSmallFile(wxCommandEvent& event);
	wxDECLARE_EVENT_TABLE();

};


enum {
	ID_Login = 1,
	ID_ShowToken = 2,
	ID_UploadHello = 3,
	ID_UploadSmallFile = 4
};


wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(wxID_EXIT, MyFrame::OnExit)
EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
EVT_MENU(ID_Login, MyFrame::OnLogin)
EVT_MENU(ID_ShowToken, MyFrame::OnShowODToken)
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
	menuODLogin->Append(ID_ShowToken, "&ShowODToken", "Show OneDrive Token");
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

void DoLogin();

OneDriveHandler* odh;

void MyFrame::OnLogin(wxCommandEvent& event) {
	/*const long ID_WEBVIEW = wxNewId();
	browser = wxWebView::New();
	browser->Create(this,1,"https://login.live.com/oauth20_authorize.srf?client_id=000000004818EA82&scope=wl.signin%20wl.offline_access%20onedrive.readwrite&response_type=code&display=popup&redirect_uri=https://login.live.com/oauth20_desktop.srf");
	browser->PostSizeEventToParent();
	this->SetStatusText("Log in to OneDrive");
	Connect(ID_WEBVIEW, wxEVT_WEBVIEW_TITLE_CHANGED, wxWebViewEventHandler(MyFrame::OnTitleChanged), NULL, this);
	Connect(ID_WEBVIEW, wxEVT_WEBVIEW_LOADED, wxWebViewEventHandler(MyFrame::OnPageLoaded), NULL, this);*/
	//DoLogin();

	odh = new OneDriveHandler();
	odh->Login();

	std::cout << "Vegigment";
}

void MyFrame::OnShowODToken(wxCommandEvent& event) {
	wxMessageBox(odh->GetAccessToken(),
		"About OneDriveLoginTest", wxOK | wxICON_INFORMATION);
}

void MyFrame::UploadHello(wxCommandEvent& event) {
	odh->UploadHelloFile();
}

void MyFrame::UploadSmallFile(wxCommandEvent& event) {
	odh->UploadSmallFile();
}

//void DoLogin() {
//	LoginBrowser* lb = new LoginBrowser("https://login.live.com/oauth20_authorize.srf?client_id=000000004818EA82&scope=wl.signin%20wl.offline_access%20onedrive.readwrite&response_type=code&display=popup&redirect_uri=https://login.live.com/oauth20_desktop.srf", "code=", wxSize(400, 550));
//	lb->ShowModal();
//	wxString url = lb->m_browser->GetCurrentURL();
//	delete lb;
//
//	int acode_start = url.First("code=");
//	wxString auth_code;
//	if (acode_start == -1) {
//		auth_code = "Nincs";
//	}
//	else {
//		auth_code = url.SubString(acode_start + 5, url.Length());
//		int acode_end = auth_code.First('&');
//		if (acode_end != -1) auth_code = auth_code.SubString(0, acode_end - 1);
//	}
//	//wxLogDebug(wxString("AuthCode: ") + auth_code);
//	wxMessageBox("code="+auth_code,
//		"Da token", wxOK | wxICON_INFORMATION);
//}
