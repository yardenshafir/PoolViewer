#include "engextcpp.hpp"
#include "..\PoolData\PoolData.cpp"

#pragma once

class EXT_CLASS : public ExtExtension
{
public:
    EXT_COMMAND_METHOD(poolview);
    virtual HRESULT Initialize(void);
    virtual void Uninitialize(void);
};

EXT_DECLARE_GLOBALS();

UINT g_helpMenu = 0;

EXT_COMMAND(poolview, "Parses the kernel pool", "{;e64,o;address;input address}{tag;s;str;Pool tag to use}")
{
    ULONG64 address;
    PCSTR tag;
    if (HasArg("tag"))
    {
        tag = GetArgStr("tag", false);
        if (strlen(tag) > 4)
        {
            g_DebugControl->Output(DEBUG_OUTPUT_ERROR, "Pool tag can be up to 4 bytes\n");
            return;
        }
        g_DebugControl->Output(DEBUG_OUTPUT_DEBUGGEE, "  Address              Size       (Status)      Tag    Type\n");
        g_DebugControl->Output(DEBUG_OUTPUT_DEBUGGEE, "  ---------------------------------------------------------\n");
        GetAllHeaps(tag);
        return;
    }
    else
    {
        address = GetUnnamedArgU64(0);
        if (address)
        {
            g_DebugControl->Output(DEBUG_OUTPUT_DEBUGGEE, "  Address              Size       (Status)      Tag    Type\n");
            g_DebugControl->Output(DEBUG_OUTPUT_DEBUGGEE, "  ---------------------------------------------------------\n");
            GetPoolDataForAddress((PVOID)address);
            return;
        }
    }
}

HRESULT EXT_CLASS::Initialize(void)
{
    HRESULT hResult;

    hResult = InitializeDebugGlobals();

    if ((!SUCCEEDED(GetTypes())) ||
        (!SUCCEEDED(GetOffsets())) ||
        (!SUCCEEDED(GetSizes())) ||
        (!SUCCEEDED(GetHeapGlobals())))
    {
        return S_FALSE;
    }

    return hResult;
}

void EXT_CLASS::Uninitialize(void)
{
    UninitializeDebugGlobals();

    return;
}