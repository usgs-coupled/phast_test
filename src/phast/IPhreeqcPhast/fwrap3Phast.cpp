#include "IPhreeqc.h"
#include "fwrapPhast.h"

#if defined(_WIN32)

#if defined(__cplusplus)
extern "C" {
#endif

//
// Intel Fortran compiler 9.1 /iface:cref /assume:underscore
//

IPQ_DLL_EXPORT int  createiphreeqcphast_(void)
{
	return CreateIPhreeqcPhastF();
}
IPQ_DLL_EXPORT int  destroyiphreeqcphast_(int *id)
{
	return DestroyIPhreeqcPhastF(id);
}

#if defined(__cplusplus)
}
#endif

#endif
