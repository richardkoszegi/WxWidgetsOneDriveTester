#ifndef _OAUTHP_H_
#define _OAUTHP_H_

//#include "BaseProvider.h"
//#include "Catalog.h"
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

namespace Provider
{
	class OAuthProvider
	{
	private:
		//wxString auth_code;
		wxString access_token, refresh_token;
		wxDateTime expire_time;
		wxCriticalSection authCS;
	public:
		OAuthProvider() : expire_time((time_t)0), access_token(""), refresh_token("")
		{}
		virtual ~OAuthProvider() {}

		//virtual wxString GetName() = 0;
		//virtual wxString GetDisplayName() = 0;
		//virtual wxString GetUserName() = 0;

		/*wxString GetAuthCode()
		{
		wxCriticalSectionLocker enter(authCS);
		wxString result = auth_code.Clone();
		return result;
		}*/

		wxString GetAccessToken()
		{
			wxCriticalSectionLocker enter(authCS);
			wxString result = access_token.Clone();
			return result;
		}

		wxString GetRefreshToken()
		{
			wxCriticalSectionLocker enter(authCS);
			wxString result = refresh_token.Clone();
			return result;
		}

		// Saves the credentials to cred parameter.
		virtual wxString SerializeCredentials()
		{
			wxCriticalSectionLocker enter(authCS);
			//wxString auth_code_enc = wxBase64Encode(auth_code.c_str(), auth_code.Len());
			//wxString refresh_token_enc = wxBase64Encode(refresh_token.c_str(), refresh_token.Len());//Ricsiszedteki
			//return /*auth_code_enc + ':' +*/ refresh_token_enc;
			return "Nope";
		}

		// Loads the credentials from cred parameter.
		virtual void DeserializeCredentials(const wxString& cred)
		{
			wxCriticalSectionLocker enter(authCS);
			/*auth_code = cred.BeforeFirst(':', &refresh_token);
			char decoded[2000];
			size_t dsize = wxBase64Decode(decoded,2000, auth_code, wxBase64DecodeMode_Relaxed); //wxBase64DecodeMode
			decoded[dsize] = '\0';
			auth_code = wxString(decoded);
			dsize = wxBase64Decode(decoded,2000, refresh_token, wxBase64DecodeMode_Relaxed);
			decoded[dsize] = '\0';*/
			char decoded[2000];
			//size_t dsize = wxBase64Decode(decoded, 2000, cred, wxBase64DecodeMode_Relaxed);//Ricsiszedteki
			//decoded[dsize] = '\0';
			//refresh_token = wxString(decoded);
			// Aquire access token from refresh token.
			this->StartRefreshToken();
		}

		virtual void StartRefreshToken() = 0;

		void RefreshToken(const char* access_token, const char* refresh_token, unsigned int expires_sec)
		{
			wxCriticalSectionLocker enter(authCS); // Locks authCS until destroyed.
			this->access_token = wxString(access_token);
			this->expire_time = wxDateTime::Now().Add(wxTimeSpan(0, 0, expires_sec, 0));
			if (refresh_token) {
				this->refresh_token = wxString(refresh_token);
				//Catalog::UpdateCredentials(this); // Only update the config file when a new refresh token is issued.//Ricsiszedteki
				UpdateCache();
			}
		}

		// Updates user infos and file system structure.
		virtual void UpdateCache() = 0;
	};
}

#endif