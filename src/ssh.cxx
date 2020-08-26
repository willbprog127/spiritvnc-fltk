/*
 * ssh.cxx - part of SpiritVNC - FLTK
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
#include "hostitem.h"
#include "ssh.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>
#include <libssh2.h>


/* create ssh session and ssh forwarding */
/* (this is called as a pthread because it blocks) */
void * svCreateSSHConnection (void * data)
{
    pthread_detach(pthread_self());

    const unsigned int LIBSSH2_INADDR_NONE = (in_addr_t) - 1;

    enum {
        LIBSSH2_AUTH_NONE = 0,
        LIBSSH2_AUTH_PASSWORD,
        LIBSSH2_AUTH_PUBLICKEY
    };

    int sockSSHSock = -1;
    int sockSSHListenSock = -1;
    int sockSSHForwardSock = -1;
    int i = 0;
    int nSSHAuthType = LIBSSH2_AUTH_NONE;
    int nSSHSockOption = 0;
    int nLoopErrors = 0;
    int nLoopErrorLimit = 100;
    unsigned int nSSHLocalPort = 0;
    ssize_t sztSSHLen = 0;
    ssize_t sztSSHWr = 0;

    char * strSSHLocalAddress = NULL;
    char sshBuffer[16384] = {0};
    char * strUserAuthList = NULL;
    char strError[255] = {0};
    bool authError = false;

    LIBSSH2_SESSION * sshSession = NULL;
    LIBSSH2_CHANNEL * sshChannel = NULL;
    fd_set structSSHSockSet;
    struct sockaddr_in structSSHSockAddress;
    struct timeval structSSHTimeVal;
    socklen_t sltSSHSockAddressLength = 0;

    bool sshError = false;

    HostItem * itm = static_cast<HostItem *>(data);

    if (itm == NULL)
    {
        itm->hasError = true;
        return SV_RET_VOID;
    }

    // initialize the libssh library
    if (libssh2_init(0) != 0)
    {
        svDebugLog("svCreateSSHConnection - ERROR - library initialization failed");
        svMessageWindow("Error: Could not initialize libssh2");
        itm->hasError = true;
        return SV_RET_VOID;
    }

    // connect to SSH server
    sockSSHSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    structSSHSockAddress.sin_family = AF_INET;
    if ((structSSHSockAddress.sin_addr.s_addr = inet_addr(itm->hostAddress.c_str())) ==
        LIBSSH2_INADDR_NONE)
    {
        svDebugLog("svCreateSSHConnection - ERROR - Bad SSH server address");
        itm->hasError = true;
        return SV_RET_VOID;
    }

    structSSHSockAddress.sin_port = htons(atoi(itm->sshPort.c_str()));
    if (connect(sockSSHSock, reinterpret_cast<sockaddr *>(&structSSHSockAddress),
        sizeof(sockaddr_in)) != 0)
    {
        svDebugLog("svCreateSSHConnection - ERROR - Could not connect to SSH server");
        // don't change itm state for this one
        return SV_RET_VOID;
    }

    /* Create a session instance */
    sshSession = libssh2_session_init();
    if (sshSession == NULL)
    {
        svDebugLog("svCreateSSHConnection - ERROR - Could not initialize SSH session");
        itm->hasError = true;
        return SV_RET_VOID;
    }

    // starts SSH session - this trades welcome banners, exchanges keys,
    // sets up crypto, compression, and MAC layers
    if (libssh2_session_handshake(sshSession, sockSSHSock) != 0)
    {
        svDebugLog("svCreateSSHConnection - ERROR - Error when starting up SSH session");
        if (sshSession != NULL)
            libssh2_session_free(sshSession);
        itm->hasError = true;
        return SV_RET_VOID;
    }

    // check which authentication methods are available
    strUserAuthList = libssh2_userauth_list(sshSession, itm->sshUser.c_str(),
        itm->sshUser.size());

    // add 'password' authentication to our bitmap
    if (strstr(strUserAuthList, "password"))
        nSSHAuthType |= LIBSSH2_AUTH_PASSWORD;

    // add 'public key' authentication to our bitmap
    if (strstr(strUserAuthList, "publickey"))
        nSSHAuthType |= LIBSSH2_AUTH_PUBLICKEY;

    // try password authentication
    if (nSSHAuthType & LIBSSH2_AUTH_PASSWORD)
    {
        if (libssh2_userauth_password(sshSession, itm->sshUser.c_str(),
            itm->sshPass.c_str()) !=0) {
            svDebugLog("svCreateSSHConnection - ERROR - Authentication by password failed");
            authError = true;
        }
    }

    // try public key authentication
    if (nSSHAuthType & LIBSSH2_AUTH_PUBLICKEY)
    {
        if (libssh2_userauth_publickey_fromfile(sshSession, itm->sshUser.c_str(),
                itm->sshKeyPublic.c_str(), itm->sshKeyPrivate.c_str(),
                    itm->sshPass.c_str()) !=0)
        {
            svDebugLog("svCreateSSHConnection -  ERROR - Authentication by public key failed");
            authError = true;
        }
        authError = false;
        //svLogToFile("svCreateSSHConnection - Authentication by public key succeeded");
    }

    // no authorization methods were successful
    if (authError == true)
    {
        svDebugLog("svCreateSSHConnection - ERROR - All supported authentication"
            " methods failed");
        sshError = true;
    }

    if (sshError == false)
    {
        sockSSHListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        structSSHSockAddress.sin_family = AF_INET;
        structSSHSockAddress.sin_port = htons(itm->sshLocalPort);

        if ((structSSHSockAddress.sin_addr.s_addr = inet_addr("127.0.0.1")) ==
            LIBSSH2_INADDR_NONE)
        {
            svDebugLog("svCreateSSHConnection - ERROR - Bad local/machine address");
            sshError = true;
        }
    }

    if (sshError == false)
    {
        nSSHSockOption = 1;
        setsockopt(sockSSHListenSock, SOL_SOCKET, SO_REUSEADDR, &nSSHSockOption,
            sizeof(nSSHSockOption));
        sltSSHSockAddressLength = sizeof(structSSHSockAddress);

        if (bind(sockSSHListenSock, reinterpret_cast<sockaddr *>(&structSSHSockAddress),
            sltSSHSockAddressLength) == -1)
        {
            svDebugLog("svCreateSSHConnection - ERROR - Could not bind to local"
                " machine address");
            sshError = true;
        }
    }

    if (sshError == false)
    {
        if (listen(sockSSHListenSock, 2) == -1)
        {
            svDebugLog("svCreateSSHConnection - ERROR - Could not listen on local"
                " machine address");
            sshError = true;
        }
    }

    if (sshError == false)
    {
        snprintf(strError, 255, "svCreateSSHConnection - Waiting for SpiritVNC to"
            " connect on %s:%d...", inet_ntoa(structSSHSockAddress.sin_addr),
            ntohs(structSSHSockAddress.sin_port));
        svDebugLog(strError);

        itm->sshReady = true;

        sockSSHForwardSock = accept(sockSSHListenSock,
            reinterpret_cast<sockaddr *>(&structSSHSockAddress), &sltSSHSockAddressLength);
        if (sockSSHForwardSock == -1)
        {
            svDebugLog("svCreateSSHConnection - ERROR - Forwarding not accepted");
            sshError = true;
        }
    }

    if (sshError == false)
    {
        strSSHLocalAddress = inet_ntoa(structSSHSockAddress.sin_addr);
        nSSHLocalPort = ntohs(structSSHSockAddress.sin_port);

        snprintf(strError, 255, "svCreateSSHConnection - Forwarding connection from %s:%u"
            " local to remote %s:%u", strSSHLocalAddress, nSSHLocalPort,
            "localhost", atoi(itm->vncPort.c_str()));
        svDebugLog(strError);

        sshChannel = libssh2_channel_direct_tcpip_ex(sshSession, "localhost",
            atoi(itm->vncPort.c_str()), strSSHLocalAddress, nSSHLocalPort);
        if (sshChannel == NULL)
        {
            svDebugLog("svCreateSSHConnection - ERROR - Could not open the"
                " direct-TCP/IP channel");
            sshError = true;
        }
    }

    if (sshError == false)
        svLogToFile("SpiritVNC - SSH connection established with "
            + itm->name + " - " + itm->hostAddress);

    // use non-blocking IO from here (due to current libssh2 API)
    libssh2_session_set_blocking(sshSession, 0);

    while (sshError == false && itm->stopSSH == false)
    {
        FD_ZERO(&structSSHSockSet);
        FD_SET(sockSSHForwardSock, &structSSHSockSet);

        structSSHTimeVal.tv_sec = 0;
        structSSHTimeVal.tv_usec = 5000;

        int rc = select(sockSSHForwardSock + 1, &structSSHSockSet, NULL, NULL, &structSSHTimeVal);

        nLoopErrors = 0;

        if (rc == -1)
        {
            svDebugLog("svCreateSSHConnection - ERROR - select() error on socket");
            sshError = true;
            break;
        }

        if (rc != 0 && FD_ISSET(sockSSHForwardSock, &structSSHSockSet))
        {
            sztSSHLen = recv(sockSSHForwardSock, sshBuffer, sizeof(sshBuffer), 0);

            if (sztSSHLen < 0)
            {
                svDebugLog("svCreateSSHConnection - ERROR - Socket receive error");
                sshError = true;
                break;
            }
            else
            if (sztSSHLen == 0)
            {
                snprintf(strError, 255, "svCreateSSHConnection - SpiritVNC viewer disconnected"
                    " from %s:%u", strSSHLocalAddress, nSSHLocalPort);
                svDebugLog(strError);
                break;
            }

            sztSSHWr = 0;

            do
            {
                i = libssh2_channel_write(sshChannel, sshBuffer, sztSSHLen);

                if (i < 0)
                {
                    svDebugLog("svCreateSSHConnection - ERROR - libssh2_channel_write error");
                    // if we exceed the loop error limit, give up

                    nLoopErrors ++;
                    if (nLoopErrors > nLoopErrorLimit)
                    {
                        sshError = true;
                        svDebugLog("svCreateSSHConnection - sshError: channel write error");
                        break;
                    }
                }
                sztSSHWr += i;
            } while (i > 0 && sztSSHWr < sztSSHLen);

            nLoopErrors = 0;
        }

        while (sshError == false && itm->stopSSH == false)
        {
            sztSSHLen = libssh2_channel_read(sshChannel, sshBuffer, sizeof(sshBuffer));

            if (sztSSHLen == LIBSSH2_ERROR_EAGAIN)
                break;
            else
            if (sztSSHLen < 0)
            {
                svDebugLog("svCreateSSHConnection - ERROR - libssh2_channel_read error");
                // if we exceed the loop error limit, give up
                nLoopErrors ++;
                if (nLoopErrors > nLoopErrorLimit)
                {
                    sshError = true;
                    break;
                }
            }

            sztSSHWr = 0;
            nLoopErrors = 0;

            while (sztSSHWr < sztSSHLen)
            {
                i = send(sockSSHForwardSock, sshBuffer + sztSSHWr, sztSSHLen - sztSSHWr, 0);

                if (i <= 0)
                {
                    svDebugLog("svCreateSSHConnection - ERROR - Socket send error");
                    sshError = true;
                    break;
                }
                sztSSHWr += i;
            }

            if (libssh2_channel_eof(sshChannel))
            {
                snprintf(strError, 255, "svCreateSSHConnection - The server"
                    " at %s:%u disconnected", "localhost", atoi(itm->vncPort.c_str()));
                svDebugLog(strError);
                sshError = true;
                break;
            }
        }
    }

    if (sshError == false)
        svLogToFile("SSH connection disconnected normally from '"
            + itm->name + "' - " + itm->hostAddress);
    else
    {
        svLogToFile("SSH connection disconnected abnormally from '"
            + itm->name + "' - " + itm->hostAddress);
        itm->hasError = true;
    }

    // shutdown and clean up
    itm->sshReady = false;
    close(sockSSHForwardSock);
    close(sockSSHListenSock);

    if (sshChannel != NULL)
        libssh2_channel_free(sshChannel);

    libssh2_session_disconnect(sshSession, "SpiritVNC disconnected normally");
    libssh2_session_free(sshSession);

    close(sockSSHSock);

    libssh2_exit();
    itm->stopSSH = false;

    return SV_RET_VOID;
}
