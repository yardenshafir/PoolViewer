#include <Windows.h>
#include <list>

#pragma once

using namespace std;

//
// Enums
//

enum RTLP_CSPARSE_BITMAP_STATE
{
    CommitBitmapInvalid = 0x0,
    UserBitmapInvalid = 0x1,
    UserBitmapValid = 0x2,
};

enum ALLOCATION_TYPE
{
    Lfh = 0x0,
    Vs = 0x1,
    Large = 0x2,
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
    ULONG DescArrayOffset;
};

struct HEAP_PAGE_RANGE_DESCRIPTOR_OFFSET
{
    ULONG RangeFlagsOffset;
    ULONG UnitSizeOffset;
};

struct SEGMENT_HEAP_OFFSETS
{
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

int
GetPoolInformation (
    _In_ char* FilePath,
    _Outptr_ int* numberOfHeaps
);