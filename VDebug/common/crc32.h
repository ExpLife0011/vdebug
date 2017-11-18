#ifndef _H_GDCRC32
#define _H_GDCRC32

#ifdef __cplusplus
extern "C" {
#endif

unsigned long _gd_crc32(const char* buffer, int bufSize, unsigned long seed);

#define std_crc32(b, l)     _gd_crc32((const char*)b, l, 0xffffffff)
#define gd_crc32(b, l, s)   _gd_crc32((const char*)b, l, s)

#ifdef __cplusplus
}
#endif

#endif // #ifndef _H_GDCRC32
