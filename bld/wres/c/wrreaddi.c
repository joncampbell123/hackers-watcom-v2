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
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include <string.h>
#include "wresall.h"
#include "reserr.h"
#include "wresdefn.h"
#include "wresrtns.h"


static bool readLangInfoList( WResFileID fid, WResResNode *res, void *fileinfo )
{
    unsigned            i;
    WResLangNode        *langnode;
    WResFileSSize       numread;

    for( i = 0; i < res->Info.NumResources; i++ ) {
        langnode = WRESALLOC( sizeof( WResLangNode ) );
        if( langnode == NULL ) {
            WRES_ERROR( WRS_MALLOC_FAILED );
            return( true );
        }
        numread = WRESREAD( fid, &(langnode->Info), sizeof( WResLangInfo ) );
        if( numread != sizeof( WResLangInfo ) ) {
            WRES_ERROR( WRESIOERR( fid, numread ) ? WRS_READ_FAILED : WRS_READ_INCOMPLETE );
            WRESFREE( langnode );
            return( true );
        }
        langnode->data = NULL;
        langnode->fileInfo = fileinfo;
        ResAddLLItemAtEnd( (void **)&(res->Head), (void **)&(res->Tail), langnode );
    }
    return( false );
}

static bool readResList( WResFileID fid, WResTypeNode *currtype, uint_16 ver, void *fileinfo )
{
    WResResNode     *newnode = NULL;
    WResResInfo     tmpresinfo;
    WResResInfo1    tmpresinfo1;
    WResLangNode    *langnode;
    bool            error;
    int             resnum;
    int             extrabytes;

    extrabytes = 0;
    error = false;
    /* loop through the list of resources of this type */
    for( resnum = 0; resnum < currtype->Info.NumResources && !error; resnum++ ) {

        /* read a resource record from disk */
        if( ver < 2 ) {
            error = WResReadFixedResRecord1( &tmpresinfo1, fid );
            tmpresinfo.NumResources = 1;
            tmpresinfo.ResName.IsName = tmpresinfo1.ResName.IsName;
            if( tmpresinfo.ResName.IsName ) {
                tmpresinfo.ResName.ID.Name.Name[0] = tmpresinfo1.ResName.ID.Name.Name[0];
                tmpresinfo.ResName.ID.Name.NumChars = tmpresinfo1.ResName.ID.Name.NumChars;
            } else {
                tmpresinfo.ResName.ID.Num = tmpresinfo1.ResName.ID.Num;
            }
        } else if( ver == 2 ) {
            error = WResReadFixedResRecord2( &tmpresinfo, fid );
        } else {
            error = WResReadFixedResRecord( &tmpresinfo, fid );
        }

        if( !error ) {
            /* allocate a new node */
            extrabytes = WResIDExtraBytes( &tmpresinfo.ResName );
            newnode = WRESALLOC( sizeof( WResResNode ) + extrabytes );
            if( newnode == NULL ) {
                error = true;
                WRES_ERROR( WRS_MALLOC_FAILED );
            }
        }
        if( !error ) {
            newnode->Head = NULL;
            newnode->Tail = NULL;
            /* copy the new resource info into the new node */
            memcpy( &(newnode->Info), &tmpresinfo, sizeof( WResResInfo ) );

            /* read the extra bytes (if any) */
            if( extrabytes > 0 ) {
                error = WResReadExtraWResID( &(newnode->Info.ResName), fid );
            }

            if( ver < 2 ) {
                langnode = WRESALLOC( sizeof( WResLangNode ) );
                if( langnode == NULL ) {
                    error = true;
                    WRES_ERROR( WRS_MALLOC_FAILED );
                }
                if( !error ) {
                    langnode->data = NULL;
                    langnode->fileInfo = fileinfo;
                    langnode->Info.MemoryFlags = tmpresinfo1.MemoryFlags;
                    langnode->Info.Offset = tmpresinfo1.Offset;
                    langnode->Info.Length = tmpresinfo1.Length;
                    langnode->Info.lang.lang = DEF_LANG;
                    langnode->Info.lang.sublang = DEF_SUBLANG;
                    ResAddLLItemAtEnd( (void **)&(newnode->Head), (void **)&(newnode->Tail), langnode );
                }
            } else if( ver == 2 ) {
                error = readLangInfoList( fid, newnode, fileinfo );
            } else {
                error = readLangInfoList( fid, newnode, fileinfo );
            }
        }
        if( !error ) {
            /* add the resource node to the linked list */
            ResAddLLItemAtEnd( (void **)&(currtype->Head), (void **)&(currtype->Tail), newnode );
        }
    }

    return( error );

} /* readResList */

static bool readTypeList( WResFileID fid, WResDirHead *currdir,uint_16 ver, void *fileinfo )
{
    WResTypeNode    *newnode;
    WResTypeInfo    newtype;
    bool            error;
    int             typenum;
    int             extrabytes;

    newnode = NULL;
    extrabytes = 0;
    error = false;
    /* loop through the list of types */
    for( typenum = 0; typenum < currdir->NumTypes && !error; typenum++ ) {
        /* read a type record from disk */
        if( ver < 3 ) {
            error = WResReadFixedTypeRecord1or2( &newtype, fid );
        } else {
            error = WResReadFixedTypeRecord( &newtype, fid );
        }
        if( !error ) {
            /* allocate a new node */
            extrabytes = WResIDExtraBytes( &(newtype.TypeName) );
            newnode = WRESALLOC( sizeof( WResTypeNode ) + extrabytes );
            if( newnode == NULL ) {
                error = true;
                WRES_ERROR( WRS_MALLOC_FAILED );
            }
        }
        if( !error ) {
            /* initialize the linked list of resources */
            newnode->Head = NULL;
            newnode->Tail = NULL;
            /* copy the new type info into the new node */
            memcpy( &(newnode->Info), &newtype, sizeof( WResTypeInfo ) );

            /* read the extra bytes (if any) */
            if( extrabytes > 0 ) {
                error = WResReadExtraWResID( &(newnode->Info.TypeName), fid );
            }
        }
        if( !error ) {
            /* add the type node to the linked list */
            ResAddLLItemAtEnd( (void **)&(currdir->Head), (void **)&(currdir->Tail), newnode );
            /* read in the list of resources of this type */
            error = readResList( fid, newnode, ver, fileinfo );
        }
    }

    return( error );

} /* readTypeList */

static bool readWResDir( WResFileID fid, WResDir currdir, void *fileinfo )
{
    WResHeader      head;
    WResExtHeader   ext_head;
    bool            error;

    ext_head.TargetOS = WRES_OS_WIN16;
    /* read the header and check that it is valid */
    error = WResReadHeaderRecord( &head, fid );
    if( !error ) {
        if( head.Magic[0] != WRESMAGIC0 || head.Magic[1] != WRESMAGIC1 ) {
            error = true;
            WRES_ERROR( WRS_BAD_SIG );
        }
    }
    if( !error ) {
        if( head.WResVer > WRESVERSION ) {
            error = true;
            WRES_ERROR( WRS_BAD_VERSION );
        }
    }
    if( !error ) {
        if( head.WResVer >= 1 ) {
            /*
             * seek to the extended header and read it
             */
            error = ( WRESSEEK( fid, sizeof( head ), SEEK_CUR ) == -1 );
            if( error ) {
                WRES_ERROR( WRS_SEEK_FAILED );
            } else {
                error = WResReadExtHeader( &ext_head, fid );
            }
        }
    }

    /* set up the initial info for the directory and seek to it's start */
    if( !error ) {
        currdir->NumResources = head.NumResources;
        currdir->NumTypes = head.NumTypes;
        currdir->TargetOS = ext_head.TargetOS;
        error = ( WRESSEEK( fid, head.DirOffset, SEEK_SET ) == -1 );
        if( error ) {
            WRES_ERROR( WRS_SEEK_FAILED );
        }
    }
    /* read in the list of types (and the resources) */
    if( !error ) {
        error = readTypeList( fid, currdir, head.WResVer, fileinfo );
    }

    return( error );

} /* readWResDir */

static bool readMResDir( WResFileID fid, WResDir currdir, bool *dup_discarded,
                        bool iswin32, void *fileinfo )
/****************************************************************************/
{
    MResResourceHeader      *head = NULL;
    M32ResResourceHeader    *head32 = NULL;
    WResDirWindow           dup;
    bool                    error;
    WResID                  *name;
    WResID                  *type;

    error = false;
    if( iswin32 ) {
        /* Read NULL header */
        head32 = M32ResReadResourceHeader( fid );
        if( head32 != NULL ) {
            MResFreeResourceHeader( head32->head16 );
            WRESFREE( head32 );
        } else {
            error = true;
        }
        if( !error ) {
            head32 = M32ResReadResourceHeader( fid );
            if( head32 != NULL ) {
                head = head32->head16;
            } else {
                error = true;
            }
        }
    } else {
        head = MResReadResourceHeader( fid );
        if( head == NULL ) error = true;
    }
    if( dup_discarded != NULL  ) {
        *dup_discarded = false;
    }
    if( iswin32 ) {
        currdir->TargetOS = WRES_OS_WIN32;
    } else {
        currdir->TargetOS = WRES_OS_WIN16;
    }
    /* assume that a NULL head is the EOF which is the only way of detecting */
    /* the end of a MS .RES file */
    while( head != NULL && !( iswin32 && head32 == NULL ) && !error ) {
        name = WResIDFromNameOrOrd( head->Name );
        type = WResIDFromNameOrOrd( head->Type );
        error = (name == NULL || type == NULL);

        /* MResReadResourceHeader leaves the file at the start of the resource*/
        if( !error ) {
            if( !type->IsName && type->ID.Num == RESOURCE2INT( RT_NAMETABLE ) ) {
                error = false;
            } else {
                error = WResAddResource2( type, name, head->MemoryFlags,
                            WRESTELL( fid ), head->Size, currdir, NULL,
                            &dup, fileinfo );
                if(  error && !WResIsEmptyWindow( dup ) ) {
                    error = false;
                    if( dup_discarded != NULL  ) {
                        *dup_discarded = true;
                    }
                }
            }
        }

        if( !error ) {
            error = ( WRESSEEK( fid, head->Size, SEEK_CUR ) == -1 );
            if( error ) {
                WRES_ERROR( WRS_SEEK_FAILED );
            }
        }

        if( name != NULL ) {
            WRESFREE( name );
            name = NULL;
        }
        if( type != NULL ) {
            WRESFREE( type );
            type = NULL;
        }
        MResFreeResourceHeader( head );
        if( iswin32 ) {
            WRESFREE( head32 );
        }

        if( !error ) {
            if( iswin32 ) {
                head32 = M32ResReadResourceHeader( fid );
                if( head32 != NULL ) {
                    head = head32->head16;
                }
            } else {
                head = MResReadResourceHeader( fid );
            }
        }
    }

    return( error );

} /* readMResDir */

bool WResReadDir( WResFileID fid, WResDir currdir, bool *dup_discarded )
{
    return( WResReadDir2( fid, currdir, dup_discarded, NULL ) );
}

bool WResReadDir2( WResFileID fid, WResDir currdir, bool *dup_discarded, void *fileinfo )
{
    bool            error;
    ResTypeInfo     restype;

    /* var representing whether or not a duplicate dir entry was
     * discarded is set to false.
     * NOTE: duplicates are not discarded by calls to readWResDir.
     */
    if( dup_discarded != NULL  ) {
        *dup_discarded = false;
    }

    /* get rid of any directory info that is already in memory */
    if( currdir->Head != NULL ) {
        __FreeTypeList( currdir );
    }

    /* seek to the start of the file */
    error = ( WRESSEEK( fid, 0, SEEK_SET ) == -1 );
    if( error ) {
        WRES_ERROR( WRS_SEEK_FAILED );
    }

    if( !error ) {
        restype = WResFindResType( fid );
        if( restype == RT_WATCOM ) {
            error = readWResDir( fid, currdir, fileinfo );
        } else if( restype == RT_WIN16 ) {
            error = readMResDir( fid, currdir, dup_discarded, false, fileinfo );
        } else {
            error = readMResDir( fid, currdir, dup_discarded, true, fileinfo );
        }
    }

    return( error );

} /* WResReadDir */
