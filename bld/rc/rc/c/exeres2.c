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
* Description:  NE resource manipulation routines, OS/2 version.
*
****************************************************************************/


#include "global.h"
#include "errors.h"
#include "os2res.h"
#include "rcrtns.h"
#include "rccore.h"
#include "exeutil.h"
#include "exeres.h"


/* Note: IBM's OS/2 RC accepts string resource IDs/types but quietly
 * replaces all strings with zeros when writing out the resources to
 * NE modules. Strange thing to do, but we'll do the same.
 */


/* Build OS/2 NE style resource table. Each resource gets one entry, and
 * resources > 64K will get an additional entry for each 64K chunk.
 */
static void buildOS2ResTable( OS2ResTable *res_tbl, WResDir dir )
/***************************************************************/
{
    WResDirWindow   wind;
    WResLangInfo    *lang;
    OS2ResEntry     *entry;
    WResTypeInfo    *res_type;
    WResResInfo     *resinfo;
    uint_16         type;
    uint_16         id;
    int_32          length;

    entry = res_tbl->resources;

    /* Walk through the WRes directory */
    for( wind = WResFirstResource( dir ); !WResIsEmptyWindow( wind ); wind = WResNextResource( wind, dir ) ) {
        lang = WResGetLangInfo( wind );
        res_type = WResGetTypeInfo( wind );
        resinfo  = WResGetResInfo( wind );

        // RT_DEFAULTICON is not written into the executable, ignore
        if( res_type->TypeName.ID.Num == OS2_RT_DEFAULTICON ) {
            wind = WResNextResource( wind, dir );
            continue;
        }

        if( res_type->TypeName.IsName )
            type = 0;
        else
            type = res_type->TypeName.ID.Num;

        if( resinfo->ResName.IsName )
            id = 0;
        else
            id = resinfo->ResName.ID.Num;

        /* Fill in resource entries */
        entry->res_type   = type;
        entry->res_id     = id;
        entry->wind       = wind;
        entry->mem_flags  = lang->MemoryFlags;
        entry->seg_length = 0;  /* Zero means 64K */
        entry->first_part = true;

        for( length = lang->Length; length > 0x10000; length -= 0x10000 ) {
            entry++;
            entry->res_type   = type;
            entry->res_id     = id;
            entry->wind       = wind;
            entry->mem_flags  = lang->MemoryFlags;
            entry->seg_length = 0;
            entry->first_part = false;
        }
        entry->seg_length = lang->Length % 0x10000;
        entry++;
    }
}


static int compareOS2ResTypeId( const void * _entry1, const void * _entry2 )
/*************************************************************************/
{
    const LXResEntry *entry1 = _entry1;
    const LXResEntry *entry2 = _entry2;

    if( entry1->resource.type_id == entry2->resource.type_id ) {
        return( entry1->resource.name_id - entry2->resource.name_id );
    } else {
        return( entry1->resource.type_id - entry2->resource.type_id );
    }
} /* compareOS2ResTypeId */


extern RcStatus InitOS2ResTable( int *err_code )
/**********************************************/
{
    OS2ResTable         *res;
    WResDir             dir;

    res = &(Pass2Info.TmpFile.u.NEInfo.OS2Res);
    dir = Pass2Info.ResFile->Dir;

    if( CmdLineParms.NoResFile ) {
        res->resources    = NULL;
        res->num_res_segs = 0;
        res->table_size   = 0;
    } else {
        res->num_res_segs = ComputeOS2ResSegCount( dir );
        /* One resource type/id record per resource segment, 16-bits each */
        res->table_size   = res->num_res_segs * 2 * sizeof( uint_16 );

        res->resources = RCALLOC( res->num_res_segs * sizeof( res->resources[0] ) );
        if( res->resources == NULL ) {
            *err_code = errno;
            return( RS_NO_MEM );
        }
        buildOS2ResTable( res, dir );

        /* OS/2 requires resources to be sorted */
        qsort( res->resources, res->num_res_segs,
                sizeof( res->resources[0] ), compareOS2ResTypeId );

    }
    return( RS_OK );
} /* InitOS2ResTable */


/* Compute the number of resource segments in an OS/2 NE module. Each
 * resource gets its own segment and resources > 64K will be split into
 * as many segments as necessary.
 */
extern uint_32 ComputeOS2ResSegCount( WResDir dir )
/*************************************************/
{
    uint_32         length;
    WResDirWindow   wind;
    WResTypeInfo    *res_type;
    WResLangInfo    *res;
    uint_32         num_res_segs;

    num_res_segs = 0;
    for( wind = WResFirstResource( dir ); !WResIsEmptyWindow( wind ); wind = WResNextResource( wind, dir ) ) {
        res = WResGetLangInfo( wind );
        res_type = WResGetTypeInfo( wind );

        // RT_DEFAULTICON is not written into the executable, ignore
        if( res_type->TypeName.ID.Num != OS2_RT_DEFAULTICON ) {
            ++num_res_segs;
            for( length = res->Length; length > 0x10000; length -= 0x10000 ) {
                ++num_res_segs;
            }
        }
    }
    return( num_res_segs );
} /* ComputeOS2ResSegCount */


/* NB: We copy resources in one go even if they span multiple segments.
 * This is fine because all segments but the last one are 64K big, and
 * hence will be nicely aligned.
 */
static RcStatus copyOneResource( WResLangInfo *lang, WResFileID res_fid,
            WResFileID out_fid, int shift_count, int *err_code )
/**********************************************************************/
{
    RcStatus            ret;
    long                out_offset;
    long                align_amount;

    /* align the output file to a boundary for shift_count */
    ret = RS_OK;
    out_offset = RCTELL( out_fid );
    if( out_offset == -1 ) {
        ret = RS_WRITE_ERROR;
        *err_code = errno;
    }
    if( ret == RS_OK ) {
        align_amount = AlignAmount( out_offset, shift_count );
        if( RCSEEK( out_fid, align_amount, SEEK_CUR ) == -1 ) {
            ret = RS_WRITE_ERROR;
            *err_code = errno;
        }
        out_offset += align_amount;
    }

    if( ret == RS_OK ) {
        if( RCSEEK( res_fid, lang->Offset, SEEK_SET ) == -1 ) {
            ret = RS_READ_ERROR;
            *err_code = errno;
        }
    }
    if( ret == RS_OK ) {
        ret = CopyExeData( res_fid, out_fid, lang->Length );
        *err_code = errno;
    }
    if( ret == RS_OK ) {
        align_amount = AlignAmount( RCTELL( out_fid ), shift_count );
        ret = PadExeData( out_fid, align_amount );
        *err_code = errno;
    }

    return( ret );
} /* copyOneResource */


RcStatus CopyOS2Resources( void )
{
    OS2ResEntry         *entry;
    WResDirWindow       wind;
    OS2ResTable         *restab;
    WResLangInfo        *lang;
    WResFileID          tmp_fid;
    WResFileID          res_fid;
    RcStatus            ret;
    int                 err_code;
    int                 shift_count;
    int                 currseg;
    segment_record      *tmpseg;
    uint_32             seg_offset;
    long                align_amount;
    int                 i;

    restab    = &(Pass2Info.TmpFile.u.NEInfo.OS2Res);
    tmp_fid   = Pass2Info.TmpFile.fid;
    res_fid   = Pass2Info.ResFile->fid;
    tmpseg    = Pass2Info.TmpFile.u.NEInfo.Seg.Segments;
    currseg   = Pass2Info.OldFile.u.NEInfo.Seg.NumSegs
                - Pass2Info.OldFile.u.NEInfo.Seg.NumOS2ResSegs;
    entry     = restab->resources;
    ret       = RS_OK;
    err_code  = 0;

    tmpseg += currseg;
    shift_count = Pass2Info.TmpFile.u.NEInfo.WinHead.align;
    seg_offset = 0;     // shut up gcc

    /* We may need to add padding before the first resource segment */
    align_amount = AlignAmount( RCTELL( tmp_fid ), shift_count );
    if( align_amount ) {
        ret = PadExeData( tmp_fid, align_amount );
        err_code = errno;
    }

    /* Walk through the resource entries */
    for( i = 0; i < restab->num_res_segs; i++, entry++, tmpseg++ ) {
        wind = entry->wind;
        lang = WResGetLangInfo( wind );

        if( entry->first_part ) {
            seg_offset = RCTELL( tmp_fid );
        } else {
            seg_offset += 0x10000;
        }

        /* Fill in segment structure */
        tmpseg->address = seg_offset >> shift_count;
        tmpseg->size    = entry->seg_length;
        tmpseg->min     = entry->seg_length;
        tmpseg->info    = SEG_DATA | SEG_READ_ONLY | SEG_LEVEL_3;
        if( entry->mem_flags & MEMFLAG_MOVEABLE )
            tmpseg->info |= SEG_MOVABLE;
        if( entry->mem_flags & MEMFLAG_PURE )
            tmpseg->info |= SEG_PURE;
        if( entry->mem_flags & MEMFLAG_PRELOAD )
            tmpseg->info |= SEG_PRELOAD;
        if( entry->mem_flags & MEMFLAG_DISCARDABLE )
            tmpseg->info |= SEG_DISCARD;

        /* For non-last segment of a resource, there's nothing to copy */
        if( !entry->first_part )
            continue;

        /* Copy resource data */
        ret = copyOneResource( lang, res_fid, tmp_fid,
                                shift_count, &err_code );

        if( ret != RS_OK )
            break;

        CheckDebugOffset( &(Pass2Info.TmpFile) );
    }

    switch( ret ) {
    case RS_WRITE_ERROR:
        RcError( ERR_WRITTING_FILE, Pass2Info.TmpFile.name, strerror( err_code ) );
        break;
    case RS_READ_ERROR:
        RcError( ERR_READING_RES, CmdLineParms.OutResFileName, strerror( err_code ) );
        break;
    case RS_READ_INCMPLT:
        RcError( ERR_UNEXPECTED_EOF, CmdLineParms.OutResFileName );
        break;
    default:
        break;
    }
    return( ret );
} /* CopyOS2Resources */


/*
 * WriteOS2ResTable
 * NB when an error occurs this function must return without altering errno
 */
extern RcStatus WriteOS2ResTable( WResFileID fid, OS2ResTable *restab, int *err_code )
/************************************************************************************/
{
    RcStatus                    ret;
    uint_16                     res_type;
    uint_16                     res_id;
    int                         i;

    ret = RS_OK;
    for( i = 0; i < restab->num_res_segs && ret == RS_OK; i++ ) {
        res_type = restab->resources[i].res_type;
        res_id   = restab->resources[i].res_id;
        if( RCWRITE( fid, &res_type, sizeof( uint_16 ) ) != sizeof( uint_16 ) ) {
            ret = RS_WRITE_ERROR;
        } else {
            if( RCWRITE( fid, &res_id, sizeof( uint_16 ) ) != sizeof( uint_16 ) ) {
                ret = RS_WRITE_ERROR;
            }
        }
    }

    *err_code = errno;
    if( restab->resources != NULL ) {
        RCFREE( restab->resources );
    }

    return( ret );
} /* WriteOS2ResTable */
