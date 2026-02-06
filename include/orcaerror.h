#ifndef __ORCAERROR_H__
#define __ORCAERROR_H__

#ifndef __COREFOUNDATION_CFPLUGINCOM__

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#define S_OK ((HRESULT)0x00000000L)
#define S_FALSE ((HRESULT)0x00000001L)

/* Macros for more detailed HRESULT analysis */
#define IS_ERROR(Status) ((unsigned long)(Status) >> 31 == SEVERITY_ERROR)
#define HRESULT_CODE(hr) ((hr) & 0xFFFF)
#define HRESULT_FACILITY(hr) (((hr) >> 16) & 0x1fff)
#define HRESULT_SEVERITY(hr) (((hr) >> 31) & 0x1)
#define SEVERITY_SUCCESS 0
#define SEVERITY_ERROR 1

/* Creating an HRESULT from its component pieces */
#define MAKE_HRESULT(sev, fac, code)                                           \
  ((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) |     \
             ((unsigned long)(code))))

#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)
#define E_NOTIMPL ((HRESULT)0x80000001L)
#define E_OUTOFMEMORY ((HRESULT)0x80000002L)
#define E_INVALIDARG ((HRESULT)0x80000003L)
#define E_NOINTERFACE ((HRESULT)0x80000004L)
#define E_POINTER ((HRESULT)0x80000005L)
#define E_HANDLE ((HRESULT)0x80000006L)
#define E_ABORT ((HRESULT)0x80000007L)
#define E_FAIL ((HRESULT)0x80000008L)
#define E_ACCESSDENIED ((HRESULT)0x80000009L)

#endif

#define E_ITEMNOTFOUND ((HRESULT)0x80001008L)

#define NOERROR S_OK

#endif
