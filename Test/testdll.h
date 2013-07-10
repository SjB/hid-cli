#ifdef NCADLL_EXPORTS
#define NCA_API __declspec(dllexport)
#else
#define NCA_API __declspec(dllimport)
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

NCA_API void nca_test_dll(int data);
NCA_API int nca_plus(int a, int b);

#if defined(__cplusplus) || defined(c_plusplus)
} // extern "C"
#endif
