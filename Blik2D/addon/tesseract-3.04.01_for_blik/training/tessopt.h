/**********************************************************************
 * File:        getopt.h
 * Description: Re-implementation of the unix code.
 * Author:					Ray Smith
 * Created:					Tue Nov 28 05:52:50 MST 1995
 *
 * (C) Copyright 1995, Hewlett-Packard Co.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include          BLIK_TESSERACT_U_host_h //original-code:"host.h"

extern int tessoptind;
extern char *tessoptarg;

int tessopt (                     //parse args
inT32 argc,                      //arg count
char *argv[],                    //args
const char *arglist                    //string of arg chars
);
