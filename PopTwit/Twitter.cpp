//---------------------------------------------------------------------------//
//
// Twitter.cpp
//  Twitter 関連の関数
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#ifdef _NODEFLIB
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#undef StringCchPrintf
#define StringCchPrintf wnsprintf
#endif

#include "shlobj.h"

#include "..\Plugin.h"
#include "..\Utility.h"

#include "Twitter.h"
#include "ConsumerKey.h"

//---------------------------------------------------------------------------//
// twitcurl ライブラリ を使用
//---------------------------------------------------------------------------//

#include "include\twitcurl.h"

#ifdef _WIN64
#pragma comment(lib, "lib\\x64\\libcurl.lib")
#pragma comment(lib, "lib\\x64\\twitcurl.lib")
#else
#pragma comment(lib, "lib\\x86\\libcurl.lib")
#pragma comment(lib, "lib\\x86\\twitcurl.lib")
#endif

//---------------------------------------------------------------------------//

#if _MSC_VER <= 1800
    #if defined(_NODEFLIB)
        #define thread_local static
    #else
        #define thread_local static __declspec(thread)
    #endif
#endif

//---------------------------------------------------------------------------//
// グローバル変数
//---------------------------------------------------------------------------//

extern LPCTSTR   PLUGIN_NAME;
extern HINSTANCE g_hInst;
extern bool      g_ask_each_tweet;

//---------------------------------------------------------------------------//
// 構造体
//---------------------------------------------------------------------------//

// ダイアログに渡すデータ
struct DlgData
{
    LPCTSTR username; // ユーザー名をダイアログに渡す
    LPTSTR  password; // ここにパスワードが返ってくる
};

//---------------------------------------------------------------------------//
// ユーティリティ関数
//---------------------------------------------------------------------------//

typedef unsigned char char8_t; // MBCS と UTF-8 を区別するため

//---------------------------------------------------------------------------//

// MBCS -> UTF-8
char8_t* toUTF8(const char* str_mbcs)
{
    thread_local char16_t str_u16[1024];
    thread_local char8_t  str_u8 [1024];

    int len = 0;
    len = ::MultiByteToWideChar
    (
        CP_ACP, MB_PRECOMPOSED, str_mbcs, -1, nullptr, 0
    );
    if ( len < 1 || 1024 < len )
    {
        return (char8_t*)"";
    }

    ::MultiByteToWideChar
    (
        CP_ACP, MB_PRECOMPOSED, str_mbcs, -1, (LPWSTR)str_u16, len
    );

    len = ::WideCharToMultiByte
    (
        CP_UTF8, 0, (LPCWSTR)str_u16, -1, nullptr, 0, nullptr, nullptr
    );
    if ( len < 1 || 1024 < len )
    {
        return (char8_t*)"";
    }

    ::WideCharToMultiByte
    (
        CP_UTF8, 0, (LPCWSTR)str_u16, -1, (LPSTR)str_u8, len, nullptr, nullptr
    );

    return str_u8;
}

//---------------------------------------------------------------------------//

// UTF-16 -> UTF-8
char8_t* toUTF8(const wchar_t* str_u16)
{
    thread_local char8_t str_u8[1024];

    const auto len = ::WideCharToMultiByte
    (
        CP_UTF8, 0, str_u16, -1, nullptr, 0, nullptr, nullptr
    );
    if ( len < 1 || 1024 < len )
    {
        return (char8_t*)"";
    }

    ::WideCharToMultiByte
    (
        CP_UTF8, 0, str_u16, -1, (LPSTR)str_u8, len, nullptr, nullptr
    );

    return str_u8;
}

//---------------------------------------------------------------------------//

char* __stdcall ReadKeyFromFile(LPCTSTR username, LPCTSTR filename)
{
    TCHAR path[MAX_PATH];
    ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);

    TCHAR buf[MAX_PATH];
    ::StringCchPrintf
    (
        buf, MAX_PATH,
        TEXT("%s\\%s\\%s_%s"), path, PLUGIN_NAME, username, filename
    );

    const auto hFile = ::CreateFile
    (
        buf, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( hFile == nullptr || hFile == INVALID_HANDLE_VALUE )
    {
        return (char*)"";
    }

    DWORD dw;
    char8_t key[MAX_PATH] = { };
    ::ReadFile(hFile, key, MAX_PATH, &dw, nullptr);

    ::CloseHandle(hFile);

    return (dw > 0) ? (char*)key : (char*)"";
}

//---------------------------------------------------------------------------//

bool __stdcall WriteKeyAsFile(LPCTSTR username, LPCTSTR filename, const char8_t* key)
{
    TCHAR path[MAX_PATH];
    ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);

    TCHAR buf[MAX_PATH];
    ::StringCchPrintf
    (
        buf, MAX_PATH,
        TEXT("%s\\%s\\%s_%s"), path, PLUGIN_NAME, username, filename
    );

    const auto hFile = ::CreateFile
    (
        buf, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( hFile == nullptr || hFile == INVALID_HANDLE_VALUE )
    {
        return false;
    }

    DWORD dw;
    const auto len = ::lstrlenA((char*)key); // NULL文字は書き込まないので +1 しない
    ::WriteFile(hFile, key, len * sizeof(char8_t), &dw, nullptr);
    ::SetEndOfFile(hFile);

    ::CloseHandle(hFile);

    return (dw > 0) ? true : false;
}

//---------------------------------------------------------------------------//

bool __stdcall DeleteKeyFile(LPCTSTR username, LPCTSTR filename)
{
    TCHAR path[MAX_PATH];
    ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);

    TCHAR buf[MAX_PATH];
    ::StringCchPrintf
    (
        buf, MAX_PATH,
        TEXT("%s\\%s\\%s_%s"), path, PLUGIN_NAME, username, filename
    );

    return  ::DeleteFile(buf) ? true : false;
}

//---------------------------------------------------------------------------//
// ダイアログ プロシージャ
//---------------------------------------------------------------------------//

// パスワード入力ダイアログのプロシージャ
BOOL CALLBACK PassDlgProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    thread_local LPCTSTR username;
    thread_local LPTSTR  password = nullptr;
    thread_local HWND    txt_pass = nullptr;

    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            username = ((DlgData*)lp)->username;
            password = ((DlgData*)lp)->password;
            txt_pass = ::GetDlgItem(hwnd, 1001);
            ::SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)username);
            ::SetFocus(txt_pass);
            return FALSE;
        }
        case WM_COMMAND:
        {
            if ( LOWORD(wp) == IDOK )
            {
                ::GetWindowText(txt_pass, password, MAX_PATH);
                ::EndDialog(hwnd, IDOK);
                return TRUE;
            }
            else if ( LOWORD(wp) == IDCANCEL )
            {
                ::EndDialog(hwnd, IDCANCEL);
                return TRUE;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return  FALSE;
}

//---------------------------------------------------------------------------//
// Twitter 関連の関数
//---------------------------------------------------------------------------//

bool __stdcall Tweet
(
    HWND hwnd, size_t index, LPCTSTR username, LPCTSTR message
)
{
    TCHAR buf[MAX_PATH];

    if ( g_ask_each_tweet )
    {
        ::StringCchPrintf(buf, MAX_PATH, TEXT("@%s から送信しますか？"), username);
        const auto ret = ::MessageBox
        (
            hwnd, buf, PLUGIN_NAME, MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1
        );
        if ( ret != IDYES )
        {
            return false;
        }
    }

    twitCurl twitterObj;
    std::string replyMsg;

    // Your consumer keys
    std::string myConsumerKey( CONSUMER_KEY );
    std::string myConsumerSecuret( CONSUMER_KEY_SECRET );

    // OAuth flow begins
    twitterObj.getOAuth().setConsumerKey( myConsumerKey );
    twitterObj.getOAuth().setConsumerSecret( myConsumerSecuret );

    // Step 1: Check if we alredy have OAuth access token from a previous run
    std::string myOAuthAccessTokenKey
    (
        ReadKeyFromFile(username, TEXT("token_key"))
    );
    std::string myOAuthAccessTokenSecret
    (
        ReadKeyFromFile(username, TEXT("token_secret"))
    );

    if ( myOAuthAccessTokenKey.size() && myOAuthAccessTokenSecret.size() )
    {
        /*console_outA
        (
            "Using - Key: %s / Secret: %s",
            myOAuthAccessTokenKey.c_str(), myOAuthAccessTokenSecret.c_str()
        );*/
        twitterObj.getOAuth().setOAuthTokenKey( myOAuthAccessTokenKey );
        twitterObj.getOAuth().setOAuthTokenSecret( myOAuthAccessTokenSecret );
    }
    else
    {
        std::string userName( (char*)toUTF8(username) );
        std::string passWord;

        do
        {
            // Show dialog box for password
            DlgData data = { username, buf };
            const auto ret = ::DialogBoxParam
            (
                g_hInst, MAKEINTRESOURCE(1000), hwnd, (DLGPROC)PassDlgProc, (LPARAM)&data
            );
            if ( ret == IDCANCEL )
            {
                WriteLog(elDebug, TEXT("%s: Tweet canceled"), PLUGIN_NAME);
                return false;
            }
            passWord = (char*)toUTF8(buf);

            // Retry to get access token key and secret
            twitterObj.setTwitterUsername( userName );
            twitterObj.setTwitterPassword( passWord );

            // Step 2: Get request token key and secret
            std::string authUrl;
            twitterObj.oAuthRequestToken( authUrl );

            // Step 3: Get PIN
            // pass auth url to twitCurl and get it via twitCurl PIN handling
            twitterObj.oAuthHandlePIN( authUrl );

            // Step 4: Exchange request token with access token
            twitterObj.oAuthAccessToken();

            // Step 5: save this access token key and secret for future use
            twitterObj.getOAuth().getOAuthTokenKey( myOAuthAccessTokenKey );
            twitterObj.getOAuth().getOAuthTokenSecret( myOAuthAccessTokenSecret );

            // Step 6: Save these keys in a file or wherever
            WriteKeyAsFile
            (
                username, TEXT("token_key"), (char8_t*)myOAuthAccessTokenKey.c_str()
            );
            WriteKeyAsFile
            (
                username, TEXT("token_secret"), (char8_t*)myOAuthAccessTokenSecret.c_str()
            );

            // Account credentials verification
            if ( twitterObj.accountVerifyCredGet() )
            {
                twitterObj.getLastWebResponse( replyMsg );
                //console_outA("accountVerifyCredGet web response: %s", replyMsg.c_str());
                if ( std::string::npos == replyMsg.find("\"errors\"") )
                {
                    WriteLog(elDebug, TEXT("%s: Got access token"), PLUGIN_NAME);
                    break;
                }
            }
            else
            {
                twitterObj.getLastCurlError( replyMsg );
                //console_outA("accountVerifyCredGet error: %s", replyMsg.c_str());

                // Delete keys which are indicated to be invalid
                DeleteKeyFile(username, TEXT("token_key"));
                DeleteKeyFile(username, TEXT("token_secret"));
            }

        } while ( true );
    }

    // Post a new status message
    std::string msgStr( (char*)toUTF8(message) );
    replyMsg = (char*)"";

    if ( twitterObj.statusUpdate( msgStr ) )
    {
        twitterObj.getLastWebResponse( replyMsg );
        //console_outA("statusUpdate web response: %s", replyMsg.c_str());
        if ( std::string::npos == replyMsg.find("\"errors\"") )
        {
            WriteLog(elDebug, TEXT("%s: Succeeded"), PLUGIN_NAME);
            return true;
        }
    }
    else
    {
        twitterObj.getLastCurlError( replyMsg );
        //console_outA("statusUpdate error: %s", replyMsg.c_str());
    }

    return false;
}

//---------------------------------------------------------------------------//

bool __stdcall UploadPicture
(
    LPCTSTR filename, LPTSTR uploadedURL, int nMaxLength
)
{
    ::StringCchPrintf(uploadedURL, nMaxLength, TEXT("%s"), filename);
    return false;
}

//---------------------------------------------------------------------------//

bool __stdcall ShortenURL
(
    LPCTSTR URL, LPTSTR shortenedURL, int nMaxLength
)
{
    ::StringCchPrintf(shortenedURL, nMaxLength, TEXT("%s"), URL);
    return false;
}

//---------------------------------------------------------------------------//

// Twitter.cpp