#pragma once

//---------------------------------------------------------------------------//
//
// Application.hpp
//  メッセージループをカプセル化するクラス
//   Copyright (C) 2013-2016 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//

namespace tapetums
{
    class Application;
}

//---------------------------------------------------------------------------//
// Class
//---------------------------------------------------------------------------//

// メッセージループをカプセル化するクラス
class tapetums::Application
{
public:
    Application()  = delete;
    ~Application() = delete;

    Application(const Application&)             = delete;
    Application& operator= (const Application&) = delete;

    Application(Application&&)             noexcept = delete;
    Application& operator= (Application&&) noexcept = delete;

public:
    static DWORD ThreadId() noexcept { return threadId(); }
    static bool  IsLoop()   noexcept { return loop(); }

public:
    static INT32 Run();
    static bool  Exit();
    static void  Pause();
    static void  Resume();

    template<typename Updater>
    static INT32 Run(Updater& update);

private:
    static DWORD& threadId() noexcept { static DWORD threadId { 0 };    return threadId; }
    static bool&  loop()     noexcept { static bool  loop     { true }; return loop; }
};

//---------------------------------------------------------------------------//
// Methods
//---------------------------------------------------------------------------//

// メッセージループ
inline INT32 tapetums::Application::Run()
{
    threadId() = ::GetCurrentThreadId();

    MSG msg { };

    while ( ::GetMessage(&msg, nullptr, 0, 0) > 0 )
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    return static_cast<INT32>(msg.wParam);
}

//---------------------------------------------------------------------------//

// ゲームループ
template<typename Updater>
inline INT32 tapetums::Application::Run(Updater& update)
{
    threadId() = ::GetCurrentThreadId();

    MSG msg { };

    while ( true )
    {
        if ( ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) )
        {
            if ( msg.message == WM_QUIT ) { break; }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else if ( loop() )
        {
            update(); // ファンクタの呼び出し
        }
        else
        {
            ::MsgWaitForMultipleObjects(0, nullptr, FALSE, INFINITE, QS_ALLINPUT);
        }
    }

    return static_cast<INT32>(msg.wParam);
}

//---------------------------------------------------------------------------//

// メッセージループの終了
inline bool tapetums::Application::Exit()
{
    return ::PostThreadMessage(threadId(), WM_QUIT, 0, 0) ? true : false;
}

//---------------------------------------------------------------------------//

// ゲームループを停止
inline void tapetums::Application::Pause()
{
    loop() = false;
}

//---------------------------------------------------------------------------//

// ゲームループを再開
inline void tapetums::Application::Resume()
{
    loop() = true;
}

//---------------------------------------------------------------------------//

// Application.hpp