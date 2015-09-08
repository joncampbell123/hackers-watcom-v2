/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description: Internal ORL interfaces. 
*
****************************************************************************/


#ifndef ORL_INTERNAL_INCLUDED
#define ORL_INTERNAL_INCLUDED

/* included in the ORL level through the lower level */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "orlglobl.h"
#include "orlhshdf.h"

#if !defined( __386__ ) && !defined( __GNUC__ )
#define ORLUNALIGNED __unaligned
#else
#define ORLUNALIGNED
#endif

/* NB The following are sort of fake.  We are hiding the contents of
   the file-specific section and symbol handles from the ORL
   level, allowing it only to check their type. */

#define ORL_SEC_TYPE(x)     (((orli_sec_handle)x)->type)
#define ORL_SYMBOL_TYPE(x)  (((orli_symbol_handle)x)->type)
#define ORL_GROUP_TYPE(x)   (((orli_group_handle)x)->type)

typedef struct orli_sec_handle_struct {
    orl_file_format                     type;
} * orli_sec_handle;

typedef struct orli_symbol_handle_struct {
    orl_file_format                     type;
} * orli_symbol_handle;

typedef struct orli_group_handle_struct {
    orl_file_format                     type;
} * orli_group_handle;

#include "orlcomon.h"

#endif
