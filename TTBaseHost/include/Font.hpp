#pragma once

//---------------------------------------------------------------------------//
//
// Font.hpp
//  フォントの作成および破棄
//   Copyright (C) 2014-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//

namespace tapetums 
{
    class Font;
}

//---------------------------------------------------------------------------//

class tapetums::Font
{
private:
    INT32   size   { 0 };
    INT32   weight { 0 };
    HFONT   font   { nullptr };
    LPCTSTR name   { nullptr };

public:
    Font() noexcept = default;
    ~Font() { free(); }

    Font(const Font& lhs) { create(lhs.size, lhs.name, lhs.weight); }
    Font& operator =(const Font& lhs) { create(lhs.size, lhs.name, lhs.weight); return *this; }

    Font(Font&&) noexcept = default;
    Font& operator =(Font&&) noexcept = default;

    Font(INT32 font_size, LPCTSTR font_name, INT32 cWeight = FW_REGULAR) { create(font_size, font_name, cWeight); }

public:
    operator HFONT() { return font; }

public:
    HFONT create
    (
        INT32 font_size, LPCTSTR font_name, INT32 cWeight = FW_REGULAR
    )
    {
        font = ::CreateFont
        (
            font_size, 0, 0, 0,
            cWeight, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
            font_name
        );

        if ( font )
        {
            size   = font_size;
            name   = font_name;
            weight = cWeight;
        }

        return font;
    }

    void free()
    {
        if ( font ) { ::DeleteObject(font); font = nullptr; }
    }
};

//---------------------------------------------------------------------------//

// Font.hpp