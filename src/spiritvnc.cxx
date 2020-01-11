/*
 * spiritvnc.cxx - part of SpiritVNC - FLTK
 * 2016-2020 Will Brokenbourgh https://www.pismotek.com/brainout/
 *
 * To God be the glory!  In Jesus name! :-D
 *
 * I don't use 'shortcuts' when checking for NULL or 0 because I want
 * new programmers to better understand what's going on in the code
 *
 * Some code comes from examples provided by the libssh2 and
 * libvncserver projects
 *
 */

/*
 * (C) Will Brokenbourgh
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_PNG_Image.H>
#include "app.h"
#include "consts_enums.h"


AppVars* app = new AppVars();


/* main program */
int main (int argc, char **argv)
{
    // tells FLTK we're a multithreaded app
    Fl::lock();

    // set graphics / display options
    Fl::visual(FL_RGB);
    Fl::visual(FL_DOUBLE | FL_INDEX);

    // create program UI
    svCreateGUI();

    // read config file, set app options and populate host list
    svConfigReadCreateHostList();

    // set or unset main window tooltips to user preference
    svSetUnsetMainWindowTooltips();

    // add default empty host list item if no items added from config file
    if (app->hostList->size() == 0)
    {
        svInsertEmptyItem();
        app->hostList->size(170, 0);
    }

    // set app icons
    svCreateAppIcons();

    // manually trigger misc events callback
    svPositionWidgets();

    app->mainWin->end();
    app->mainWin->show(argc, argv);

    Fl::focus(app->hostList);
    app->hostList->take_focus();

    // ignore SIGPIPE from libvncclient socket calls
    signal(SIGPIPE, SIG_IGN);

    // start up the connection 'supervisor' timer callback
    // do NOT change the interval of this timer
    // because program logic expects this to always be
    // near 1 second
    Fl::add_timeout(SV_ONE_SECOND, svConnectionWatcher);

    // start watching the clipboard
    Fl::add_clipboard_notify(svHandleLocalClipboard);

    svResizeScroller();

    // restore main window saved position and size on
    // Mac OS X / macOS immediately
    #ifdef __APPLE__
    svRestoreWindowSizePosition(NULL);
    #endif

    Fl::redraw();
    Fl::wait();

    // restore main window saved postition and size
    //  *don't* do delayed repositioning on Mac OS X / macOS
    #ifndef __APPLE__
    // x11 window managers usually need delayed repositioning
    Fl::add_timeout(0.5, svRestoreWindowSizePosition);
    #endif

    VncObject::masterMessageLoop(NULL);

    return Fl::run();
}