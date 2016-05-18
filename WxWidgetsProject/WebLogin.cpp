#include "WebLogin.h"

#ifndef WX_PRECOMP
#include <wx/intl.h>
#include <wx/string.h>
#endif

const long WebLogin::ID_WEBVIEW = wxNewId();

WebLogin::WebLogin(wxString url, wxString compareToClose, wxSize size)
	:wxDialog(NULL, wxID_ANY, "Login", wxDefaultPosition, size), compareToClose(compareToClose)
{
	Init();
	m_browser = wxWebView::New(this, ID_WEBVIEW, url);
	Connect(ID_WEBVIEW, wxEVT_WEBVIEW_TITLE_CHANGED, wxWebViewEventHandler(WebLogin::OnTitleChanged), NULL, this);
	Connect(ID_WEBVIEW, wxEVT_WEBVIEW_LOADED, wxWebViewEventHandler(WebLogin::OnPageLoaded), NULL, this);
}

WebLogin::WebLogin(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, wxString title)
	:wxDialog(parent, id, title, pos, size)
{
	Init();
}

WebLogin::~WebLogin()
{

}

void WebLogin::Init()
{

}

void WebLogin::OnTitleChanged(wxWebViewEvent& evt)
{
	if (m_browser->GetCurrentTitle().Contains(compareToClose)) { this->Close(true); }
}
void WebLogin::OnPageLoaded(wxWebViewEvent& evt)
{
	wxString url = m_browser->GetCurrentURL();
	if (m_browser->GetCurrentURL().Contains(compareToClose)) { this->Close(true); }
}
