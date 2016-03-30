#pragma once

//---------------------------------------------------------------------------//
//
// Transcode.hpp
//  文字列エンコード変換関数
//   Copyright (C) 2014-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <cstdint>

#include <windows.h>

//---------------------------------------------------------------------------//

using char8_t = unsigned char; // MBCS と UTF-8 を区別するため

//---------------------------------------------------------------------------//

namespace tapetums
{
    inline size_t MBCSLen (const char8_t*  str_u8);
    inline size_t MBCSLen (const char16_t* str_u16);
    inline size_t MBCSLen (const wchar_t*  str_u16);
    inline size_t UTF8Len (const char*     str_mbcs);
    inline size_t UTF8Len (const char16_t* str_u16);
    inline size_t UTF8Len (const wchar_t*  str_u16);
    inline size_t UTF16Len(const char*     str_mbcs);
    inline size_t UTF16Len(const char8_t*  str_u8);

    inline bool toMBCS (const char8_t*  str_u8,   char*     buf, size_t buf_size);
    inline bool toMBCS (const char16_t* str_u16,  char*     buf, size_t buf_size);
    inline bool toMBCS (const wchar_t*  str_u16,  char*     buf, size_t buf_size);
    inline bool toUTF8 (const char*     str_mbcs, char8_t*  buf, size_t buf_size);
    inline bool toUTF8 (const char16_t* str_u16,  char8_t*  buf, size_t buf_size);
    inline bool toUTF8 (const wchar_t*  str_u16,  char8_t*  buf, size_t buf_size);
    inline bool toUTF16(const char*     str_mbcs, char16_t* buf, size_t buf_size);
    inline bool toUTF16(const char*     str_mbcs, wchar_t*  buf, size_t buf_size);
    inline bool toUTF16(const char8_t*  str_u8,   char16_t* buf, size_t buf_size);
    inline bool toUTF16(const char8_t*  str_u8,   wchar_t*  buf, size_t buf_size);

}

//---------------------------------------------------------------------------//
//  文字列エンコード後の文字数を測る関数
//---------------------------------------------------------------------------//

// UTF-8 -> MBCS
inline size_t tapetums::MBCSLen(const char8_t* str_u8)
{
    const auto cch = 1 + UTF16Len(str_u8);
    const auto tmp = new wchar_t[cch];

    const auto len = ::MultiByteToWideChar
    (
        CP_UTF8, 0, (LPCCH)str_u8, -1, tmp, (INT32)cch
    );

    const auto ret = ::WideCharToMultiByte
    (
        CP_ACP, 0, tmp, len, nullptr, 0, nullptr, nullptr
    );

    delete tmp;

    return ret;
}

//---------------------------------------------------------------------------//

// UTF-16 -> MBCS
inline size_t tapetums::MBCSLen(const char16_t* str_u16)
{
    const auto ret = ::WideCharToMultiByte
    (
        CP_ACP, 0, (LPCWSTR)str_u16, -1, nullptr, 0, nullptr, nullptr
    );

    return (ret != 0);
}

//---------------------------------------------------------------------------//

// UTF-16 -> MBCS
inline size_t tapetums::MBCSLen(const wchar_t* str_u16)
{
    return MBCSLen((const char16_t*)str_u16);
}

//---------------------------------------------------------------------------//

// MBCS -> UTF-8
inline size_t tapetums::UTF8Len(const char* str_mbcs)
{
    const auto cch = 1 + UTF16Len(str_mbcs);
    const auto tmp = new wchar_t[cch];

    const auto len = ::MultiByteToWideChar
    (
        CP_ACP, MB_PRECOMPOSED, (LPCCH)str_mbcs, -1, tmp, (INT32)cch
    );

    const auto ret = ::WideCharToMultiByte
    (
        CP_UTF8, 0, tmp, len, nullptr, 0, nullptr, nullptr
    );

    delete tmp;

    return ret;
}

//---------------------------------------------------------------------------//

// UTF-16 -> UTF-8
inline size_t tapetums::UTF8Len(const char16_t* str_u16)
{
    const auto ret = ::WideCharToMultiByte
    (
        CP_UTF8, 0, (LPCWSTR)str_u16, -1, nullptr, 0, nullptr, nullptr
    );

    return ret;
}

//---------------------------------------------------------------------------//

// UTF-16 -> UTF-8
inline size_t tapetums::UTF8Len(const wchar_t* str_u16)
{
    return UTF8Len((const char16_t*)str_u16);
}

//---------------------------------------------------------------------------//

// MBCS -> UTF-16
inline size_t tapetums::UTF16Len(const char* str_mbcs)
{
    const auto ret = ::MultiByteToWideChar
    (
        CP_ACP, MB_PRECOMPOSED, str_mbcs, -1, nullptr, 0
    );

    return ret;
}

//---------------------------------------------------------------------------//

// UTF-8 -> UTF-16
inline size_t tapetums::UTF16Len(const char8_t* str_u8)
{
    const auto ret = ::MultiByteToWideChar
    (
        CP_UTF8, 0, (LPCCH)str_u8, -1, nullptr, 0
    );

    return ret;
}

//---------------------------------------------------------------------------//
//  文字列エンコード変換関数
//---------------------------------------------------------------------------//

// UTF-8 -> MBCS
inline bool tapetums::toMBCS(const char8_t* str_u8, char* buf, size_t buf_size)
{
    const auto cch = 1 + UTF16Len(str_u8);
    const auto tmp = new wchar_t[cch];

    const auto len = ::MultiByteToWideChar
    (
        CP_UTF8, 0, (LPCCH)str_u8, -1, tmp, (INT32)cch
    );

    const auto ret = ::WideCharToMultiByte
    (
        CP_ACP, 0, tmp, len, buf, (INT32)buf_size, nullptr, nullptr
    );

    delete tmp;

    return (ret != 0);
}

//---------------------------------------------------------------------------//

// UTF-16 -> MBCS
inline bool tapetums::toMBCS(const char16_t* str_u16, char* buf, size_t buf_size)
{
    const auto ret = ::WideCharToMultiByte
    (
        CP_ACP, 0, (LPCWCH)str_u16, -1, buf, (INT32)buf_size, nullptr, nullptr
    );

    return (ret != 0);
}

//---------------------------------------------------------------------------//

// UTF-16 -> MBCS
inline bool tapetums::toMBCS(const wchar_t* str_u16, char* buf, size_t buf_size)
{
    return toMBCS((const char16_t*)str_u16, buf, buf_size);
}

//---------------------------------------------------------------------------//

// MBCS -> UTF-8
inline bool tapetums::toUTF8(const char* str_mbcs, char8_t* buf, size_t buf_size)
{
    const auto cch = 1 + UTF16Len(str_mbcs);
    const auto tmp = new wchar_t[cch];

    const auto len = ::MultiByteToWideChar
    (
        CP_ACP, MB_PRECOMPOSED, str_mbcs, -1, tmp, (INT32)cch
    );

    const auto ret = ::WideCharToMultiByte
    (
        CP_UTF8, 0, tmp, len, (LPSTR)buf, (INT32)buf_size, nullptr, nullptr
    );

    delete tmp;

    return (ret != 0);
}

//---------------------------------------------------------------------------//

// UTF-16 -> UTF-8
inline bool tapetums::toUTF8(const char16_t* str_u16, char8_t* buf, size_t buf_size)
{
    const auto ret = ::WideCharToMultiByte
    (
        CP_UTF8, 0, (LPCWSTR)str_u16, -1, (LPSTR)buf, (INT32)buf_size, nullptr, nullptr
    );

    return (ret != 0);
}

//---------------------------------------------------------------------------//

// UTF-16 -> UTF-8
inline bool tapetums::toUTF8(const wchar_t* str_u16, char8_t* buf, size_t buf_size)
{
    return toUTF8((const char16_t*)str_u16, buf, buf_size);
}

//---------------------------------------------------------------------------//

// MBCS -> UTF-16
inline bool tapetums::toUTF16(const char* str_mbcs, char16_t* buf, size_t buf_size)
{
    const auto ret = ::MultiByteToWideChar
    (
        CP_ACP, MB_PRECOMPOSED, str_mbcs, -1, (LPWSTR)buf, (INT32)buf_size
    );

    return (ret != 0);
}

//---------------------------------------------------------------------------//

// MBCS -> UTF-16
inline bool tapetums::toUTF16(const char* str_mbcs, wchar_t* buf, size_t buf_size)
{
    return toUTF16(str_mbcs, (char16_t*)buf, buf_size);
}

//---------------------------------------------------------------------------//

// UTF-8 -> UTF-16
inline bool tapetums::toUTF16(const char8_t* str_u8, char16_t* buf, size_t buf_size)
{
    const auto ret = ::MultiByteToWideChar
    (
        CP_UTF8, 0, (LPCCH)str_u8, -1, (LPWSTR)buf, (INT32)buf_size
    );

    return (ret != 0);
}

//---------------------------------------------------------------------------//

// UTF-8 -> UTF-16
inline bool tapetums::toUTF16(const char8_t* str_u8, wchar_t* buf, size_t buf_size)
{
    return toUTF16(str_u8, (char16_t*)buf, buf_size);
}

//---------------------------------------------------------------------------//

// Transcode.hpp