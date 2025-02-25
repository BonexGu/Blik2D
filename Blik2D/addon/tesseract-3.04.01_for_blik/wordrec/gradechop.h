/* -*-C-*-
 ********************************************************************************
 *
 * File:        gradechop.h  (Formerly gradechop.h)
 * Description:
 * Author:       Mark Seaman, SW Productivity
 * Created:      Fri Oct 16 14:37:00 1987
 * Modified:     Tue Jul  9 16:40:39 1991 (Mark Seaman) marks@hpgrlt
 * Language:     C
 * Package:      N/A
 * Status:       Reusable Software Component
 *
 * (c) Copyright 1987, Hewlett-Packard Company.
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
 *********************************************************************************/

#ifndef GRADECHOP_H
#define GRADECHOP_H

/*----------------------------------------------------------------------
              I n c l u d e s
----------------------------------------------------------------------*/
#include BLIK_TESSERACT_U_seam_h //original-code:"seam.h"
#include BLIK_TESSERACT_U_ndminx_h //original-code:"ndminx.h"

/*----------------------------------------------------------------------
              M a c r o s
----------------------------------------------------------------------*/
/**********************************************************************
 * partial_split_priority
 *
 * Assign a priority to this split based on the features that it has.
 * Grade it according to the different rating schemes and return the
 * value of its goodness.
 **********************************************************************/

#define partial_split_priority(split)  \
(grade_split_length   (split) +      \
	grade_sharpness      (split))       \

#endif
