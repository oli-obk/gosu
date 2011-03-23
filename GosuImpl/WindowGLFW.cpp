#ifndef NOMINMAX
#define NOMINMAX
#endif // RAGE >_<

#include <Gosu/Window.hpp>
#include <Gosu/WinUtility.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <boost/bind.hpp>
#include <cassert>
#include <stdexcept>
#include <vector>

#include <GL/glfw.h>
// TODO: Put fullscreen logic in different file, track fullscreen state and
// enable dynamic toggling between fullscreen and window.


namespace
{
    Gosu::TextInput* textInputSingleton(Gosu::TextInput* newTextInput = 0)
    {
        static Gosu::TextInput* textInput = 0;

        if(newTextInput != 0)
            textInput = newTextInput;

        return textInput;
    }

    void GLFWCALL wndProcHook(UINT message, WPARAM wparam, LPARAM lparam)
    {
        if (textInputSingleton() && textInputSingleton()->feedMessage(message, wparam, lparam))
            return;
    }
}

namespace Gosu
{
    unsigned screenWidth()
    {
        return GetSystemMetrics(SM_CXSCREEN);
    }
    
    unsigned screenHeight()
    {
        return GetSystemMetrics(SM_CYSCREEN);
    }
}

struct Gosu::Window::Impl
{
    HWND handle;
	HDC hdc;
    boost::scoped_ptr<Graphics> graphics;
    boost::scoped_ptr<Input> input;
	double updateInterval;
    bool iconified;
    bool closed;

    unsigned originalWidth, originalHeight;

    Impl()
    : handle(0), hdc(0), iconified(false), closed(false)
    {
    }

    ~Impl()
    {
        /*if (hdc)
            ReleaseDC(handle, hdc);
        if (handle)
            DestroyWindow(handle);*/
    }
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen,
    double updateInterval)
: pimpl(new Impl)
{
    pimpl->originalWidth = width;
    pimpl->originalHeight = height;

    glfwInit();

    int mode = GLFW_WINDOW;
    if(fullscreen)
    {
        mode = GLFW_FULLSCREEN;
    }

    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

    if(!glfwOpenWindow(width, height, 0, 0, 0, 0, 0, 0, mode))
    {
        // Todo: error handling
    }

    pimpl->handle = glfwGetWindowHandle();
    Win::check(pimpl->handle);

    this->setCaption(L"");

    pimpl->hdc = glfwGetDC();
    Win::check(pimpl->hdc);

    // Setup VSync
    glfwSwapInterval(1);

    SetLastError(0);
    SetWindowLongPtr(handle(), GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(this));
    if (GetLastError() != 0)
        Win::throwLastError("setting the window's GWLP_USERDATA pointer");

    // Center the window when not in fullscreen
    if(!fullscreen)
    {
        /*HWND desktopWindow = GetDesktopWindow();
        RECT desktopRect;
        GetClientRect(desktopWindow, &desktopRect);
        int desktopW = desktopRect.right - desktopRect.left;
        int desktopH = desktopRect.bottom - desktopRect.top;*/
        int windowX = (screenWidth() - width) / 2;
        int windowY = (screenHeight() - height) / 2;
        glfwSetWindowPos(windowX, windowY);
    }

    pimpl->graphics.reset(new Gosu::Graphics(width, height, fullscreen));
    graphics().setResolution(pimpl->originalWidth, pimpl->originalHeight);
    pimpl->input.reset(new Gosu::Input(handle()));
    input().setMouseFactors(1.0 * pimpl->originalWidth / width, 1.0 * pimpl->originalHeight / height);
    input().onButtonDown = boost::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = boost::bind(&Window::buttonUp, this, _1);


    textInputSingleton(pimpl->input->textInput());
    glfwSetWndProcHook(wndProcHook);

    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
    wglMakeCurrent(0, 0);
}

std::wstring Gosu::Window::caption() const
{
    int bufLen = GetWindowTextLength(handle()) + 1;

    if (bufLen < 2)
        return L"";

    std::vector<TCHAR> buf(bufLen);
    GetWindowText(handle(), &buf.front(), bufLen);
    return &buf.front();
}

void Gosu::Window::setCaption(const std::wstring& value)
{
    SetWindowText(handle(), value.c_str());
}

double Gosu::Window::updateInterval() const
{
    return pimpl->updateInterval;
}

namespace GosusDarkSide
{
    // TODO: Find a way for this to fit into Gosu's design.
    // This can point to a function that wants to be called every
    // frame, e.g. rb_thread_schedule.
    typedef void (*HookOfHorror)();
    HookOfHorror oncePerTick = 0;
}

void Gosu::Window::show()
{
    ShowWindow(handle(), SW_SHOW);
    UpdateWindow(handle());
    try
    {
        unsigned lastTick = 0;

        while(!pimpl->closed)
        {
            /*if (!::IsWindowVisible(handle()))
            {
                // TODO: Find out what the Sleep here is doing...
                Sleep(50);
                return;
            }*/

            unsigned ms = milliseconds();

            if (ms < lastTick || ms - lastTick >= static_cast<unsigned>(pimpl->updateInterval))
            {
                lastTick = ms;
                input().update();
                // TODO: Bad heuristic -- this causes flickering cursor on right and bottom border of the
                // window.
                if (input().mouseX() >= 0 && input().mouseY() >= 0)
                    SendMessage(handle(), WM_SETCURSOR, reinterpret_cast<WPARAM>(handle()), HTCLIENT);
                update();

                if (needsRedraw())
                {
                    if (pimpl->graphics && graphics().begin())
                    {
                        try
                        {
                            draw();
                        }
                        catch (...)
                        {
                            graphics().end();
                            throw;
                        }
                        graphics().end();
                    }
                    
                    glfwSwapBuffers();
                }

                // There probably should be a proper "oncePerTick" handler
                // system in the future. Right now, this is necessary to give
                // timeslices to Ruby's green threads in Ruby/Gosu.
                if (GosusDarkSide::oncePerTick) GosusDarkSide::oncePerTick();
            }
            else if (pimpl->updateInterval - (ms - lastTick) > 5)
                // More than 5 ms left until next update: Sleep to reduce
                // processur usage, Sleep() is accurate enough for that.
                Sleep(5);
        }
    }
    catch (...)
    {
        close();
        throw;
    }
}

void Gosu::Window::close()
{
    ShowWindow(handle(), SW_HIDE);

    pimpl->closed = true;
    glfwTerminate();
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *pimpl->graphics;
}

const Gosu::Input& Gosu::Window::input() const
{
    return *pimpl->input;
}

Gosu::Input& Gosu::Window::input()
{
    return *pimpl->input;
}

HWND Gosu::Window::handle() const
{
    return pimpl->handle;
}

LRESULT Gosu::Window::handleMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
/*
	if (message == WM_SETCURSOR)
    {
		if (LOWORD(lparam) != HTCLIENT || GetForegroundWindow() != handle() || needsCursor())
		{
			static const HCURSOR arrowCursor = LoadCursor(0, IDC_ARROW);
			SetCursor(arrowCursor);
	    }
		else
			SetCursor(NULL);
        return TRUE;
    }

    if (message == WM_SETFOCUS && graphics().fullscreen() && IsWindowVisible(pimpl->handle))
    {
        if (pimpl->iconified)
        {
            OpenIcon(pimpl->handle);
            int w = graphics().width(), h = graphics().height(), bpp = 32, rr = 60;
            setVideoMode(findClosestVideoMode(&w, &h, &bpp, &rr));
            pimpl->iconified = false;
        }
        return 0;
    }

    if (message == WM_KILLFOCUS && graphics().fullscreen() && IsWindowVisible(pimpl->handle))
    {
        if (!pimpl->iconified)
        {
            ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
            CloseWindow(pimpl->handle);
            pimpl->iconified = true;
        }
        return 0;
    }

    if (message == WM_CLOSE)
    {
        close();
        return 0;
    }

    if (message == WM_PAINT)
    {
        if (pimpl->graphics && graphics().begin())
        {
            try
            {
                draw();
            }
            catch (...)
            {
                graphics().end();
                throw;
            }
            graphics().end();
        }
        SwapBuffers(pimpl->hdc);
        ValidateRect(handle(), 0);
        return 0;
    }

    if (message == WM_SYSCOMMAND)
    {
        switch(wparam)
        {
            case SC_SCREENSAVE:
            case SC_MONITORPOWER:
                if (graphics().fullscreen())
                    return 0;
                else
                    break;
            case SC_KEYMENU:
                return 0;
        }
    }

    if (pimpl->input && input().textInput() && input().textInput()->feedMessage(message, wparam, lparam))
        return 0;

    return DefWindowProc(handle(), message, wparam, lparam);*/
    return 0;
}

// Deprecated.

class Gosu::Audio {};
namespace { Gosu::Audio dummyAudio; }

const Gosu::Audio& Gosu::Window::audio() const
{
    return dummyAudio;
}
 
Gosu::Audio& Gosu::Window::audio()
{
    return dummyAudio;
}
