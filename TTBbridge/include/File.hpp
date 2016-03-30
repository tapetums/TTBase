#pragma once

//---------------------------------------------------------------------------//
//
// File.hpp
//  Windows ファイル RAII クラス
//   Copyright (C) 2014-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <cstdint>

#include <algorithm>

#include <windows.h>
#include <strsafe.h>

#if defined(DELETE)
  #undef DELETE
#endif

#if defined(max)
  #undef max
#endif
#if defined(min)
  #undef min
#endif

//---------------------------------------------------------------------------//

namespace tapetums
{
    class File;
}

//---------------------------------------------------------------------------//

// Windows ファイル RAII クラス
class tapetums::File
{
public:
    enum class ACCESS : DWORD
    {
        UNKNOWN = 0,
        READ    = GENERIC_READ,
        WRITE   = GENERIC_READ | GENERIC_WRITE,
    };

    enum class SHARE : DWORD
    {
        EXCLUSIVE = 0,
        READ      = FILE_SHARE_READ,
        WRITE     = FILE_SHARE_READ | FILE_SHARE_WRITE,
        DELETE    = FILE_SHARE_DELETE,
    };

    enum class OPEN : DWORD
    {
        NEW         = CREATE_NEW,        // Fails   if existing
        OR_TRUNCATE = CREATE_ALWAYS,     // Clears  if existing
        EXISTING    = OPEN_EXISTING,     // Fails   if not existing
        OR_CREATE   = OPEN_ALWAYS,       // Creates if not existing
        TRUNCATE    = TRUNCATE_EXISTING, // Fails   if not existing
    };

    enum class ORIGIN : DWORD
    {
        BEGIN   = FILE_BEGIN,
        CURRENT = FILE_CURRENT,
        END     = FILE_END,
    };

protected:
    wchar_t m_name[MAX_PATH] { '\0' };

    HANDLE   m_handle { INVALID_HANDLE_VALUE };
    HANDLE   m_map    { nullptr };
    uint8_t* m_ptr    { nullptr };
    int64_t  m_pos    { 0 };
    int64_t  m_size   { 0 };

public:
    File() = default;
    ~File() { Close(); }

    File(const File&)             = delete;
    File& operator =(const File&) = delete;

    File(File&& rhs)             noexcept = default;
    File& operator =(File&& rhs) noexcept = default;

    File(int64_t size, LPCWSTR lpFileName, ACCESS accessMode)
    { Map(size, lpFileName, accessMode); }

    File(LPCWSTR lpFileName, ACCESS accessMode)
    { Open(lpFileName, accessMode); }

    File(LPCWSTR lpFileName, ACCESS accessMode, SHARE shareMode, OPEN createMode)
    { Create(lpFileName, accessMode, shareMode, createMode); }

public:
    bool     is_open()   const noexcept { return m_handle != INVALID_HANDLE_VALUE; }
    bool     is_mapped() const noexcept { return m_map != nullptr; }
    LPCWSTR  name()      const noexcept { return m_name; }
    HANDLE   handle()    const noexcept { return m_handle; }
    int64_t  position()  const noexcept { return m_pos; }
    uint8_t* pointer()   const noexcept { return m_map ? m_ptr + m_pos : nullptr; }
    int64_t  size()      const noexcept { return m_size; }

public:
    bool    Create(LPCWSTR lpFileName, ACCESS accessMode, SHARE shareMode, OPEN createMode);
    bool    Open(LPCWSTR lpName, ACCESS accessMode);
    void    Close();
    bool    Map(ACCESS accessMode);
    bool    Map(int64_t size, LPCWSTR lpName, ACCESS accessMode);
    void    UnMap();
    size_t  Read(void* buf, size_t size);
    size_t  Write(const void* const buf, size_t size);
    int64_t Seek(int64_t distance, ORIGIN origin = ORIGIN::BEGIN);
    bool    SetEndOfFile();
    bool    Flush(size_t dwNumberOfBytesToFlush = 0);

    template<typename T>
    size_t Read(T* t) { return Read(t, sizeof(T)); }

    template<typename T>
    size_t Write(const T& t) { return Write((const T* const)&t, sizeof(T)); }
};

//---------------------------------------------------------------------------//
// メソッド
//---------------------------------------------------------------------------//

// ファイルを開く
inline bool tapetums::File::Create
(
    LPCWSTR lpFileName,
    ACCESS  accessMode,
    SHARE   shareMode,
    OPEN    createMode
)
{
    if ( m_handle != INVALID_HANDLE_VALUE ) { return true; }

    ::StringCchCopyW(m_name, MAX_PATH, lpFileName);

    m_handle = ::CreateFileW
    (
        lpFileName, (DWORD)accessMode, (DWORD)shareMode, nullptr,
        (DWORD)createMode, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if ( m_handle == INVALID_HANDLE_VALUE )
    {
        return false;
    }

    LARGE_INTEGER li;
    ::GetFileSizeEx(m_handle, &li);
    m_size = li.QuadPart;

    return true;
}

//---------------------------------------------------------------------------//

// メモリマップトファイルを開く
inline bool tapetums::File::Open
(
    LPCWSTR lpName, ACCESS accessMode
)
{
    if ( m_map ) { return true; }
    if ( m_handle != INVALID_HANDLE_VALUE ) { return false; }

    ::StringCchCopyW(m_name, MAX_PATH, lpName);

    m_map = ::OpenFileMappingW
    (
        accessMode == ACCESS::READ ? FILE_MAP_READ : FILE_MAP_WRITE,
        FALSE, lpName
    );
    if ( nullptr == m_map )
    {
        return false;
    }

    m_ptr = (uint8_t*)::MapViewOfFile
    (
        m_map,
        accessMode == ACCESS::READ ? FILE_MAP_READ : FILE_MAP_WRITE,
        0, 0, 0
    );
    if ( nullptr == m_ptr )
    {
        UnMap();
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------//

// ファイルを閉じる
inline void tapetums::File::Close()
{
    UnMap();
    Flush();

    if ( m_handle != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }

    m_pos  = 0;
    m_size = 0;
}

//---------------------------------------------------------------------------//

// 既存のファイルをメモリにマップする
inline bool tapetums::File::Map
(
    ACCESS accessMode
)
{
    if ( m_handle == INVALID_HANDLE_VALUE ) { return false; }

    return Map(0, nullptr, accessMode);
}

//---------------------------------------------------------------------------//

// メモリマップトファイルを生成する
inline bool tapetums::File::Map
(
    int64_t size, LPCWSTR lpName, ACCESS accessMode
)
{
    if ( m_map ) { return true; }

    LARGE_INTEGER li;
    li.QuadPart = (size > 0) ? size : m_size;
    if ( li.QuadPart == 0 )
    {
        return false;
    }

    if ( m_handle == INVALID_HANDLE_VALUE )
    {
        ::StringCchCopyW(m_name, MAX_PATH, lpName);
    }

    m_map = ::CreateFileMappingW
    (
        m_handle, nullptr,
        accessMode == ACCESS::READ ? PAGE_READONLY : PAGE_READWRITE,
        li.HighPart, li.LowPart, lpName
    );
    if ( nullptr == m_map )
    {
        return false;
    }

    m_ptr = (uint8_t*)::MapViewOfFile
    (
        m_map,
        accessMode == ACCESS::READ ? FILE_MAP_READ : FILE_MAP_WRITE,
        0, 0, 0
    );
    if ( nullptr == m_ptr )
    {
        UnMap();
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------//

// メモリマップトファイルを閉じる
inline void tapetums::File::UnMap()
{
    if ( m_ptr )
    {
        ::FlushViewOfFile(m_ptr, 0);
        ::UnmapViewOfFile(m_ptr);
        m_ptr = nullptr;
    }
    if ( m_map )
    {
        ::CloseHandle(m_map);
        m_map = nullptr;
    }
}

//---------------------------------------------------------------------------//

// ファイルもしくはメモリマップトファイルから読み込む
inline size_t tapetums::File::Read
(
    void* buf, size_t size
)
{
    size_t cb { 0 };

    if ( m_map )
    {
        cb = std::min(size, (size_t)m_size - size);
        ::memcpy(buf, m_ptr + m_pos, cb);
    }
    else
    {
        ::ReadFile(m_handle, buf, (DWORD)size, (DWORD*)&cb, nullptr);
    }

    m_pos += cb;

    return cb;
}

//---------------------------------------------------------------------------//

// ファイルもしくはメモリマップトファイルに書き込む
inline size_t tapetums::File::Write
(
    const void* const buf, size_t size
)
{
    size_t cb { 0 };

    if ( m_map )
    {
        cb = std::min(size, (size_t)m_size - size);
        ::memcpy((m_ptr + m_pos), buf, cb);
    }
    else
    {
        ::WriteFile(m_handle, buf, (DWORD)size, (DWORD*)&cb, nullptr);
    }

    m_pos += cb;

    return cb;
}

//---------------------------------------------------------------------------//

// ファイルポインタを移動する
inline int64_t tapetums::File::Seek
(
    int64_t distance, ORIGIN origin
)
{
    if ( m_map )
    {
        if ( origin == ORIGIN::END )
        {
            m_pos = m_size - distance;
        }
        else if ( origin == ORIGIN::CURRENT )
        {
            m_pos += distance;
        }
        else
        {
            m_pos = distance;
        }

        if ( m_pos < 0 )
        {
            m_pos = 0;
        }
        else if ( m_pos >= m_size )
        {
            m_pos = (distance > 0) ? m_size : 0;
        }

        return (intptr_t)m_ptr + m_pos;
    }
    else
    {
        LARGE_INTEGER li;
        li.QuadPart = distance;
        ::SetFilePointerEx(m_handle, li, &li, (DWORD)origin);

        m_pos = li.QuadPart;
        return m_pos;
    }
}

//---------------------------------------------------------------------------//

// ファイルを終端する
inline bool tapetums::File::SetEndOfFile()
{
    if ( m_handle == INVALID_HANDLE_VALUE )
    {
        return true;
    }
    else
    {
        return ::SetEndOfFile(m_handle) ? true : false;
    }
}

//---------------------------------------------------------------------------//

// ファイルもしくはメモリマップトファイルをフラッシュする
inline bool tapetums::File::Flush
(
    size_t dwNumberOfBytesToFlush
)
{
    if ( m_ptr )
    {
        return ::FlushViewOfFile(m_ptr, dwNumberOfBytesToFlush) ? true : false;
    }
    else
    {
        return ::FlushFileBuffers(m_handle) ? true : false;
    }
}

//---------------------------------------------------------------------------//

// File.hpp