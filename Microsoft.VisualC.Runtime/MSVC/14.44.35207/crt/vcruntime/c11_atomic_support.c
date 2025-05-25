// Copyright (c) Microsoft Corporation. All rights reserved.
//
// C11 atomic support routines: extern inline definitions
//
// C11 atomic support is currently gated behind `/experimental:c11atomics` so
// it isn't yet ABI locked. Once that flag is removed, this TU will have some 
// particular ABI considerations:
//   This TU is inserted into the VCRuntime static / import libraries, so it has
//   some particular ABI considerations. Notably we can't remove any of these
//   functions, or change their meaning or signature without breaking ABI for
//   "old" binaries that call them. We can add _new_ functions without restriction
//   since this TU isn't part of the redist.
#include <vcruntime_c11_atomic_support.h>

extern inline void _Check_memory_order(const unsigned int _Order);
extern inline void _Atomic_thread_fence(const unsigned int _Order);

extern inline void _Atomic_lock_acquire(volatile long* _Spinlock);
extern inline void _Atomic_lock_release(volatile long* _Spinlock);

extern inline void _Atomic_signal_fence(int _Order);
extern inline _Bool _Atomic_is_lock_free(size_t _Sz);

extern inline void _Atomic_store8(volatile char* _Ptr, char _Desired, int _Order);
extern inline void _Atomic_store16(volatile short* _Ptr, short _Desired, int _Order);
extern inline void _Atomic_store32(volatile int* _Ptr, int _Desired, int _Order);
extern inline void _Atomic_store64(volatile long long* _Ptr, long long _Desired, int _Order);
extern inline void _Atomic_storef(volatile float* _Ptr, float _Desired, int _Order);
extern inline void _Atomic_stored(volatile double* _Ptr, double _Desired, int _Order);

extern inline char _Atomic_load8(const volatile char* _Ptr, int _Order);
extern inline short _Atomic_load16(const volatile short* _Ptr, int _Order);
extern inline int _Atomic_load32(const volatile int* _Ptr, int _Order);
extern inline long long _Atomic_load64(const volatile long long* _Ptr, int _Order);
extern inline float _Atomic_loadf(const volatile float* _Ptr, int _Order);
extern inline double _Atomic_loadd(const volatile double* _Ptr, int _Order);

extern inline _Bool _Atomic_compare_exchange_strong8(volatile char* _Ptr, char* _Expected, char _Desired, int _Order);
extern inline _Bool _Atomic_compare_exchange_strong16(
    volatile short* _Ptr, short* _Expected, short _Desired, int _Order);
extern inline _Bool _Atomic_compare_exchange_strong32(volatile int* _Ptr, int* _Expected, int _Desired, int _Order);
extern inline _Bool _Atomic_compare_exchange_strong64(
    volatile long long* _Ptr, long long* _Expected, long long _Desired, int _Order);
extern inline _Bool _Atomic_compare_exchange_strongf(volatile float* _Ptr, float* _Expected, float _Desired, int _Order);
extern inline _Bool _Atomic_compare_exchange_strongd(
    volatile double* _Ptr, double* _Expected, double _Desired, int _Order);

extern inline char _Atomic_exchange8(volatile char* _Ptr, int _Desired, int _Order);
extern inline short _Atomic_exchange16(volatile short* _Ptr, int _Desired, int _Order);
extern inline int _Atomic_exchange32(volatile int* _Ptr, int _Desired, int _Order);
extern inline long long _Atomic_exchange64(volatile long long* _Ptr, long long _Desired, int _Order);
extern inline float _Atomic_exchangef(volatile float* _Ptr, float _Desired, int _Order);
extern inline double _Atomic_exchanged(volatile double* _Ptr, double _Desired, int _Order);

extern inline char _Atomic_fetch_add8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_fetch_add16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_fetch_add32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_fetch_add64(volatile long long* _Ptr, long long _Val, int _Order);
extern inline float _Atomic_fetch_addf(volatile float* _Ptr, float _Val, int _Order);
extern inline double _Atomic_fetch_addd(volatile double* _Ptr, double _Val, int _Order);

extern inline char _Atomic_add_fetch8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_add_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_add_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_add_fetch64(volatile long long* _Ptr, long long _Val, int _Order);
extern inline signed char _Atomic_add_fetchs8f(volatile signed char* _Ptr, float _Val, int _Order);
extern inline short _Atomic_add_fetchs16f(volatile short* _Ptr, float _Val, int _Order);
extern inline int _Atomic_add_fetchs32f(volatile int* _Ptr, float _Val, int _Order);
extern inline long long _Atomic_add_fetchs64f(volatile long long* _Ptr, float _Val, int _Order);
extern inline unsigned char _Atomic_add_fetchu8f(volatile unsigned char* _Ptr, float _Val, int _Order);
extern inline unsigned short _Atomic_add_fetchu16f(volatile unsigned short* _Ptr, float _Val, int _Order);
extern inline unsigned int _Atomic_add_fetchu32f(volatile unsigned int* _Ptr, float _Val, int _Order);
extern inline unsigned long long _Atomic_add_fetchu64f(volatile unsigned long long* _Ptr, float _Val, int _Order);
extern inline signed char _Atomic_add_fetchs8d(volatile signed char* _Ptr, double _Val, int _Order);
extern inline short _Atomic_add_fetchs16d(volatile short* _Ptr, double _Val, int _Order);
extern inline int _Atomic_add_fetchs32d(volatile int* _Ptr, double _Val, int _Order);
extern inline long long _Atomic_add_fetchs64d(volatile long long* _Ptr, double _Val, int _Order);
extern inline unsigned char _Atomic_add_fetchu8d(volatile unsigned char* _Ptr, double _Val, int _Order);
extern inline unsigned short _Atomic_add_fetchu16d(volatile unsigned short* _Ptr, double _Val, int _Order);
extern inline unsigned int _Atomic_add_fetchu32d(volatile unsigned int* _Ptr, double _Val, int _Order);
extern inline unsigned long long _Atomic_add_fetchu64d(volatile unsigned long long* _Ptr, double _Val, int _Order);
extern inline float _Atomic_add_fetchf(volatile float* _Ptr, float _Val, int _Order);
extern inline double _Atomic_add_fetchd(volatile double* _Ptr, double _Val, int _Order);
extern inline float _Atomic_add_fetchfd(volatile float* _Ptr, double _Val, int _Order);

extern inline char _Atomic_fetch_sub8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_fetch_sub16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_fetch_sub32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_fetch_sub64(volatile long long* _Ptr, long long _Val, int _Order);
extern inline float _Atomic_fetch_subf(volatile float* _Ptr, float _Val, int _Order);
extern inline double _Atomic_fetch_subd(volatile double* _Ptr, double _Val, int _Order);

extern inline char _Atomic_sub_fetch8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_sub_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_sub_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_sub_fetch64(volatile long long* _Ptr, long long _Val, int _Order);
extern inline signed char _Atomic_sub_fetchs8f(volatile signed char* _Ptr, float _Val, int _Order);
extern inline short _Atomic_sub_fetchs16f(volatile short* _Ptr, float _Val, int _Order);
extern inline int _Atomic_sub_fetchs32f(volatile int* _Ptr, float _Val, int _Order);
extern inline long long _Atomic_sub_fetchs64f(volatile long long* _Ptr, float _Val, int _Order);
extern inline unsigned char _Atomic_sub_fetchu8f(volatile unsigned char* _Ptr, float _Val, int _Order);
extern inline unsigned short _Atomic_sub_fetchu16f(volatile unsigned short* _Ptr, float _Val, int _Order);
extern inline unsigned int _Atomic_sub_fetchu32f(volatile unsigned int* _Ptr, float _Val, int _Order);
extern inline unsigned long long _Atomic_sub_fetchu64f(volatile unsigned long long* _Ptr, float _Val, int _Order);
extern inline signed char _Atomic_sub_fetchs8d(volatile signed char* _Ptr, double _Val, int _Order);
extern inline short _Atomic_sub_fetchs16d(volatile short* _Ptr, double _Val, int _Order);
extern inline int _Atomic_sub_fetchs32d(volatile int* _Ptr, double _Val, int _Order);
extern inline long long _Atomic_sub_fetchs64d(volatile long long* _Ptr, double _Val, int _Order);
extern inline unsigned char _Atomic_sub_fetchu8d(volatile unsigned char* _Ptr, double _Val, int _Order);
extern inline unsigned short _Atomic_sub_fetchu16d(volatile unsigned short* _Ptr, double _Val, int _Order);
extern inline unsigned int _Atomic_sub_fetchu32d(volatile unsigned int* _Ptr, double _Val, int _Order);
extern inline unsigned long long _Atomic_sub_fetchu64d(volatile unsigned long long* _Ptr, double _Val, int _Order);
extern inline float _Atomic_sub_fetchf(volatile float* _Ptr, float _Val, int _Order);
extern inline double _Atomic_sub_fetchd(volatile double* _Ptr, double _Val, int _Order);
extern inline float _Atomic_sub_fetchfd(volatile float* _Ptr, double _Val, int _Order);

extern inline char _Atomic_fetch_and8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_fetch_and16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_fetch_and32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_fetch_and64(volatile long long* _Ptr, long long _Val, int _Order);

extern inline char _Atomic_and_fetch8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_and_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_and_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_and_fetch64(volatile long long* _Ptr, long long _Val, int _Order);

extern inline char _Atomic_fetch_or8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_fetch_or16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_fetch_or32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_fetch_or64(volatile long long* _Ptr, long long _Val, int _Order);

extern inline char _Atomic_or_fetch8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_or_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_or_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_or_fetch64(volatile long long* _Ptr, long long _Val, int _Order);

extern inline char _Atomic_fetch_xor8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_fetch_xor16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_fetch_xor32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_fetch_xor64(volatile long long* _Ptr, long long _Val, int _Order);

extern inline char _Atomic_xor_fetch8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_xor_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_xor_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_xor_fetch64(volatile long long* _Ptr, long long _Val, int _Order);

extern inline char _Atomic_mult_fetch8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_mult_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_mult_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_mult_fetch64(volatile long long* _Ptr, long long _Val, int _Order);
extern inline signed char _Atomic_mult_fetchs8f(volatile signed char* _Ptr, float _Val, int _Order);
extern inline short _Atomic_mult_fetchs16f(volatile short* _Ptr, float _Val, int _Order);
extern inline int _Atomic_mult_fetchs32f(volatile int* _Ptr, float _Val, int _Order);
extern inline long long _Atomic_mult_fetchs64f(volatile long long* _Ptr, float _Val, int _Order);
extern inline unsigned char _Atomic_mult_fetchu8f(volatile unsigned char* _Ptr, float _Val, int _Order);
extern inline unsigned short _Atomic_mult_fetchu16f(volatile unsigned short* _Ptr, float _Val, int _Order);
extern inline unsigned int _Atomic_mult_fetchu32f(volatile unsigned int* _Ptr, float _Val, int _Order);
extern inline unsigned long long _Atomic_mult_fetchu64f(volatile unsigned long long* _Ptr, float _Val, int _Order);
extern inline signed char _Atomic_mult_fetchs8d(volatile signed char* _Ptr, double _Val, int _Order);
extern inline short _Atomic_mult_fetchs16d(volatile short* _Ptr, double _Val, int _Order);
extern inline int _Atomic_mult_fetchs32d(volatile int* _Ptr, double _Val, int _Order);
extern inline long long _Atomic_mult_fetchs64d(volatile long long* _Ptr, double _Val, int _Order);
extern inline unsigned char _Atomic_mult_fetchu8d(volatile unsigned char* _Ptr, double _Val, int _Order);
extern inline unsigned short _Atomic_mult_fetchu16d(volatile unsigned short* _Ptr, double _Val, int _Order);
extern inline unsigned int _Atomic_mult_fetchu32d(volatile unsigned int* _Ptr, double _Val, int _Order);
extern inline unsigned long long _Atomic_mult_fetchu64d(volatile unsigned long long* _Ptr, double _Val, int _Order);
extern inline float _Atomic_mult_fetchf(volatile float* _Ptr, float _Val, int _Order);
extern inline double _Atomic_mult_fetchd(volatile double* _Ptr, double _Val, int _Order);
extern inline float _Atomic_mult_fetchfd(volatile float* _Ptr, double _Val, int _Order);

extern inline unsigned char _Atomic_div_fetch8(volatile unsigned char* _Ptr, unsigned int _Val, int _Order);
extern inline unsigned short _Atomic_div_fetch16(volatile unsigned short* _Ptr, unsigned int _Val, int _Order);
extern inline unsigned int _Atomic_div_fetch32(volatile unsigned int* _Ptr, unsigned int _Val, int _Order);
extern inline unsigned long long _Atomic_div_fetch64( volatile unsigned long long* _Ptr, unsigned long long _Val, int _Order);
extern inline unsigned char _Atomic_div_fetch8_64(volatile unsigned char* _Ptr, unsigned long long _Val, int _Order);
extern inline unsigned short _Atomic_div_fetch16_64(volatile unsigned short* _Ptr, unsigned long long _Val, int _Order);
extern inline unsigned int _Atomic_div_fetch32_64(volatile unsigned int* _Ptr, unsigned long long _Val, int _Order);
extern inline signed char _Atomic_div_fetchs8f(volatile signed char* _Ptr, float _Val, int _Order);
extern inline short _Atomic_div_fetchs16f(volatile short* _Ptr, float _Val, int _Order);
extern inline int _Atomic_div_fetchs32f(volatile int* _Ptr, float _Val, int _Order);
extern inline long long _Atomic_div_fetchs64f(volatile long long* _Ptr, float _Val, int _Order);
extern inline unsigned char _Atomic_div_fetchu8f(volatile unsigned char* _Ptr, float _Val, int _Order);
extern inline unsigned short _Atomic_div_fetchu16f(volatile unsigned short* _Ptr, float _Val, int _Order);
extern inline unsigned int _Atomic_div_fetchu32f(volatile unsigned int* _Ptr, float _Val, int _Order);
extern inline unsigned long long _Atomic_div_fetchu64f(volatile unsigned long long* _Ptr, float _Val, int _Order);
extern inline signed char _Atomic_div_fetchs8d(volatile signed char* _Ptr, double _Val, int _Order);
extern inline short _Atomic_div_fetchs16d(volatile short* _Ptr, double _Val, int _Order);
extern inline int _Atomic_div_fetchs32d(volatile int* _Ptr, double _Val, int _Order);
extern inline long long _Atomic_div_fetchs64d(volatile long long* _Ptr, double _Val, int _Order);
extern inline unsigned char _Atomic_div_fetchu8d(volatile unsigned char* _Ptr, double _Val, int _Order);
extern inline unsigned short _Atomic_div_fetchu16d(volatile unsigned short* _Ptr, double _Val, int _Order);
extern inline unsigned int _Atomic_div_fetchu32d(volatile unsigned int* _Ptr, double _Val, int _Order);
extern inline unsigned long long _Atomic_div_fetchu64d(volatile unsigned long long* _Ptr, double _Val, int _Order);
extern inline float _Atomic_div_fetchf(volatile float* _Ptr, float _Val, int _Order);
extern inline double _Atomic_div_fetchd(volatile double* _Ptr, double _Val, int _Order);
extern inline float _Atomic_div_fetchfd(volatile float* _Ptr, double _Val, int _Order);
extern inline signed char _Atomic_divs_fetch8(volatile signed char* _Ptr, unsigned int _Val, int _Order);
extern inline short _Atomic_divs_fetch16(volatile short* _Ptr, unsigned int _Val, int _Order);
extern inline int _Atomic_divs_fetch32(volatile int* _Ptr, unsigned int _Val, int _Order);
extern inline long long _Atomic_divs_fetch64(volatile long long* _Ptr, unsigned long long _Val, int _Order);
extern inline signed char _Atomic_divs_fetch8_64(volatile signed char* _Ptr, unsigned long long _Val, int _Order);
extern inline short _Atomic_divs_fetch16_64(volatile short* _Ptr, unsigned long long _Val, int _Order);
extern inline int _Atomic_divs_fetch32_64(volatile int* _Ptr, unsigned long long _Val, int _Order);

extern inline signed char _Atomic_idiv_fetch8(volatile signed char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_idiv_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_idiv_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_idiv_fetch64(volatile long long* _Ptr, long long _Val, int _Order);
extern inline signed char _Atomic_idiv_fetch8_64(volatile signed char* _Ptr, long long _Val, int _Order);
extern inline short _Atomic_idiv_fetch16_64(volatile short* _Ptr, long long _Val, int _Order);
extern inline int _Atomic_idiv_fetch32_64(volatile int* _Ptr, long long _Val, int _Order);

extern inline unsigned char _Atomic_idivu_fetch8(volatile unsigned char* _Ptr, int _Val, int _Order);
extern inline unsigned short _Atomic_idivu_fetch16(volatile unsigned short* _Ptr, int _Val, int _Order);
extern inline unsigned int _Atomic_idivu_fetch32(volatile unsigned int* _Ptr, int _Val, int _Order);
extern inline unsigned long long _Atomic_idivu_fetch64(volatile unsigned long long* _Ptr, long long _Val, int _Order);
extern inline unsigned char _Atomic_idivu_fetch8_64(volatile unsigned char* _Ptr, long long _Val, int _Order);
extern inline unsigned short _Atomic_idivu_fetch16_64(volatile unsigned short* _Ptr, long long _Val, int _Order);
extern inline unsigned int _Atomic_idivu_fetch32_64(volatile unsigned int* _Ptr, long long _Val, int _Order);

extern inline char _Atomic_shl_fetch8(volatile char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_shl_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_shl_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_shl_fetch64(volatile long long* _Ptr, long long _Val, int _Order);

extern inline signed char _Atomic_sar_fetch8(volatile signed char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_sar_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_sar_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_sar_fetch64(volatile long long* _Ptr, int _Val, int _Order);

extern inline unsigned char _Atomic_shr2_fetch8(volatile unsigned char* _Ptr, int _Val, int _Order);
extern inline unsigned short _Atomic_shr2_fetch16(volatile unsigned short* _Ptr, int _Val, int _Order);
extern inline unsigned int _Atomic_shr2_fetch32(volatile unsigned int* _Ptr, int _Val, int _Order);
extern inline unsigned long long _Atomic_shr2_fetch64(volatile unsigned long long* _Ptr, int _Val, int _Order);

extern inline signed char _Atomic_imod_fetch8(volatile signed char* _Ptr, int _Val, int _Order);
extern inline short _Atomic_imod_fetch16(volatile short* _Ptr, int _Val, int _Order);
extern inline int _Atomic_imod_fetch32(volatile int* _Ptr, int _Val, int _Order);
extern inline long long _Atomic_imod_fetch64(volatile long long* _Ptr, long long _Val, int _Order);
extern inline signed char _Atomic_imod_fetch8_64(volatile signed char* _Ptr, long long _Val, int _Order);
extern inline short _Atomic_imod_fetch16_64(volatile short* _Ptr, long long _Val, int _Order);
extern inline int _Atomic_imod_fetch32_64(volatile int* _Ptr, long long _Val, int _Order);

extern inline unsigned char _Atomic_imodu_fetch8(volatile unsigned char* _Ptr, int _Val, int _Order);
extern inline unsigned short _Atomic_imodu_fetch16(volatile unsigned short* _Ptr, int _Val, int _Order);
extern inline unsigned int _Atomic_imodu_fetch32(volatile unsigned int* _Ptr, int _Val, int _Order);
extern inline unsigned long long _Atomic_imodu_fetch64(volatile unsigned long long* _Ptr, long long _Val, int _Order);
extern inline unsigned char _Atomic_imodu_fetch8_64(volatile unsigned char* _Ptr, long long _Val, int _Order);
extern inline unsigned short _Atomic_imodu_fetch16_64(volatile unsigned short* _Ptr, long long _Val, int _Order);
extern inline unsigned int _Atomic_imodu_fetch32_64(volatile unsigned int* _Ptr, long long _Val, int _Order);

extern inline unsigned char _Atomic_mod_fetch8(volatile unsigned char* _Ptr, unsigned int _Val, int _Order);
extern inline unsigned short _Atomic_mod_fetch16(volatile unsigned short* _Ptr, unsigned int _Val, int _Order);
extern inline unsigned int _Atomic_mod_fetch32(volatile unsigned int* _Ptr, unsigned int _Val, int _Order);
extern inline unsigned long long _Atomic_mod_fetch64(volatile unsigned long long* _Ptr, unsigned long long _Val, int _Order);
extern inline unsigned char _Atomic_mod_fetch8_64(volatile unsigned char* _Ptr, unsigned long long _Val, int _Order);
extern inline unsigned short _Atomic_mod_fetch16_64(volatile unsigned short* _Ptr, unsigned long long _Val, int _Order);
extern inline unsigned int _Atomic_mod_fetch32_64(volatile unsigned int* _Ptr, unsigned long long _Val, int _Order);
extern inline signed char _Atomic_mods_fetch8(volatile signed char* _Ptr, unsigned int _Val, int _Order);
extern inline short _Atomic_mods_fetch16(volatile short* _Ptr, unsigned int _Val, int _Order);
extern inline int _Atomic_mods_fetch32(volatile int* _Ptr, unsigned int _Val, int _Order);
extern inline long long _Atomic_mods_fetch64(volatile long long* _Ptr, unsigned long long _Val, int _Order);
extern inline signed char _Atomic_mods_fetch8_64(volatile signed char* _Ptr, unsigned long long _Val, int _Order);
extern inline short _Atomic_mods_fetch16_64(volatile short* _Ptr, unsigned long long _Val, int _Order);
extern inline int _Atomic_mods_fetch32_64(volatile int* _Ptr, unsigned long long _Val, int _Order);

extern inline void _Atomic_lock_and_store(volatile void* _Obj, const void* _Desired, int _Offset, size_t _Size);
extern inline void _Atomic_lock_and_load(volatile void* _Obj, void* _Dest, int _Offset, size_t _Size);
extern inline void _Atomic_lock_and_exchange(
    volatile void* _Obj, const void* _Desired, void* _Dest, int _Offset, size_t _Size);
extern inline _Bool _Atomic_lock_and_compare_exchange_strong(
    volatile void* _Obj, void* _Expected, const void* _Desired, int _Offset, size_t _Size);
