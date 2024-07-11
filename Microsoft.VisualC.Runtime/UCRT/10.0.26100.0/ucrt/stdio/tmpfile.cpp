//
// tmpfile.cpp
//
//      Copyright (c) Microsoft Corporation. All rights reserved.
//
// Defines tmpfile() and tmpfile_s(), which create temporary files, and tmpnam()
// and tmpnam_s(), which generate temporary file names (and are used by the
// tmpfile() functions).
//
#include <corecrt_internal_stdio.h>
#include <sys/stat.h>



// These are static buffers used by tmpnam() and tmpfile() to build file names.
// The sizes are computed as follows:  The tmpname string looks like so:
//
//     PrefixFirstPart.SecondPart
//
// Prefix is "s" (1 character in length).  FirstPart is generated by converting
// the process identifier to a base 32 string.  The maximum process identifier
// so converted is 7 characters in length.  The "." is one character in length.
// This gives a total of 1 + 7 + 1 = 9 for the "PrefixFirstPart." part of the
// file name.
//
// The SecondPart is generated by converting a number to a string.  In tmpnam,
// the maximum number is SHRT_MAX, the maximum length of which is 3 characters.
// In tmpnam_s, the maximum number is INT_MAX, which is 7 characters long.
//
// The tmpnam paths are generated relative to "\\", so the total lengths for the
// two functions are the length of that string plus 12 or plus 16 characters.
//
// The tmpfile path is generated in a manner similar to tmpnam, but relative to
// an arbitrary path, so MAX_PATH is used in lieu of _countof("\\").
namespace {

    enum class buffer_id
    {
        tmpnam,
        tmpfile,
        tmpnam_s,
        count
    };
}

static char*    narrow_tmpfile_buffer_pointers[static_cast<size_t>(buffer_id::count)];
static wchar_t* wide_tmpfile_buffer_pointers  [static_cast<size_t>(buffer_id::count)];

extern "C" void __cdecl __acrt_stdio_free_tmpfile_name_buffers_nolock()
{
    for (char*& p : narrow_tmpfile_buffer_pointers)
    {
        _free_crt(p);
        p = nullptr;
    }

    for (wchar_t*& p : wide_tmpfile_buffer_pointers)
    {
        _free_crt(p);
        p = nullptr;
    }
}

static char*& __cdecl get_tmpfile_buffer_pointer_nolock(buffer_id const id, char) throw()
{
    return narrow_tmpfile_buffer_pointers[static_cast<size_t>(id)];
}

static wchar_t*& __cdecl get_tmpfile_buffer_pointer_nolock(buffer_id const id, wchar_t) throw()
{
    return wide_tmpfile_buffer_pointers[static_cast<size_t>(id)];
}

template <typename Character>
_Success_(return != nullptr)
static Character* __cdecl get_tmpfile_buffer_nolock(buffer_id const id) throw()
{
    Character*& buffer_pointer = get_tmpfile_buffer_pointer_nolock(id, Character());
    if (!buffer_pointer)
    {
        buffer_pointer = _calloc_crt_t(Character, L_tmpnam).detach();
    }

    return buffer_pointer;
}

template <typename Character>
_Success_(return == true)
static bool __cdecl initialize_tmpfile_buffer_nolock(buffer_id const buffer_id) throw()
{
    typedef __acrt_stdio_char_traits<Character> stdio_traits;

    Character* const buffer       = get_tmpfile_buffer_nolock<Character>(buffer_id);
    size_t     const buffer_count = L_tmpnam;

    if (!buffer)
    {
        return false;
    }

    // The temporary path must be short enough so that we can append a file name
    // of the form [buffer id][process id].[unique id], which is at most 21
    // characters in length (plus we must leave room for the null terminator).
    //    1 Buffer Id              ("s", "t", or "u")
    //    7 Base-36 Process Id     (maximum: "1z141z3")
    //   13 Base-36 Unique File Id (maximum: "3w5e11264sgsf")
    DWORD const max_supported_temp_path_length = buffer_count - 22;
  
    // Generate the path prefix; make sure it ends with a slash or backslash:
    // CRT_REFACTOR TODO We need to use the WinRT temp path logic here.
    DWORD const temp_path_length = stdio_traits::get_temp_path(static_cast<DWORD>(buffer_count), buffer);
    if (temp_path_length == 0 || temp_path_length > max_supported_temp_path_length)
    {
        buffer[0] = '\0';
        return false;
    }

    Character* tail = buffer + temp_path_length;

    auto tail_count = [&](){ return buffer_count - (tail - buffer); };

    // Append the buffer identifier part of the file name:
    switch (buffer_id)
    {
    case buffer_id::tmpnam:   *tail++ = sizeof(Character) == 1 ? 's' : 'v'; break;
    case buffer_id::tmpfile:  *tail++ = sizeof(Character) == 1 ? 't' : 'w'; break;
    case buffer_id::tmpnam_s: *tail++ = sizeof(Character) == 1 ? 'u' : 'x'; break;
    }

    // Append the process identifier part of the file name:
    _ERRCHECK(stdio_traits::ulltot_s(GetCurrentProcessId(), tail, tail_count(), 36));
    tail += stdio_traits::tcslen(tail);

    // Append the dot part of the file name and the initial unique id:
    *tail++ = '.';
    *tail++ = '0';
    *tail++ = '\0';

    return true;
}



template <typename Character>
_Success_(return == true)
static bool __cdecl generate_tmpfile_file_name(
    _Inout_updates_z_(file_name_count) Character* const file_name,
    _In_ size_t                                   const file_name_count
    ) throw()
{
    typedef __acrt_stdio_char_traits<Character> stdio_traits;

    Character* const dot = reinterpret_cast<Character*>(stdio_traits::tcsrchr(file_name, '.'));
    _VALIDATE_RETURN_NOERRNO(dot != nullptr,                                         false);
    _VALIDATE_RETURN_NOERRNO(dot >= file_name,                                       false);
    _VALIDATE_RETURN_NOERRNO(file_name_count > static_cast<size_t>(dot - file_name), false);

    Character* const unique_id = dot + 1;
    size_t     const unique_id_count = file_name_count - (unique_id - file_name);

    uint64_t const next_identifier = stdio_traits::tcstoull(unique_id, nullptr, 36) + 1;
    if (next_identifier == 0)
        return false;

#pragma warning(disable:__WARNING_POSTCONDITION_NULLTERMINATION_VIOLATION) // 26036 Prefast doesn't understand perfect forwarding.
    _ERRCHECK(stdio_traits::ulltot_s(next_identifier, unique_id, unique_id_count, 36));
    return true;
}



static char** __cdecl get_tmpnam_ptd_buffer(char) throw()
{
    __acrt_ptd* const ptd = __acrt_getptd_noexit();
    if (ptd == nullptr)
        return nullptr;

    return &ptd->_tmpnam_narrow_buffer;
}

static wchar_t** __cdecl get_tmpnam_ptd_buffer(wchar_t) throw()
{
    __acrt_ptd* const ptd = __acrt_getptd_noexit();
    if (ptd == nullptr)
        return nullptr;

    return &ptd->_tmpnam_wide_buffer;
}



template <typename Character>
_Success_(return == 0)
static errno_t __cdecl common_tmpnam_nolock(
    _Out_writes_opt_z_(result_buffer_count) Character* const result_buffer,
    size_t                                             const result_buffer_count,
    buffer_id                                          const buffer_id
    ) throw()
{
    typedef __acrt_stdio_char_traits<Character> stdio_traits;

    Character* const global_buffer       = get_tmpfile_buffer_nolock<Character>(buffer_id);
    size_t     const global_buffer_count = L_tmpnam;

    if (!global_buffer)
    {
        return ENOMEM;
    }

    // Initialize the tmpnam buffer if it has not yet been initialized.
    // Otherwise, generate the next file name:
    if (global_buffer[0] == 0)
    {
        initialize_tmpfile_buffer_nolock<Character>(buffer_id);
    }
    else if (!generate_tmpfile_file_name(global_buffer, global_buffer_count))
    {
        return ENOENT;
    }

    // Generate a file name that does not already exist:
    while (stdio_traits::taccess_s(global_buffer, 0) == 0)
    {
        if (!generate_tmpfile_file_name(global_buffer, global_buffer_count))
        {
            return ENOENT;
        }
    }

    // If the result buffer is non-null, copy the file name there, if it will fit:
    if (result_buffer != nullptr)
    {
        if (buffer_id != buffer_id::tmpnam &&
            stdio_traits::tcslen(global_buffer) >= result_buffer_count)
        {
            if (result_buffer_count != 0)
            {
                result_buffer[0] = 0;
            }

            return ERANGE;
        }

#pragma warning(suppress:__WARNING_POSTCONDITION_NULLTERMINATION_VIOLATION) // 26036 Prefast doesn't understand perfect forwarding.
        _ERRCHECK(stdio_traits::tcscpy_s(result_buffer, result_buffer_count, global_buffer));

        return 0;
    }

    // If the result buffer is null, use a buffer owned by the per-thread data:
    _ASSERTE(buffer_id == buffer_id::tmpnam);

    Character** const ptd_buffer = get_tmpnam_ptd_buffer(Character());
    if (ptd_buffer == nullptr)
    {
        return ENOMEM;
    }

    if (*ptd_buffer == nullptr)
    {
        *ptd_buffer = _calloc_crt_t(Character, global_buffer_count).detach();
        if (*ptd_buffer == nullptr)
        {
            return ENOMEM;
        }
    }

    _ERRCHECK(stdio_traits::tcscpy_s(*ptd_buffer, global_buffer_count, global_buffer));
    return 0;
}



// Generates a temporary file name that is unique in the directory specified by
// the related macros in <stdio.h>.  Returns the file name in the result_buffer,
// or in a per-thread buffer if the result_buffer is null.  Returns zero on
// success; returns an error code and sets errno on failure.
template <typename Character>
_Success_(return == 0)
static errno_t common_tmpnam(
    _Out_writes_opt_z_(result_buffer_count) Character* const result_buffer,
    _In_ size_t                                        const result_buffer_count,
    _In_ buffer_id                                     const buffer_id,
    _Outptr_result_z_                      Character** const result_pointer
    ) throw()
{
    errno_t return_value = 0;

    __acrt_lock(__acrt_tempnam_lock);
    __try
    {
        errno_t const saved_errno = errno;

        return_value = common_tmpnam_nolock(result_buffer, result_buffer_count, buffer_id);
        if (return_value != 0)
        {
            *result_pointer = result_buffer;
            errno = return_value;
            __leave;
        }

        *result_pointer = result_buffer != nullptr
            ? result_buffer
            : *get_tmpnam_ptd_buffer(Character());

        errno = saved_errno;
    }
    __finally
    {
        __acrt_unlock(__acrt_tempnam_lock);
    }

    return return_value;
}



_Success_(return == 0)
static errno_t __cdecl common_tmpfile_nolock(_Out_ FILE** const stream, int const sh_flag) throw()
{
    char*  const global_buffer       = get_tmpfile_buffer_nolock<char>(buffer_id::tmpfile);
    size_t const global_buffer_count = L_tmpnam;

    if (!global_buffer)
    {
        return ENOMEM;
    }

    // Initialize the tmpfile buffer if it has not yet been initialized.
    // Otherwise, generate the next file name:
    if (*global_buffer == 0)
    {
        if (!initialize_tmpfile_buffer_nolock<char>(buffer_id::tmpfile))
        {
            return EINVAL; // REVIEW Which error?
        }
    }
    else if (!generate_tmpfile_file_name(global_buffer, global_buffer_count))
    {
        return EINVAL; // REVIEW Which error?
    }

    __crt_stdio_stream const local_stream = __acrt_stdio_allocate_stream();
    if (!local_stream.valid())
    {
        return EMFILE;
    }

    errno_t result = 0;

    __try
    {
        errno_t const saved_errno = errno;
        errno = 0;

        // Create a temporary file.  Note that the loop below will only create a
        // new file.  It will not open and truncate an existing behavior.  This
        // behavior is permitted under ISO C.  The behavior implemented below is
        // compatible with prior versions of the C Runtime and makes error
        // checking easier.
        int fh = 0;

        int const open_flag       = _O_CREAT | _O_EXCL | _O_RDWR | _O_BINARY | _O_TEMPORARY;
        int const permission_mode = _S_IREAD | _S_IWRITE;

        while ((result = _sopen_s(&fh, global_buffer, open_flag, sh_flag, permission_mode)) == EEXIST)
        {
            if (!generate_tmpfile_file_name(global_buffer, global_buffer_count))
            {
                break;
            }
        }

        if (errno == 0)
        {
            errno = saved_errno;
        }

        // Ensure that the loop above did indeed create the file:
        if (fh == -1)
        {
            __leave;
        }

        // Initialize the stream:
        local_stream->_tmpfname = _strdup_crt(global_buffer);
        if (local_stream->_tmpfname == nullptr)
        {
            _close(fh);
            result = ENOMEM;
            __leave;
        }

        local_stream->_cnt  = 0;
        local_stream->_base = nullptr;
        local_stream->_ptr  = nullptr;
        local_stream.set_flags(_commode | _IOUPDATE);
        local_stream->_file = fh;
        *stream = local_stream.public_stream();
        result = 0;
    }
    __finally
    {
        // If we didn't complete initialization of the stream successfully, we
        // must free the allocated stream:
        if (local_stream->_file == -1)
            __acrt_stdio_free_stream(local_stream);

        local_stream.unlock();
    }

    return result;
}



_Success_(return == 0)
static errno_t __cdecl common_tmpfile(_Out_ FILE** const stream, int const sh_flag) throw()
{
    _VALIDATE_RETURN_ERRCODE(stream != nullptr, EINVAL);
    *stream = nullptr;

    errno_t return_value = 0;

    __acrt_lock(__acrt_tempnam_lock);
    __try
    {
        return_value = common_tmpfile_nolock(stream, sh_flag);
        if (return_value != 0)
        {
            errno = return_value;
        }
    }
    __finally
    {
        __acrt_unlock(__acrt_tempnam_lock);
    }

    return return_value;
}



extern "C" errno_t __cdecl tmpnam_s(
    char*  const result_buffer,
    size_t const result_buffer_count
    )
{
    char* result = nullptr;
    _VALIDATE_RETURN_ERRCODE(result_buffer != nullptr, EINVAL);
    return common_tmpnam(result_buffer, result_buffer_count, buffer_id::tmpnam_s, &result);
}

extern "C" errno_t __cdecl _wtmpnam_s(
    wchar_t* const result_buffer,
    size_t   const result_buffer_count
    )
{
    wchar_t* result = nullptr;
    _VALIDATE_RETURN_ERRCODE(result_buffer != nullptr, EINVAL);
    return common_tmpnam(result_buffer, result_buffer_count, buffer_id::tmpnam_s, &result);
}

extern "C" char* __cdecl tmpnam(char* const result_buffer)
{
    char* result = nullptr;
    common_tmpnam(result_buffer, L_tmpnam, buffer_id::tmpnam, &result);
    return result;
}

extern "C" wchar_t* __cdecl _wtmpnam(wchar_t* const result_buffer)
{
    wchar_t* result = nullptr;
    common_tmpnam(result_buffer, L_tmpnam, buffer_id::tmpnam, &result);
    return result;
}

// Creates a temporary file with the file mode "w+b".  The file will be deleted
// automatically when it is closed or when the program terminates normally.
// On success, returns a stream; on failure, returns nullptr.
extern "C" FILE* __cdecl tmpfile()
{
    FILE* stream = nullptr;
    common_tmpfile(&stream, _SH_DENYNO);
    return stream;
}

// Creates a temporary file with the file mode "w+b".  The file will be deleted
// automatically when it is closed or when the program terminates normally.
// On success, returns zero and '*stream' refers to the newly opened stream.
// On failure, returns an error code and '*stream' is null.
extern "C" errno_t __cdecl tmpfile_s(FILE** const stream)
{
    return common_tmpfile(stream, _SH_DENYRW);
}



// Ensure that _rmtmp is called during static CRT termination:
#ifndef CRTDLL

    extern "C" extern unsigned __acrt_tmpfile_used;

    extern "C" void __acrt_force_use_of_tmpfile()
    {
        ++__acrt_tmpfile_used;
    }

#endif
