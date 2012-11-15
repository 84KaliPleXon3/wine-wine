/*
 * Copyright 2012 Hans Leidekker for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include <stdarg.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "objbase.h"
#include "wincodec.h"

#include "wincodecs_private.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(wincodecs);

typedef struct ColorContext {
    IWICColorContext IWICColorContext_iface;
    LONG ref;
    WICColorContextType type;
    BYTE *profile;
    UINT profile_len;
    UINT exif_color_space;
} ColorContext;

static inline ColorContext *impl_from_IWICColorContext(IWICColorContext *iface)
{
    return CONTAINING_RECORD(iface, ColorContext, IWICColorContext_iface);
}

static HRESULT WINAPI ColorContext_QueryInterface(IWICColorContext *iface, REFIID iid,
    void **ppv)
{
    ColorContext *This = impl_from_IWICColorContext(iface);
    TRACE("(%p,%s,%p)\n", iface, debugstr_guid(iid), ppv);

    if (!ppv) return E_INVALIDARG;

    if (IsEqualIID(&IID_IUnknown, iid) ||
        IsEqualIID(&IID_IWICColorContext, iid))
    {
        *ppv = &This->IWICColorContext_iface;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    IUnknown_AddRef((IUnknown*)*ppv);
    return S_OK;
}

static ULONG WINAPI ColorContext_AddRef(IWICColorContext *iface)
{
    ColorContext *This = impl_from_IWICColorContext(iface);
    ULONG ref = InterlockedIncrement(&This->ref);

    TRACE("(%p) refcount=%u\n", iface, ref);

    return ref;
}

static ULONG WINAPI ColorContext_Release(IWICColorContext *iface)
{
    ColorContext *This = impl_from_IWICColorContext(iface);
    ULONG ref = InterlockedDecrement(&This->ref);

    TRACE("(%p) refcount=%u\n", iface, ref);

    if (ref == 0)
    {
        HeapFree(GetProcessHeap(), 0, This->profile);
        HeapFree(GetProcessHeap(), 0, This);
    }

    return ref;
}

static HRESULT WINAPI ColorContext_InitializeFromFilename(IWICColorContext *iface,
    LPCWSTR wzFilename)
{
    FIXME("(%p,%s)\n", iface, debugstr_w(wzFilename));
    return E_NOTIMPL;
}

static HRESULT WINAPI ColorContext_InitializeFromMemory(IWICColorContext *iface,
    const BYTE *pbBuffer, UINT cbBufferSize)
{
    ColorContext *This = impl_from_IWICColorContext(iface);
    TRACE("(%p,%p,%u)\n", iface, pbBuffer, cbBufferSize);

    if (This->type != WICColorContextUninitialized)
        return WINCODEC_ERR_WRONGSTATE;

    HeapFree(GetProcessHeap(), 0, This->profile);
    if (!(This->profile = HeapAlloc(GetProcessHeap(), 0, cbBufferSize)))
        return E_OUTOFMEMORY;

    memcpy(This->profile, pbBuffer, cbBufferSize);
    This->profile_len = cbBufferSize;
    This->type = WICColorContextProfile;

    return S_OK;
}

static HRESULT WINAPI ColorContext_InitializeFromExifColorSpace(IWICColorContext *iface,
    UINT value)
{
    ColorContext *This = impl_from_IWICColorContext(iface);
    TRACE("(%p,%u)\n", iface, value);

    if (This->type != WICColorContextUninitialized && This->type != WICColorContextExifColorSpace)
        return WINCODEC_ERR_WRONGSTATE;

    This->exif_color_space = value;
    This->type = WICColorContextExifColorSpace;

    return S_OK;
}

static HRESULT WINAPI ColorContext_GetType(IWICColorContext *iface,
    WICColorContextType *pType)
{
    ColorContext *This = impl_from_IWICColorContext(iface);
    TRACE("(%p,%p)\n", iface, pType);

    *pType = This->type;
    return S_OK;
}

static HRESULT WINAPI ColorContext_GetProfileBytes(IWICColorContext *iface,
    UINT cbBuffer, BYTE *pbBuffer, UINT *pcbActual)
{
    FIXME("(%p,%u,%p,%p)\n", iface, cbBuffer, pbBuffer, pcbActual);
    return E_NOTIMPL;
}

static HRESULT WINAPI ColorContext_GetExifColorSpace(IWICColorContext *iface,
    UINT *pValue)
{
    ColorContext *This = impl_from_IWICColorContext(iface);
    TRACE("(%p,%p)\n", iface, pValue);

    *pValue = This->exif_color_space;
    return S_OK;
}

static const IWICColorContextVtbl ColorContext_Vtbl = {
    ColorContext_QueryInterface,
    ColorContext_AddRef,
    ColorContext_Release,
    ColorContext_InitializeFromFilename,
    ColorContext_InitializeFromMemory,
    ColorContext_InitializeFromExifColorSpace,
    ColorContext_GetType,
    ColorContext_GetProfileBytes,
    ColorContext_GetExifColorSpace
};

HRESULT ColorContext_Create(IWICColorContext **colorcontext)
{
    ColorContext *This;

    This = HeapAlloc(GetProcessHeap(), 0, sizeof(ColorContext));
    if (!This) return E_OUTOFMEMORY;

    This->IWICColorContext_iface.lpVtbl = &ColorContext_Vtbl;
    This->ref = 1;
    This->type = 0;
    This->profile = NULL;
    This->profile_len = 0;
    This->exif_color_space = ~0u;

    *colorcontext = &This->IWICColorContext_iface;

    return S_OK;
}