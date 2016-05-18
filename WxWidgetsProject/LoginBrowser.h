#include <wx/wxprec.h>
#include <wx/webview.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class LoginBrowser : public wxDialog {
public:
	LoginBrowser(const wxString url, wxString compareToClose, wxSize size = wxSize(500, 400));
	LoginBrowser(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, const wxString title = "Login");
	virtual ~LoginBrowser();

	void OnTitleChanged(wxWebViewEvent& evt);
	void OnPageLoaded(wxWebViewEvent& evt);

	wxWebView* m_browser;
protected:
	static const long ID_WEBVIEW;
private:
	wxString compareToClose;
	void Init();
};

