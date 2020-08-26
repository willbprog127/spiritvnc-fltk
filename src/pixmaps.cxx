/*
 * pixmaps.cxx - part of SpiritVNC - FLTK
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


#include "pixmaps.h"

const char * pmBlank[] = {
    "16 16 1 1",
    "   c None",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                "
};

const char * pmListAddNew[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "                ",
    "                ",
    "       ..       ",
    "       ..       ",
    "       ..       ",
    "       ..       ",
    "       ..       ",
    "  ............  ",
    "  ............  ",
    "       ..       ",
    "       ..       ",
    "       ..       ",
    "       ..       ",
    "       ..       ",
    "                ",
    "                "
};

const char * pmListDelete[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "  ............  ",
    "  ............  ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                "
};

const char * pmListDown[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "                ",
    "                ",
    "  ............  ",
    "  ............  ",
    "   ..........   ",
    "   ..........   ",
    "    ........    ",
    "    ........    ",
    "     ......     ",
    "     ......     ",
    "      ....      ",
    "      ....      ",
    "       ..       ",
    "       ..       ",
    "                ",
    "                "
};

const char * pmListListen[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "      .....     ",
    "     ..   ..    ",
    "    .  ... ..   ",
    "    . .  .  .   ",
    "   .  . ...     ",
    "   . .  . .     ",
    "   . . .. .     ",
    "   ..  .  .     ",
    "    .  .  .     ",
    "    .. .. .     ",
    "     .  ...     ",
    "     .          ",
    "     ..         ",
    "      .    .    ",
    "      ..  ..    ",
    "       ....     "
};

const char * pmListOptions[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "                ",
    " .............  ",
    " .............  ",
    " .............  ",
    "                ",
    "                ",
    " .............  ",
    " .............  ",
    " .............  ",
    "                ",
    "                ",
    " .............  ",
    " .............  ",
    " .............  ",
    "                ",
    "                "
};

const char * pmListScan[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "                ",
    "     ......     ",
    "    ........    ",
    "   ..      ..   ",
    "  ....      ..  ",
    " ..  ..      .. ",
    " ..   ..     .. ",
    " ..   ...    .. ",
    " ..    ..    .. ",
    " ..          .. ",
    " ..          .. ",
    "  ..        ..  ",
    "   ..      ..   ",
    "    ........    ",
    "     ......     ",
    "                "
};

const char * pmListScanScanning[] = {
    "16 16 3 1",
    "   c None",
    ".  c #404040",
    "+  c #74D57D",
    "                ",
    "  +++......+++  ",
    " +++........+++ ",
    " ++..++..++..++ ",
    " +....++++++..+ ",
    " ..++..++++++.. ",
    " ..+++..+++++.. ",
    " ...++...+++... ",
    " ...+++..+++... ",
    " ..++++++++++.. ",
    " ..++++++++++.. ",
    " +..++++++++..+ ",
    " ++..++..++..++ ",
    " +++........+++ ",
    "  +++......+++  ",
    "                "
};

const char * pmListUp[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "                ",
    "                ",
    "       ..       ",
    "       ..       ",
    "      ....      ",
    "      ....      ",
    "     ......     ",
    "     ......     ",
    "    ........    ",
    "    ........    ",
    "   ..........   ",
    "   ..........   ",
    "  ............  ",
    "  ............  ",
    "                ",
    "                "
};

const char * pmStatusConnected[] = {
    "16 16 2 1",
    "   c None",
    ".  c #58BF55",
    "                ",
    "                ",
    "                ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusConnecting[] = {
    "16 16 5 1",
    "   c None",
    ".  c #666666",
    "+  c #D5D5D5",
    "@  c #E8E8E8",
    "#  c #EFEFEF",
    "                ",
    "                ",
    "                ",
    "   ++++++++++   ",
    "   +@##..##@+   ",
    "   +###..###+   ",
    "   +###..###+   ",
    "   +###....#+   ",
    "   +###....#+   ",
    "   +########+   ",
    "   +########+   ",
    "   +@######@+   ",
    "   ++++++++++   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusDisconnected[] = {
    "16 16 2 1",
    "   c None",
    ".  c #D5D5D5",
    "                ",
    "                ",
    "                ",
    "   ..........   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   ..........   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusDisconnectedError[] = {
    "16 16 3 1",
    "   c None",
    ".  c #D4D4D4",
    "+  c #F3F16E",
    "                ",
    "                ",
    "                ",
    "   ..........   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   ..........   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusDisconnectedBigError[] = {
"16 16 4 1",
" 	c None",
".	c #D4D4D4",
"+	c #C07271",
"@	c #FFFFFF",
"                ",
"                ",
"                ",
"   ..........   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .++++++++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   ..........   ",
"                ",
"                ",
"                "
};

const char * pmStatusNoConnect[] = {
    "16 16 2 1",
    "   c None",
    ".  c #BFBFBF",
    "                ",
    "                ",
    "                ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "   ..........   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusConnectedCB[] = {
    "16 16 4 1",
    "   c None",
    ".  c #666666",
    "+  c #D4D4D4",
    "@  c #EFEFEF",
    "                ",
    "                ",
    "                ",
    "   ++++++++++   ",
    "   +@@@..@@@+   ",
    "   +@@@..@@@+   ",
    "   +@@@..@@@+   ",
    "   +@@@..@@@+   ",
    "   +@@@..@@@+   ",
    "   +@@@..@@@+   ",
    "   +@@@..@@@+   ",
    "   +@@@..@@@+   ",
    "   ++++++++++   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusConnectingCB[] = {
    "16 16 5 1",
    "   c None",
    ".  c #666666",
    "+  c #D5D5D5",
    "@  c #E8E8E8",
    "#  c #EFEFEF",
    "                ",
    "                ",
    "                ",
    "   ++++++++++   ",
    "   +@##..##@+   ",
    "   +###..###+   ",
    "   +###..###+   ",
    "   +###....#+   ",
    "   +###....#+   ",
    "   +########+   ",
    "   +########+   ",
    "   +@######@+   ",
    "   ++++++++++   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusDisconnectedCB[] = {
    "16 16 2 1",
    "   c None",
    ".  c #D5D5D5",
    "                ",
    "                ",
    "                ",
    "   ..........   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   .        .   ",
    "   ..........   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusDisconnectedErrorCB[] = {
    "16 16 4 1",
    "   c None",
    ".  c #666666",
    "+  c #D4D4D4",
    "@  c #EFEFEF",
    "                ",
    "                ",
    "                ",
    "   ++++++++++   ",
    "   +@@@@@@..+   ",
    "   +@@@@@...+   ",
    "   +@@@@...@+   ",
    "   +@@@@@.@@+   ",
    "   +@@.@@@@@+   ",
    "   +@...@@@@+   ",
    "   +...@@@@@+   ",
    "   +..@@@@@@+   ",
    "   ++++++++++   ",
    "                ",
    "                ",
    "                "
};

const char * pmStatusDisconnectedBigErrorCB[] = {
"16 16 4 1",
" 	c None",
".	c #D4D4D4",
"+	c #000000",
"@	c #FFFFFF",
"                ",
"                ",
"                ",
"   ..........   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   .++++++++.   ",
"   .+++@@+++.   ",
"   .+++@@+++.   ",
"   ..........   ",
"                ",
"                ",
"                "
};

const char * pmStatusNoConnectCB[] = {
    "16 16 4 1",
    "   c None",
    ".  c #666666",
    "+  c #BFBFBF",
    "@  c #EFEFEF",
    "                ",
    "                ",
    "                ",
    "   ++++++++++   ",
    "   +@@@@@@@@+   ",
    "   +@@@@@@@@+   ",
    "   +@@@@@@@@+   ",
    "   +........+   ",
    "   +........+   ",
    "   +@@@@@@@@+   ",
    "   +@@@@@@@@+   ",
    "   +@@@@@@@@+   ",
    "   ++++++++++   ",
    "                ",
    "                ",
    "                "
};

const char * pmListHelp[] = {
    "16 16 2 1",
    "   c None",
    ".  c #404040",
    "                ",
    "     ......     ",
    "    ........    ",
    "   ....  ....   ",
    "   ...    ...   ",
    "   ...    ...   ",
    "          ...   ",
    "        ...     ",
    "      ....      ",
    "      ...       ",
    "      ...       ",
    "                ",
    "      ...       ",
    "      ...       ",
    "      ...       ",
    "                "
};
