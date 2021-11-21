#include <Windows.h>
#include <winternl.h>
#include <DbgEng.h>
#include <list>
#include <stdio.h>
#include <string>

#pragma once

using namespace std;

//
// Enums
//

extern PDEBUG_CLIENT g_DebugClient;
extern PDEBUG_SYMBOLS g_DebugSymbols;
extern PDEBUG_DATA_SPACES4 g_DataSpaces;
extern PDEBUG_CONTROL g_DebugContro;

enum class RTLP_CSPARSE_BITMAP_STATE
{
    CommitBitmapInvalid = 0x0,
    UserBitmapInvalid = 0x1,
    UserBitmapValid = 0x2,
};

enum class ALLOCATION_TYPE
{
    Lfh = 0x0,
    Vs = 0x1,
    Large = 0x2,
    Big = 0x3
};

typedef _Enum_is_bitflag_ enum _POOL_TYPE {
    NonPagedPool,
    NonPagedPoolExecute = NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed = NonPagedPool + 2,
    DontUseThisType,
    NonPagedPoolCacheAligned = NonPagedPool + 4,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS = NonPagedPool + 6,
    MaxPoolType,

    //
    // Define base types for NonPaged (versus Paged) pool, for use in cracking
    // the underlying pool type.
    //

    NonPagedPoolBase = 0,
    NonPagedPoolBaseMustSucceed = NonPagedPoolBase + 2,
    NonPagedPoolBaseCacheAligned = NonPagedPoolBase + 4,
    NonPagedPoolBaseCacheAlignedMustS = NonPagedPoolBase + 6,

    //
    // Note these per session types are carefully chosen so that the appropriate
    // masking still applies as well as MaxPoolType above.
    //

    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

    NonPagedPoolNx = 512,
    NonPagedPoolNxCacheAligned = NonPagedPoolNx + 4,
    NonPagedPoolSessionNx = NonPagedPoolNx + 32,

} _Enum_is_bitflag_ POOL_TYPE;

//
// Needed offsets inside structures
//

struct HEAP_MANAGER_STATE_OFFSETS
{
    ULONG HeapManagerOffset;
    ULONG PoolNodeOffset;
    ULONG NumberOfPoolsOffset;
    ULONG SpecialHeapsOffset;
};

struct RTLP_HP_HEAP_MANAGER_OFFSETS
{
    ULONG AllocTrackerOffset;
    ULONG VaMgrOffset;
};

struct RTLP_HP_ALLOC_TRACKER_OFFSETS
{
    ULONG AllocTrackerBitmapOffset;
    ULONG BaseAddressOffset;
};

struct RTL_CSPARSE_BITMAP_OFFSETS
{
    ULONG CommitDirectoryOffset;
    ULONG CommitBitmapOffset;
    ULONG UserBitmapOffset;
};

struct HEAP_VAMGR_CTX_OFFSETS
{
    ULONG VaSpaceOffset;
};

struct HEAP_VAMGR_VASPACE_OFFSETS
{
    ULONG VaRangeArrayOffset;
    ULONG BaseAddressOffset;
};

struct RTL_SPARSE_ARRAY_OFFSETS
{
    ULONG ElementSizeShiftOffset;
    ULONG BitmapOffset;
};

struct HEAP_VAMGR_RANGE_OFFSETS
{
    ULONG AllocatedOffset;
};

struct HEAP_SEG_CONTEXT_OFFSETS
{
    ULONG SegmentListHeadOffset;
    ULONG FirstDescriptorIndexOffset;
    ULONG SegmentCountOffset;
    ULONG UnitShiftOffset;
};

struct HEAP_PAGE_SEGMENT_OFFSETS
{
    ULONG ListEntryOffset;
    ULONG SignatureOffset;
    ULONG SegmentCommitStateOffset;
    ULONG DescArrayOffset;
};

struct HEAP_PAGE_RANGE_DESCRIPTOR_OFFSET
{
    ULONG TreeSignatureOffset;
    ULONG RangeFlagsOffset;
    ULONG UnitSizeOffset;
};

struct SEGMENT_HEAP_OFFSETS
{
    ULONG LargeAllocMetadataOffset;
    ULONG SegContextsOffsets;
};

struct EX_HEAP_POOL_NODE_OFFSETS
{
    ULONG HeapsOffset;
};

struct HEAP_VS_SUBSEGMENT_OFFSETS
{
    ULONG SignatureOffset;
    ULONG SizeOffset;
};

struct HEAP_VS_CHUNK_HEADER_OFFSETS
{
    ULONG SizesOffset;
};

struct HEAP_VS_CHUNK_HEADER_SIZE_OFFSETS
{
    ULONG HeaderBitsOffset;
    ULONG UnsafeSizeOffset;
    ULONG AllocatedOffset;
};

struct POOL_HEADER_OFFSETS
{
    ULONG PoolTagOffset;
};

struct HEAP_LFH_SUBSEGMENT_OFFSETS
{
    ULONG BlockOffsets;
    ULONG BlockBitmap;
};

struct HEAP_LFH_SUBSEGMENT_ENCODED_OFFSETS_OFFSETS
{
    ULONG EncodedData;
    ULONG BlockSize;
    ULONG FirstBlockOffset;
};

struct POOL_TRACKER_BIG_PAGES_OFFSETS
{
    ULONG Va;
    ULONG Key;
    ULONG NumberOfBytes;
};

struct HEAP_LARGE_ALLOC_DATA_OFFSETS
{
    ULONG TreeNodeOffset;
    ULONG VirtualAddressOffset;
};

struct RTL_RB_TREE_OFFSETS
{
    ULONG RootOffset;
    ULONG EncodedOffset;
};

struct RTL_BALANCED_NODE_OFFSETS
{
    ULONG LeftOffset;
    ULONG RightOffset;
};

struct ALLOC
{
    ULONG Size;
    string PoolTag;
    BOOLEAN Allocated;
    ULONG64 Address;
    ALLOCATION_TYPE Type;
};

struct HEAP
{
    ULONG NodeNumber;
    BOOLEAN Special;
    POOL_TYPE PoolType;
    ULONG64 NumberOfAllocations;
    list<ALLOC> Allocations;
    ULONG64 Address;
};

struct POOL_VIEW_FLAGS {
    union {
        ULONG AllFlags;
        struct {
            ULONG OnlyPaged : 1;
            ULONG OnlyNonPaged : 1;
        };
    };
};

//
// structs needed to create live dmp
//

typedef union _SYSDBG_LIVEDUMP_CONTROL_FLAGS
{
    struct
    {
        ULONG UseDumpStorageStack : 1;
        ULONG CompressMemoryPagesData : 1;
        ULONG IncludeUserSpaceMemoryPages : 1;
        ULONG Reserved : 29;
    };
    ULONG AsUlong;
} SYSDBG_LIVEDUMP_CONTROL_FLAGS;

typedef union _SYSDBG_LIVEDUMP_CONTROL_ADDPAGES
{
    struct
    {
        ULONG HypervisorPages : 1;
        ULONG Reserved : 31;
    };
    ULONG AsUlong;
} SYSDBG_LIVEDUMP_CONTROL_ADDPAGES;

typedef struct _SYSDBG_LIVEDUMP_CONTROL
{
    ULONG Version;
    ULONG BugCheckCode;
    ULONG_PTR BugCheckParam1;
    ULONG_PTR BugCheckParam2;
    ULONG_PTR BugCheckParam3;
    ULONG_PTR BugCheckParam4;
    PVOID DumpFileHandle;
    PVOID CancelEventHandle;
    SYSDBG_LIVEDUMP_CONTROL_FLAGS Flags;
    SYSDBG_LIVEDUMP_CONTROL_ADDPAGES AddPagesControl;
} SYSDBG_LIVEDUMP_CONTROL, * PSYSDBG_LIVEDUMP_CONTROL;

typedef
NTSTATUS
(__stdcall*
    NtSystemDebugControl) (
        ULONG ControlCode,
        PVOID InputBuffer,
        ULONG InputBufferLength,
        PVOID OutputBuffer,
        ULONG OutputBufferLength,
        PULONG ReturnLength
        );

//
// Externally-used functions
//

char*
GetNextAllocation (
    _Outptr_ ULONG64* Address,
    _Outptr_ int* Size,
    _Outptr_ bool* Allocated,
    _Outptr_ int* Type
);

bool
GetNextHeapInformation (
    _Outptr_ ULONG64* Address,
    _Outptr_ int* NodeNumber,
    _Outptr_ long* NumberOfAllocations,
    _Outptr_ int* PoolType,
    _Outptr_ bool* Special
);

HRESULT
GetPoolInformation (
    _In_ char* FilePath,
    _In_ bool CreateLiveDump,
    _Outptr_ int* numberOfHeaps
);

HRESULT
InitializeDebugGlobals(
    void
);

VOID
UninitializeDebugGlobals(
    void
);

std::list<HEAP>
GetAllHeaps(
    _In_opt_ PCSTR Tag,
    _In_opt_ POOL_VIEW_FLAGS Flags
);

VOID
GetPoolDataForAddress(
    _In_ PVOID Address
);

HRESULT
GetTypes(
    void
);

HRESULT
GetOffsets(
    void
);

HRESULT
GetSizes(
    void
);

HRESULT
GetHeapGlobals(
    void
);

typedef HRESULT(*DebugCreateFunc)(_In_ REFIID, _Out_ PVOID*);