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
* Description:  WLIB DLL version mainline.
*
****************************************************************************/


#include "wlib.h"
#include "idedll.h"
#ifdef __WATCOMC__
    #include <malloc.h> // for _heapshrink()
#endif
#include "banner.h"
#include "main.h"

#include "clibext.h"


static IDECBHdl     ideHdl;
static IDECallBacks *ideCb;
static IDEInitInfo  *ideInfo;

unsigned IDEAPI IDEGetVersion( void )
{
    return( IDE_CUR_DLL_VER );
}

IDEBool IDEAPI IDEInitDLL( IDECBHdl hdl, IDECallBacks *cb, IDEDllHdl *info )
{
    ideHdl = hdl;
    ideCb = cb;
    *info = NULL;
    return( InitSubSystems() != 0 );
}

IDEBool IDEAPI IDEPassInitInfo( IDEDllHdl hdl, IDEInitInfo *info )
{
    hdl = hdl;
    ideInfo = info;
    return( FALSE );
}

IDEBool IDEAPI IDERunYourSelf( IDEDllHdl hdl, const char *opts, IDEBool *fatalerr )
{
    char        *argv[ 3 ];

    hdl = hdl;
    *fatalerr = FALSE;
    argv[ 0 ] = "";
    argv[ 1 ] = (char *)opts;
    argv[ 2 ] = NULL;
    return( WlibMainLine( argv ) != 0 );
}

IDEBool IDEAPI IDERunYourSelfArgv(// COMPILE A PROGRAM (ARGV ARGS)
    IDEDllHdl hdl,              // - handle for this instantiation
    int argc,                   // - # of arguments
    char **argv,                // - argument vector
    IDEBool* fatal_error )      // - addr[ fatality indication ]
{
    hdl = hdl; argc = argc;
    *fatal_error = FALSE;
    return( WlibMainLine( argv ) != 0 );
}

void IDEAPI IDEStopRunning( void )
{
    if( !ideInfo || ideInfo->ver <= 2 || ideInfo->console_output ) {
        exit( 1 );
    } else {
        longjmp( Env, 1 );
    }
}

void IDEAPI IDEFreeHeap( void )
{
#ifdef __WATCOMC__
    _heapshrink();
#endif
}

void IDEAPI IDEFiniDLL( IDEDllHdl hdl )
{
    hdl = hdl;
    FiniSubSystems();
}

char *WlibGetEnv( char *name )
{
    char *env;

    if( ideInfo->ignore_env == FALSE && ideCb != NULL ) {
        if( ideCb->GetInfo( ideHdl, IDE_GET_ENV_VAR, (IDEGetInfoWParam)name, (IDEGetInfoLParam)&env ) == FALSE ) {
            return( env );
        }
    }
    return( NULL );

}
void FatalResError( void )
{
    IDEMsgInfo          msg_info;

    if( ideCb != NULL ) {
        msg_info.severity = IDEMSGSEV_ERROR;
        msg_info.flags = 0;
        msg_info.helpfile = NULL;
        msg_info.helpid = 0;
        msg_info.msg = NO_RES_MESSAGE;
        ideCb->PrintWithInfo( ideHdl, &msg_info );
    }
    longjmp( Env, 1 );
}

void FatalError( int str, ... )
{
    va_list             arglist;
    char                buff[ MAX_ERROR_SIZE ];
    char                msg[ 512 ];
    IDEMsgInfo          msg_info;

    va_start( arglist, str );
    MsgGet( str, buff );
    vsnprintf( msg, 512, buff, arglist );
    if( ideCb != NULL ) {
        IdeMsgInit( &msg_info, IDEMSGSEV_ERROR, msg );
        ideCb->PrintWithInfo( ideHdl, &msg_info );
    }
    va_end( arglist );
    longjmp( Env, 1 );
}

void Warning( int str, ... )
{
    va_list             arglist;
    char                buff[ MAX_ERROR_SIZE ];
    char                msg[ 512 ];
    IDEMsgInfo          msg_info;

    if( Options.quiet )
        return;
    MsgGet( str, buff );
    va_start( arglist, str );
    vsnprintf( msg, 512, buff, arglist );
    if( ideCb != NULL ) {
        IdeMsgInit( &msg_info, IDEMSGSEV_WARNING, msg );
        ideCb->PrintWithInfo( ideHdl, &msg_info );
    }
    va_end( arglist );
}

void Message( char *buff, ... )
{
    va_list             arglist;
    char                msg[ 512 ];
    IDEMsgInfo          msg_info;

    if( Options.quiet )
        return;
    va_start( arglist, buff );
    vsnprintf( msg, 512, buff, arglist );
    if( ideCb != NULL ) {
        IdeMsgInit( &msg_info, IDEMSGSEV_NOTE_MSG, msg );
        ideCb->PrintWithInfo( ideHdl, &msg_info );
    }
    va_end( arglist );
}

static bool Wait_for_return( void )
/*********************************/
// return true if we should stop printing
{
    int   c;

    c = getchar();
    return( c == 'q' || c == 'Q' );
}

void Usage( void )
/****************/
{
    char                buff[ MAX_ERROR_SIZE ];
    int                 str;
    int                 str_first;
    int                 str_last;
    IDEMsgInfo          msg_info;
    int                 count;
    if( ideCb != NULL ) {
        msg_info.severity = IDEMSGSEV_BANNER;
        count = 3;
        msg_info.flags = 0;
        msg_info.helpfile = NULL;
        msg_info.helpid = 0;
        msg_info.msg = buff;
        if( Options.ar ) {
            str_first = USAGE_AR_FIRST;
            str_last = USAGE_AR_LAST;
        } else {
            str_first = USAGE_WLIB_FIRST;
            str_last = USAGE_WLIB_LAST;
        }
        for( str = str_first; str <= str_last; ++str ) {
            MsgGet( str, buff );
            if( ideInfo && ideInfo->ver > 2 && ideInfo->console_output &&
                ( count > 20 && buff[ 0 ] == '\0' || count == 24 ) ) {
                msg_info.msg = "    (Press Return to continue)" ;
                ideCb->PrintWithInfo( ideHdl, &msg_info );
                if( Wait_for_return() )
                    break;
                count = 0;
                msg_info.msg = buff;
            }
            ++count;
            if( buff[ 0 ] == '\0' ) {
                continue;
            }
            ideCb->PrintWithInfo( ideHdl, &msg_info );
        }
    }
    longjmp( Env, 1 );
}

void Banner( void )
{
    IDEMsgInfo          msg_info;
    static char *bannerText[] = {
#ifndef NDEBUG
        banner1w( "Library Manager", _WLIB_VERSION_ ) " [Internal Development]",
#else
        banner1w( "Library Manager", _WLIB_VERSION_ ),
#endif
        banner2,
        banner2a( "1984" ),
        banner3,
        banner3a,
        NULL
    };
    static int alreadyDone=0;
    char **text;

    if( Options.quiet || alreadyDone || Options.terse_listing )
        return;

    alreadyDone = 1;
    if( ideCb != NULL ) {
        msg_info.severity = IDEMSGSEV_BANNER;
        msg_info.flags = 0;
        msg_info.helpfile = NULL;
        msg_info.helpid = 0;
        text = bannerText;
        while( *text ) {
            msg_info.msg = *text++;
            ideCb->PrintWithInfo( ideHdl, &msg_info );
        }
    }
}
