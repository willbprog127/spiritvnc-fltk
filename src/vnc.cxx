/*
 * vnc.cxx - part of SpiritVNC - FLTK
 * 2016-2020 Will Brokenbourgh https://www.pismotek.com/brainout/
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


#include "app.h"
#include "consts_enums.h"
#include "vnc.h"


/* create a listening vnc obect */
void VncObject::createVNCListener ()
{
    HostItem * itm = new HostItem();
    if (itm == NULL)
        return;
    itm->name = "Listening";
    itm->scaling = 'f';
    itm->showRemoteCursor = true;
    itm->isListener = true;

    // set host list status icon
    itm->icon = app->iconDisconnected;

    app->hostList->add("Listening", itm);
    app->hostList->icon(app->hostList->size(), itm->icon);
    app->hostList->bottomline(app->hostList->size());

    app->hostList->redraw();

    VncObject::createVNCObject(itm); //, true);
}


/* initializes and attempts connection with
 * libvnc client / VncObject object
 * (static method)
 */
void VncObject::createVNCObject (HostItem * itm) //, bool listen)
{
    // if itm is null or our viewer is already created, return
    if (itm == NULL
        || itm->isConnected == true
        || itm->isConnecting == true)
        return;

    // if the host type is 'v' or 's', create vnc viewer
    if (itm->hostType == 'v' || itm->hostType == 's')
    {
        // create new vnc viewer
        itm->vnc = new VncObject();
        VncObject * vnc = itm->vnc;
        vnc->itm = itm;

        if (itm->vnc == NULL)
        {
            svMessageWindow("Error! Could not create new VncObject!", "SpiritVNC - FLTK");
            return;
        }

        // reset itm state flags
        itm->isConnecting = true;
        itm->isConnected = false;
        itm->isWaitingForShow = false;
        itm->hasCouldntConnect = false;
        itm->hasError = false;
        itm->hasDisconnectRequest = false;
        itm->hasAlreadyShown = false;
        itm->hasEnded = false;

        // store this viewer pointer in libvnc client data
        rfbClientSetClientData(vnc->vncClient, app->libVncVncPointer, vnc);

        // set up remote / local cursor
        if (itm->showRemoteCursor == false)
        {
            vnc->vncClient->appData.useRemoteCursor = false;
            vnc->vncClient->GotCursorShape = NULL;
        }
        else
        {
            vnc->vncClient->appData.useRemoteCursor = true;
            vnc->vncClient->GotCursorShape = VncObject::handleCursorShapeChange;
        }

        // set up vnc compression and quality levels
        vnc->vncClient->appData.compressLevel = itm->compressLevel;
        vnc->vncClient->appData.qualityLevel = itm->qualityLevel;

        vnc->vncClient->appData.encodingsString = strdup("tight copyrect hextile");

        itm->vncAddressAndPort = itm->hostAddress + ":" + itm->vncPort;

        // add to viewers waiting ref count
        app->nViewersWaiting ++;

        svLogToFile(std::string(std::string("SpiritVNC - Attempting to connect to ")
            + itm->name + " - " + itm->hostAddress).c_str());

        // set host list item status icon
        itm->icon = app->iconConnecting;
        Fl::awake(svHandleListItemIconChange);

        // ############  SSH CONNECTION ###############################################
        // we connect to this host with vnc through ssh
        if (itm->hostType == 's')
        {
            svDebugLog("svCreateVNCObject - Host is 'SVNC'");

            struct stat structStat;

            // oops, can't open ssh key file
            if (stat(itm->sshKeyPublic.c_str(), &structStat) != 0
                || stat(itm->sshKeyPrivate.c_str(), &structStat) != 0)
            {
                itm->isConnecting = false;
                itm->hasCouldntConnect = true;
                itm->hasError = true;

                svLogToFile("SpiritVNC ERROR - Could not open the public or private"
                    " SSH key file");
                svMessageWindow(std::string("Could not open the public or private SSH key") +
                    std::string(" file for ") + itm->name + itm->hostAddress);

                if (vnc != NULL && vnc->vncClient != NULL)
                    VncObject::endAndDeleteViewer(vnc);

                svHandleThreadConnection(itm);

                return;
            }

            itm->sshLocalPort = svFindFreeTcpPort();

            char strVNCAddressAndPort[50] = {0};

            snprintf(strVNCAddressAndPort, 50, "127.0.0.1:%i", itm->sshLocalPort);
            itm->vncAddressAndPort = strVNCAddressAndPort;

            svDebugLog("svCreateVNCObject - Creating and running threadSSH");

            // create, launch and detach call to create our ssh connection
            int sshResult = pthread_create(&itm->threadSSH, NULL, svCreateSSHConnection, itm);

            if (sshResult != 0)
            {
                std::cout << "SpiritVNC - FLTK: Couldn't create SSH thread  Error: "
                    << sshResult << std::endl;
                itm->isConnecting = false;
                itm->hasCouldntConnect = true;
                itm->hasError = true;

                if (vnc != NULL && vnc->vncClient != NULL)
                    VncObject::endAndDeleteViewer(vnc);

                svHandleThreadConnection(itm);

                return;
            }

            time_t timeOut = time(NULL) + (SV_CONNECTION_TIMEOUT_SECS - 1);

            svDebugLog("svCreateVNCObject - About to enter itm->sshReady timer loop");

            // loop until the ssh connection is ready
            // or exit if ssh times out
            while (itm->sshReady == false)
            {
                if (time(NULL) >= timeOut)
                {
                    itm->isConnecting = false;
                    itm->hasCouldntConnect = true;

                    svHandleThreadConnection(itm);

                    return;
                }

                Fl::check();
            }
        }
        // ############  SSH CONNECTION END ###########################################

        svDebugLog("svCreateVNCObject - Creating and running itm->threadRFB");

        // create, launch and detach call to create our vnc connection
        int rfbResult = pthread_create(&itm->threadRFB, NULL, VncObject::initVNCConnection, itm);

        if (rfbResult != 0)
        {
            std::cout << "SpiritVNC - FLTK: Couldn't create RFB thread  Error: " << rfbResult;
            itm->isConnecting = false;
            itm->hasCouldntConnect = true;
            itm->hasError = true;

            if (vnc != NULL && vnc->vncClient != NULL)
                VncObject::endAndDeleteViewer(vnc);

            svHandleThreadConnection(itm);

            return;
        }
    }

    return;
}


/*
 * ends all vnc objects (usually called right before program quits)
 * (static method)
 */
void VncObject::endAllViewers ()
{
    HostItem * itm = NULL;
    VncObject * vnc = NULL;

    for (int i = 0; i <= app->hostList->size(); i ++)
    {
        itm = static_cast<HostItem *>(app->hostList->data(i));

        if (itm != NULL)
        {
            vnc = itm->vnc;
            if (vnc != NULL
                && (itm->isConnected == true
                || itm->isConnecting == true
                || itm->isWaitingForShow == true))
            {
                itm->hasDisconnectRequest = true;

                VncObject::endAndDeleteViewer(vnc);
            }

            vnc = NULL;
        }
    }
}


/* set vnc object to disconnect and stop working */
/* (instance method) */
void VncObject::endViewer ()
{
    //this->GONK!

    if (itm != NULL && itm->vnc != NULL)
    {
        // only hide main viewer if this is the currently-displayed itm
        if (app->vncViewer->vnc != NULL && itm == app->vncViewer->vnc->itm)
            hideMainViewer();

        // host disconnected unexpectedly / interrupted connection
        if (itm->isConnected == true && itm->hasDisconnectRequest == false)
        {
            if (itm->isListener == false)
            {
                itm->icon = app->iconDisconnectedError;
                Fl::awake(svHandleListItemIconChange);
            }

            svLogToFile(std::string("SpiritVNC - Unexpectedly disconnected from ")
                + itm->name + std::string(" - ") + itm->hostAddress);
        }

        // we disconnected purposely from host
        if ((itm->isConnected == true || itm->isConnecting == true)
            && itm->hasDisconnectRequest == true)
        {
            if (itm->isListener == false)
            {
                // set host list item status icon
                itm->icon = app->iconDisconnected;
                Fl::awake(svHandleListItemIconChange);
            }

            if (app->shuttingDown)
            {
                svLogToFile(std::string("SpiritVNC - Automatically "
                    "disconnecting because program is shutting down ")
                    + itm->name + std::string(" - ") + itm->hostAddress);
            }
            else
            {
               svLogToFile(std::string("SpiritVNC - Manually disconnected from ")
                    + itm->name + std::string(" - ") + itm->hostAddress);
            }
        }

        // decrement our connecteditems count
        app->connectedItems --;

        // stop this viewer's connection worker thread
        if (itm->threadRFBRunning == true)
        {
            // make sure there's a valid thread before canceling
            if (itm->threadRFB != 0)
                pthread_cancel(itm->threadRFB);

            itm->threadRFBRunning = false;
        }

        // tell ssh to clean up if a ssh/vnc connection
        if (itm->hostType == 's')
        {
            itm->stopSSH = true;
            itm->sshReady = false;
        }

        itm->isConnected = false;
        itm->isConnecting = false;
        itm->hasDisconnectRequest = false;
        itm->hasEnded = true;

        // clean up the client
        rfbClientCleanup(vncClient);
    }
}


/* calls endViewer and cleans up associated VncObject * memory */
/* (static method) */
void VncObject::endAndDeleteViewer (VncObject * vnc)
{
    if (vnc == NULL)
        return;

    vnc->endViewer();

    delete vnc;
    vnc = NULL;
}


/* checks to see if vnc client will fit within scroller */
/* (instance method) */
bool VncObject::fitsScroller ()
{
    const HostItem * itm = static_cast<HostItem *>(this->itm);
    if (itm == NULL || itm->vnc == NULL)
        return false;

    const rfbClient * cl = itm->vnc->vncClient;
    if (cl == NULL)
        return false;

    if (cl->width <= app->scroller->w() && cl->height <= app->scroller->h())
        return true;
    else
        return false;
}


/* handle cursor change */
/* (static method / callback) */
void VncObject::handleCursorShapeChange (rfbClient * cl, int xHot, int yHot, int nWidth,
    int nHeight, int nBytesPerPixel)
{
    VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

    if (vnc == NULL ||
        cl == NULL ||
        Fl::belowmouse() != app->vncViewer ||
        vnc->allowDrawing == false)
        return;

    const int nSSize = nWidth * nHeight * nBytesPerPixel;

    // local image buffer
    uchar strS[nSSize];

    // copy libvnc cursor image data to local image buffer
    memcpy(strS, cl->rcSource, nSSize);

    // if image has alpha, apply mask
    if (nBytesPerPixel == 2 || nBytesPerPixel == 4)
    {
        int nM = 0;
        int nMV = 0;

        for (int i = (nBytesPerPixel - 1); i < nSSize; i += nBytesPerPixel)
        {
            nMV = cl->rcMask[nM];
            if (nMV > 0)
                nMV = 255;
            strS[i] = nMV;
            nM ++;
        }
    }

    // create rgb image from raw data
    Fl_RGB_Image * img = new Fl_RGB_Image(strS, nWidth, nHeight, nBytesPerPixel);

    if (img == NULL)
        return;

    // delete previous copy, if any
    if (vnc->imgCursor != NULL)
        delete vnc->imgCursor;

    // copy rgb image to vncViewer and set x+y hotspots
    vnc->imgCursor = static_cast<Fl_RGB_Image *>(img->copy());

    if (vnc->imgCursor == NULL)
        return;

    vnc->nCursorXHot = xHot;
    vnc->nCursorYHot = yHot;

    svHandleThreadCursorChange(NULL);

    if (img != NULL)
        delete img;
}


/* (static method) */
void VncObject::handleFrameBufferUpdate (rfbClient * cl) //, int x, int y, int w, int h)
{
    VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

    if (vnc == NULL ||
        vnc->allowDrawing == false)
            return;

    app->vncViewer->redraw();
}


/* handle copy/cut FROM vnc host */
/* (static method) */
void VncObject::handleRemoteClipboardProc (rfbClient * cl, const char * text, int textlen)
{
    VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

    if (vnc == NULL)
        return;

    VncObject * vnc2 = app->vncViewer->vnc;

    if (vnc2 == NULL)
        return;

    if (vnc2 == vnc && text != NULL && textlen > 0)
    {
        app->blockLocalClipboardHandling = true;

        Fl::copy(text, textlen, 1);

        SendFramebufferUpdateRequest(cl, 0, 0, cl->width, cl->height, false);

        app->blockLocalClipboardHandling = false;

        app->vncViewer->redraw();
        Fl::awake();
    }
}


/* set main VncViewer object to hide itself */
/* (static method) */
void VncObject::hideMainViewer ()
{
    VncObject * vnc = app->vncViewer->vnc;

    if (vnc == NULL)
        return;

    vnc->allowDrawing = false;

    Fl::lock();
    app->vncViewer->vnc = NULL;
    app->vncViewer->size(0, 0);
    app->scroller->scroll_to(0, 0);
    app->scroller->type(0);
    app->scroller->redraw();
    Fl::unlock();
}


/* initialize and connect to a vnc host/server */
/* (this is called as a thread because it blocks) */
void * VncObject::initVNCConnection (void * data)
{
    // detach this thread
    pthread_detach(pthread_self());

    char * strParams[2] = {NULL, NULL};

    Fl::lock();
    HostItem * itm = static_cast<HostItem *>(data);
    Fl::unlock();

    if (itm == NULL)
    {
        itm->isConnected = false;
        itm->isConnecting = false;
        itm->hasError = true;

        Fl::awake(svHandleThreadConnection, itm);

        return SV_RET_VOID;
    }

    itm->threadRFBRunning = true;

    int nNumOfParams = 2;
    VncObject * vnc = itm->vnc;

    if (vnc == NULL)
    {
        itm->isConnected = false;
        itm->isConnecting = false;
        itm->hasError = true;
        itm->threadRFBRunning = false;

        Fl::awake(svHandleThreadConnection, itm);

        return SV_RET_VOID;
    }

    char strP[1024] = {0};

    // 'program name' parameter
    strParams[0] = strdup("SpiritVNCFLTK");

    if (itm->isListener == false)
    {
        strncpy(strP, itm->vncAddressAndPort.c_str(), 1023);
        strParams[1] = strdup(strP);
    }
    else
    {
        strncpy(strP, "-listennofork", 1023);
        vnc->vncClient->listenAddress = strdup("0.0.0.0");
        strParams[1] = strdup(strP);
    }

    // if the second parameter is invalid, get out
    if (strParams[1] == NULL || strlen(strParams[1]) < 7)
    {
        itm->isConnected = false;
        itm->isConnecting = false;
        itm->hasError = true;
        itm->threadRFBRunning = false;

        Fl::awake(svHandleThreadConnection, itm);

        return SV_RET_VOID;
    }

    // libvnc - attempt to connect to host
    // this function blocks, that's why this function runs as a thread
    if (rfbInitClient(vnc->vncClient, &nNumOfParams, strParams) == false)
    {
        perror("SpiritVNC - rfbInitClient");
        itm->isConnected = false;
        itm->isConnecting = false;
        itm->hasCouldntConnect = true;
        itm->threadRFBRunning = false;

        Fl::awake(svHandleThreadConnection, itm);

        return SV_RET_VOID;
    }

    itm->isConnected = true;
    itm->isConnecting = false;
    itm->isWaitingForShow = true;
    itm->threadRFBRunning = false;

    // send message to main thread
    Fl::awake(svHandleThreadConnection, itm);

    return SV_RET_VOID;
}


/* libvnc logging callback */
/* (static function) */
void VncObject::libVncLogging (const char * format, ...)
{
    va_list args;
    char timeBuf[50] = {0};
    char msgBuf[FL_PATH_MAX] = {0};
    time_t logClock;

    // build time-stamp
    time(&logClock);
    strftime(timeBuf, 50, "%Y-%m-%d %X ", localtime(&logClock));

    // combine format string and args into nowBuff
    va_start(args, format);
    vsnprintf(msgBuf, FL_PATH_MAX, format, args);
    va_end(args);

    // analyze messages in case user needs to be alerted
    svParseLogMessages(timeBuf, msgBuf);

    // only write libvncserver msgs if in verbose mode
    if (app->verboseLogging == true)
    {
        std::ofstream ofs;
        char logFileName[FL_PATH_MAX] = {0};

        snprintf(logFileName, FL_PATH_MAX, "%s/spiritvnc-fltk.log", app->configPath.c_str());

        ofs.open(logFileName, std::ofstream::out | std::ofstream::app);

        // oops, can't open log file
        if (ofs.fail())
        {
            std::cout << "SpiritVNC ERROR - Could not open log file for writing" <<
                std::endl << std::flush;
            return;
        }

        // write time-stamp to log
        ofs << timeBuf << "- " << msgBuf << std::endl;
        ofs.close();
   }
}


/* master loop to handle all vnc objects' message checking */
/* (static function) */
void VncObject::masterMessageLoop (void * notUsed)
{
    HostItem * itm = NULL;
    VncObject * vnc = NULL;

    int checkDelay = 0;

    while (app->shuttingDown == false)
    {
        // iterate through the host list and check each for an active vnc object
        for (int i = 0; i <= app->hostList->size(); i ++)
        {
            Fl::lock();
            itm = static_cast<HostItem *>(app->hostList->data(i));
            Fl::unlock();

            if (itm == NULL)
                continue;

            vnc = itm->vnc;

            if (vnc == NULL)
                continue;

            // if this vnc object is connected and is active, process vnc messages
            if (itm->isConnected == true
                && itm->hasEnded == false
                && app->shuttingDown == false)
                    VncObject::checkVNCMessages(vnc);

            itm = NULL;
            vnc = NULL;
        }

        checkDelay ++;

        if (checkDelay == 10)
        {
            checkDelay = 0;
            Fl::check();
        }
    }
}


/* libvnc send password to host callback */
/* (static function) */
char * VncObject::handlePassword (rfbClient * cl)
{
    if (cl == NULL)
    {
        svLogToFile("SpiritVNC ERROR - handlePassword: vnc->vncClient is null");
        return NULL;
    }

    VncObject * vnc = static_cast<VncObject *>(rfbClientGetClientData(cl, app->libVncVncPointer));

    if (vnc == NULL) {
        svLogToFile("SpiritVNC ERROR - handlePassword: vnc is null");
        return NULL;
    }

    HostItem * itm = svItmFromVnc(vnc);

    if (itm == NULL)
        return NULL;

    const char * strPass = itm->vncPassword.c_str();

    if (strPass == NULL)
        return NULL;
    else
        return strdup(strPass);
}


/* set vnc object to show itself */
/* (instance method) */
void VncObject::showViewer ()
{
    const HostItem * itm = static_cast<HostItem *>(this->itm);

    if (itm == NULL || vncClient == NULL)
        return;

    app->vncViewer->vnc = this;

    SendFramebufferUpdateRequest(vncClient, 0, 0, vncClient->width, vncClient->height, false);

    int leftMargin = (app->hostList->x() + app->hostList->w() + 3);

    // scale off / scroll if host screen is too big
    if (itm->scaling == 's' || (itm->scaling == 'f' && fitsScroller() == true))
    {
        app->scroller->type(Fl_Scroll::BOTH);
        app->vncViewer->size(vncClient->width, vncClient->height);
    }

    // scale 'zoom' or 'fit'
    if (itm->scaling == 'z' || (itm->scaling == 'f' && fitsScroller() == false))
    {
        // maximize vnc viewer size
        app->vncViewer->size(
            app->mainWin->w() - leftMargin,
            app->mainWin->h());

        // calculate host screen ratio
        float dRatio = static_cast<float>(vncClient->width) / static_cast<float>(vncClient->height);

        // set height of image to max height and check if resulting width is less than max width,
        // otherwise set width of image to max width and calculate new height
        if (static_cast<float>(app->vncViewer->h()) * dRatio <= app->vncViewer->w())
            app->vncViewer->size(static_cast<int>(static_cast<float>(app->vncViewer->h()) * dRatio),
                app->vncViewer->h());
        else
            app->vncViewer->size(app->vncViewer->w(),
                static_cast<int>(static_cast<float>(app->vncViewer->w()) / dRatio));
    }

    app->scroller->scroll_to(0, 0);

    svResizeScroller();

    // viewer centering (only if not zooming)
    if (itm->scaling == 's' || (itm->scaling == 'f' && fitsScroller() == true))
    {
        // figure out x position for viewer
        if (itm->centerX == true)
        {
            app->scroller->position((app->scroller->w() - vncClient->width) / 2 + leftMargin,
                app->scroller->y());

            if (app->scroller->x() < 0)
                app->scroller->position(leftMargin, app->scroller->y());
        }
        else
            app->scroller->position(leftMargin, app->scroller->y());

        // figure out y position for viewer
        if (itm->centerY == true)
        {
            app->scroller->position(app->scroller->x(), (app->scroller->h() - vncClient->height) / 2);

            if (app->scroller->y() < 0)
                app->scroller->position(app->scroller->x(), 0);
        }
        else
            app->scroller->position(app->scroller->x(), 0);
    }

    allowDrawing = true;

    app->scroller->redraw();
}


/* check and act on libvnc host messages */
/* (static method) */
void VncObject::checkVNCMessages (VncObject * vnc)
{
    int nMsg = 0;

    if (vnc == NULL || vnc->vncClient == NULL)
        return;

    // below modified to '0' based on select() docs suggestion
    nMsg = WaitForMessage(vnc->vncClient, 0);

    if (nMsg < 0)
    {
        VncObject::endAndDeleteViewer(vnc);
        return;
    }

    if (nMsg)
    {
        // reset inactive seconds so we don't automatically disconnect
        vnc->inactiveSeconds = 0;

        if (HandleRFBServerMessage(vnc->vncClient) == FALSE)
        {
            VncObject::endAndDeleteViewer(vnc);
            return;
        }
        else
            Fl::awake();
    }
}


/*
 * ########################################################################################
 * ########################## VNC VIEWER ##################################################
 * ########################################################################################
 */
/* draw event for vnc view widget */
/* (instance method) */
void VncViewer::draw ()
{
    VncObject * vnc = app->vncViewer->vnc;

    if (vnc == NULL ||
        vnc->allowDrawing == false ||
        vnc->vncClient == NULL)
        return;

    rfbClient * cl = vnc->vncClient;
    HostItem * itm = static_cast<HostItem *>(vnc->itm);

    if (cl == NULL || itm == NULL || cl->frameBuffer == NULL)
        return;

    int nBytesPerPixel = cl->format.bitsPerPixel / 8;

    // get out if client or scroller size is wrong
    if (cl->width < 1 || cl->height < 1 || app->scroller->w() < 1 || app->scroller->h() < 1)
        return;

    // 's'croll or 'f'it + real size scale mode geometry
    if (itm->scaling == 's' || (itm->scaling == 'f' && vnc->fitsScroller() == true))
    {
        // draw that vnc host!
        fl_draw_image(
            cl->frameBuffer,
            app->scroller->x() - app->scroller->xposition(),
            app->scroller->y() - app->scroller->yposition(),
            cl->width,
            cl->height,
            nBytesPerPixel,
            0);

        return;
    }

    // 'z'oom or 'f'it + oversized scale mode geometry
    if (itm->scaling == 'z' || (itm->scaling == 'f' && vnc->fitsScroller() == false))
    {
        Fl_Image * imgC = NULL;
        Fl_RGB_Image * imgZ = NULL;

        int isize = cl->width * cl->height * nBytesPerPixel;

        // if there's an alpha byte, set it to 255
		if (nBytesPerPixel == 2 || nBytesPerPixel == 4)
        {
            for (int i = (nBytesPerPixel - 1); i < isize; i+= nBytesPerPixel)
                cl->frameBuffer[i] = 255;
        }

		imgZ = new Fl_RGB_Image(cl->frameBuffer, cl->width, cl->height, nBytesPerPixel);

        if (imgZ != NULL)
        {
            // set appropriate scale quality
            if (itm->scalingFast == true)
                imgZ->RGB_scaling(FL_RGB_SCALING_NEAREST);
            else
                imgZ->RGB_scaling(FL_RGB_SCALING_BILINEAR);

            // scale down imgZ image to imgC
            imgC = imgZ->copy(app->vncViewer->w(), app->vncViewer->h());

            // draw scaled image onto screen
            if (imgC != NULL)
                imgC->draw(app->vncViewer->x(), app->vncViewer->y());
        }

        if (imgC != NULL)
            delete imgC;

        if (imgZ != NULL)
            delete imgZ;
    }
}


/* handle events for vnc view widget */
/* (instance method) */
int VncViewer::handle (int event)
{
    if (app->vncViewer == NULL)
        return 0;

    VncObject * vnc = app->vncViewer->vnc;

    if (vnc == NULL)
        return 0;

    // bail out if this is not the active vnc object
    if (vnc->allowDrawing == false)
        return 0;

    float nMouseX = 0;
    float nMouseY = 0;
    static int nButtonMask;
    HostItem * itm = vnc->itm;

    // itm is null
    if (itm == NULL)
        return 0;

    rfbClient * cl = vnc->vncClient;

    // client is null
    if (cl == NULL)
        return 0;

    // scrolled / non-scaled sizing
    if (itm->scaling == 's' || (itm->scaling == 'f' && vnc->fitsScroller() == true))
    {
        nMouseX = Fl::event_x() - app->scroller->x() + app->scroller->xposition();
        nMouseY = Fl::event_y() - app->scroller->y() + app->scroller->yposition();
    }

    // scaled sizing
    if (itm->scaling == 'z' || (itm->scaling == 'f' && vnc->fitsScroller() == false))
    {
        float fXAdj = float(app->vncViewer->w()) / float(cl->width);
        float fYAdj = float(app->vncViewer->h()) / float(cl->height);

        nMouseX = float(Fl::event_x() - app->scroller->x()) / fXAdj;
        nMouseY = float(Fl::event_y() - app->scroller->y()) / fYAdj;
    }

    switch (event)
    {
        // ** mouse events **
        case FL_DRAG:
            if (Fl::event_button() == FL_LEFT_MOUSE)
                nButtonMask |= rfbButton1Mask;

            if (Fl::event_button() == FL_RIGHT_MOUSE)
                nButtonMask |= rfbButton3Mask;

            SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
            SendIncrementalFramebufferUpdateRequest(vnc->vncClient);

            app->scanIsRunning = false;
            return 1;
            break;
        case FL_PUSH:
            // left mouse button
            if (Fl::event_button() == FL_LEFT_MOUSE)
            {
                nButtonMask |= rfbButton1Mask;
                SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
                app->scanIsRunning = false;
                return 1;
            }
            // right mouse button
            if (Fl::event_button() == FL_RIGHT_MOUSE)
            {
                nButtonMask |= rfbButton3Mask;
                SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
                app->scanIsRunning = false;
                return 1;
            }
            break;
        case FL_RELEASE:
            // left mouse button
            if (Fl::event_button() == FL_LEFT_MOUSE)
            {
                // left mouse click
                nButtonMask &= ~rfbButton1Mask;
                SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
                SendIncrementalFramebufferUpdateRequest(vnc->vncClient);
                app->scanIsRunning = false;
                return 1;
            }

            // right mouse button
            if (Fl::event_button() == FL_RIGHT_MOUSE)
            {
                nButtonMask &= ~rfbButton3Mask;
                SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
                SendIncrementalFramebufferUpdateRequest(vnc->vncClient);
                app->scanIsRunning = false;
                return 1;
            }
            break;
        case FL_MOUSEWHEEL:
        	{
            int nYWheel = Fl::event_dy();

            // handle vertical scrolling
            // (vnc protocol doesn't seem to support horizontal scrolling)
            if (nYWheel != 0)
            {
                int nYDirection = 0;

                if (nYWheel > 0)
                    nYDirection = rfbWheelDownMask;
                if (nYWheel < 0)
                    nYDirection = rfbWheelUpMask;

                nButtonMask |= nYDirection;
                SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
                nButtonMask &= ~nYDirection;
                SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
                SendIncrementalFramebufferUpdateRequest(vnc->vncClient);
                return 1;
            }
            break;
        }
        case FL_MOVE:
            SendPointerEvent(vnc->vncClient, nMouseX, nMouseY, nButtonMask);
            //SendIncrementalFramebufferUpdateRequest(vnc->vncClient);
            return 1;
            break;
        // ** keyboard events **
        case FL_KEYDOWN:
            sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), itm, cl, true);
            app->scanIsRunning = false;
            return 1;
            break;
        case FL_SHORTCUT:
            sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), itm, cl, true);
            app->scanIsRunning = false;
            return 1;
            break;
        case FL_KEYUP:
            sendCorrectedKeyEvent(Fl::event_text(), Fl::event_key(), itm, cl, false);
            app->scanIsRunning = false;
            return 1;
            break;
        // ** misc events **
        case FL_ENTER:
            if (vnc->imgCursor != NULL && itm->showRemoteCursor == true)
                svHandleThreadCursorChange(NULL);
            return 1;
            break;
        case FL_LEAVE:
            app->mainWin->cursor(FL_CURSOR_DEFAULT);
            Fl::wait();
            return 1;
            break;
        case FL_FOCUS:
            return 1;
            break;
        case FL_UNFOCUS:
            return 1;
            break;
        case FL_PASTE:
			{
				int intClipLen = Fl::event_length();
				char strClipText[intClipLen];

				if (intClipLen > 0)
				{
					strncpy(strClipText, Fl::event_text(), intClipLen);

					// send clipboard text to remote server
					SendClientCutText(app->vncViewer->vnc->vncClient,
						const_cast<char *>(strClipText), intClipLen);
				}
				return 1;
			break;
			}
        default:
            break;
    }

    return Fl_Box::handle(event);
}


/* fix / convert X11 key codes to rfb key codes */
void VncViewer::sendCorrectedKeyEvent (const char * strIn, const int nKey,
    HostItem * itm, rfbClient * cl, bool downState)
{
    int nK = Fl::event_key();

    if (cl == NULL || itm == NULL)
        return;

    // F8 window
    if (nK == XK_F8 && downState == false)
    {
        svShowF8Window();
        return;
    }

    // F12 macro
    if (nK == XK_F12 && downState == false)
    {
        svSendKeyStrokesToHost(itm->f12Macro, vnc);
        return;
    }

    // send key
    if ((nK >= 32 && nK <= 255) && Fl::event_ctrl() == 0)
        SendKeyEvent(cl, strIn[0], downState);
    else
        SendKeyEvent(cl, nK, downState);
}
