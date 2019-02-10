#ifndef LIBDEF_COMLIB_H_H_
#define LIBDEF_COMLIB_H_H_

#ifdef COMLIB_EXPORTS
#define COMAPI _declspec(dllexport)
#else
#define COMAPI
#endif
#endif //LIBDEF_COMLIB_H_H_