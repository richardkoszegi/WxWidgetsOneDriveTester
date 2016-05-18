#include "LoginBrowser.h"

LoginBrowser::~LoginBrowser()
{
}

const long LoginBrowser::ID_WEBVIEW = wxNewId();

LoginBrowser::LoginBrowser(wxString url, wxString compareToClose, wxSize size)//, bool(*title_change)(wxString title) )
	:wxDialog(NULL, wxID_ANY, "Login", wxDefaultPosition, size), compareToClose(compareToClose)
{
	Init();
	m_browser = wxWebView::New(this, ID_WEBVIEW, url);
	//m_browser->LoadURL(url);
	Connect(ID_WEBVIEW, wxEVT_WEBVIEW_TITLE_CHANGED, wxWebViewEventHandler(LoginBrowser::OnTitleChanged), NULL, this);
	Connect(ID_WEBVIEW, wxEVT_WEBVIEW_LOADED, wxWebViewEventHandler(LoginBrowser::OnPageLoaded), NULL, this);
}

LoginBrowser::LoginBrowser(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, wxString title)
	:wxDialog(parent, id, title, pos, size)
{
	/*Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
	SetClientSize(wxSize(367,278));
	Move(wxDefaultPosition);*/
	Init();
}

void LoginBrowser::Init()
{

}

void LoginBrowser::OnTitleChanged(wxWebViewEvent& evt)
{
	if (m_browser->GetCurrentTitle().Contains(compareToClose)) { this->Close(true); }
}
void LoginBrowser::OnPageLoaded(wxWebViewEvent& evt)
{
	wxString url = m_browser->GetCurrentURL();
	if (m_browser->GetCurrentURL().Contains(compareToClose)) { this->Close(true); }
}
