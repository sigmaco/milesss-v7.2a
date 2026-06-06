//############################################################################
//##                                                                        ##
//##  BANKFILE.INL                                                          ##
//##                                                                        ##
//##  RAD bankfile utility module, to be #included within a .c or .cpp file ##
//##                                                                        ##
//##  jmiles@pop.net                                                        ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#ifdef __cplusplus
   extern "C" {
   #define DEFPARM(x) = x       // Default parameters are supported when compiled as .cpp
#else
   #define DEFPARM(x)           // Default parameters are ignored when compiled as .c
#endif

#ifndef IS_PS2
  #pragma pack(1)               // Make sure struct packing disabled
#endif

#ifdef IS_WINDOWS
  #pragma warning(disable:4505) // Allow unused local functions
#endif

static void ____________________________________________________Externs_________________________________________(){}

//
// #define BANK_WRITE_SUPPORT to enable file modification (write routines must be 
// supplied)
//
// The app must also #define BANKSIZE to either S32 or S64, to define whether the
// bankfile routines should be built for 32-bit compatibility
//
// BANKFILE_FUNCTION_DECORATE is a macro used to decorate the public bankfile functions.
// It is passed the return type and name of the function and it can insert whatever
// modifiers you need before or after the return type (__cdecl, __dllexport,
// static, etc). By default, this macro is defined to "static fnrettype fnname".
// 

#ifndef RADSTRUCT
  #ifdef __GCC__
    #define RADSTRUCT struct __attribute__((__packed__))
  #else
    #define RADSTRUCT struct
  #endif
#endif

#ifndef BANKFILE_FUNCTION_DECORATE
  #define BANKFILE_FUNCTION_DECORATE( fnrettype, fnname ) static fnrettype fnname
#endif

#ifndef BANKSIZE
  #error Must define BANKSIZE as S32 or S64
#endif

//
// The app can override the following macros in the module(s) that include
// BANKFILE.INL:
//

#ifndef BANK_malloc
  #define BANK_malloc(bytes)                         malloc(bytes)
#endif

#ifndef BANK_free
  #define BANK_free(ptr)                             free(ptr)
#endif

#ifndef BANK_memcpy
  #define BANK_memcpy(dest,src,len)                  memcpy(dest,src,len)
#endif

#ifndef BANK_stricmp
  #define BANK_stricmp(t1,t2)                        _stricmp(t1,t2)
#endif

#ifndef BANK_strnicmp
  #define BANK_strnicmp(t1,t2,n)                     _strnicmp(t1,t2,n)
#endif

#ifndef BANK_open_read_only
  #define BANK_open_read_only(filename)              _open(filename, O_BINARY | O_RDONLY)
#endif

#ifndef BANK_seek
  #define BANK_seek(file_descriptor, offset)         _lseek((int) file_descriptor, (long) offset, SEEK_SET)
#endif

#ifndef BANK_read
  #define BANK_read(file_descriptor, dest, n_bytes)  _read((int) file_descriptor, dest, (long) n_bytes)
#endif

#ifndef BANK_close
  #define BANK_close(file_descriptor)                _close((int) file_descriptor)
#endif

#ifndef BANK_assert
   #define BANK_assert(expr) assert(expr)
#endif

#ifdef BANK_WRITE_SUPPORT   

  #ifndef BANK_open_create
    #define BANK_open_create(filename)                 _open(filename, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, _S_IREAD | _S_IWRITE)
  #endif
  
  #ifndef BANK_open_read_write
    #define BANK_open_read_write(filename)             _open(filename, O_BINARY | O_RDWR)
  #endif
  
  #ifndef BANK_write
    #define BANK_write(file_descriptor, dest, n_bytes) _write((int) file_descriptor, dest, (long) n_bytes); 
  #endif
  
  #ifndef BANK_system_time

    static S64 BANK_system_time (void)
      {
      //
      // Returns # of 1000-ns intervals since 1-Jan-1601 UTC
      //
      // This time follows DST and other changes to the system clock.
      // It is consequently *not* monotonic in nature, and should not
      // be used to implement delay loops or similar constructs
      //           

      static union
         {
         FILETIME ftime;
         S64      itime;
         }
      T;

      T.itime = 0;

      GetSystemTimeAsFileTime(&T.ftime);

      return T.itime / 10;
      }

  #endif
  
#endif

#ifndef BANKFN
  #define BANKFN(ret_type,function_name) ret_type function_name
#endif

#ifndef DEFAULT_BF_DIRBLK_BYTES
  #define DEFAULT_BF_DIRBLK_BYTES 512   // Use very small values for testing, normally much bigger (64K-1MB)
#endif

#ifndef DEFAULT_BF_INITDIR_BYTES      
  #define DEFAULT_BF_INITDIR_BYTES 0    // Don't create an initial directory block by default  
#endif

#ifndef SUPPORTED_BF_VERSION
  #define SUPPORTED_BF_VERSION 0x100
#endif

#ifndef BF_FILE_TYPE_ID
  #define BF_FILE_TYPE_ID "RADBFile"    // Must be 8 chars!
#endif

static void ____________________________________________________Public_declarations________________________________________(){}

typedef struct _BANKFILE * HBANKFILE;

typedef enum
{
   BANK_OK = 0,              // No error
   BANK_OUT_OF_MEMORY,       // BANK_malloc() failure
   BANK_INVALID_OBJECT,      // Bad/NULL handle
   BANK_INVALID_REQUEST,     // Attempt to do something unsupported (e.g., read from a negative offset)
   BANK_INVALID_TYPE,        // File type not identifiable
   BANK_INVALID_FILE_HANDLE, // File handle was not valid (e.g., closed by app)
   BANK_SIZE_TYPE_TOO_SMALL, // Tried to open a 32-bit bankfile with BANKSIZE=S64
   BANK_SIZE_TYPE_TOO_LARGE, // Tried to open a 64-bit bankfile with BANKSIZE=S32
   BANK_NOT_SUPPORTED,       // File version is not supported
   BANK_CORRUPT,             // File is corrupt (failed CRC or other integrity check)
   BANK_NOT_WRITABLE,        // Attempted to write to file that wasn't open for writing
   BANK_IO_ERROR,            // General filesystem access problem (bad seek, etc.)
   BANK_READ_ERROR,          // Failure to read expected amount of data (e.g., file truncated)
   BANK_WRITE_ERROR,         // Failure to write data (e.g., disk full)
   BANK_FILE_NOT_FOUND,      // An attempt to open a readable file failed
   BANK_OPEN_FAILURE,        // An attempt to open a writable file failed
   BANK_ENTRY_NOT_FOUND,     // Requested named entry was not found in bankfile, or end of enumeration
   BANK_ENTRY_DELETED,       // The only entr(ies) by this name were deleted
   BANK_GENERAL_FAILURE      // Catchall error
}
BANKRESULT;

#ifdef IS_WINDOWS
  #pragma pack(1)
#endif

#ifdef __WATCOMC__
   #ifdef S64
      #undef S64
      #undef U64
   #endif

   typedef struct _S64
      {
      S32 l;
      S32 h;
      } 
   S64;

   typedef struct _U64
      {
      U32 l;
      U32 h;
      } 
   U64;

   #define TO_BANKSIZE(x) ((BANKSIZE) (x).l)    

#else

   #define TO_BANKSIZE(x) ((BANKSIZE) (x))

#endif

typedef RADSTRUCT _BANKFILEINFO   
{
   char    *filename;                // Original filename passed to BankOpen() or BankWriteFile()
   SINTa    file_handle;             // Platform-specific handle to physical file, or -1 if closed
   BANKSIZE embedded_at_offset;      // Offset parameter passed to BankOpenReadOnly()
   char     sig[8];                  // e.g., 'RADBFILE'
   S32      bankfile_version;        // e.g., 0x100 = 1.00
   S64      created_time;            // Bankfile creation timestamp
   S64      modified_time;           // Time at last flush operation
   S32      n_entries;               // Total # of entries, including deleted and abandoned ones
   S32      n_dirblks;               // Total # of directory blocks in file
   BANKSIZE file_size;               // Total size of bank file
   BANKSIZE data_bytes;              // Total amount of space assigned to entry data
   BANKSIZE data_bytes_used;         // # of bytes actually used to store data
   BANKSIZE directory_bytes;         // Total # of bytes in directory blocks
   BANKSIZE directory_bytes_used;    // # of bytes actually used to store directory contents and headers
   BANKSIZE minimum_directory_size;  // # of bytes that would be needed to store all active directory entries in a single block 
   BANKSIZE default_dirblk_bytes;    // Size of newly-allocated directory blocks
   BANKSIZE initial_dirblk_bytes;    // Size of directory block immediately following file header, or 0 if none
}
BANKFILEINFO;

typedef RADSTRUCT _BANKENTRYINFO 
{
   char    *name;               // Name of resource assigned when written
   char    *original_filename;  // Original source filename (optional; always valid but may be empty) 
   char    *user_string;        // Application-specific user string (optional; always valid but may be empty)
   U64      user;               // Application-specific user value
          
   S32      is_deleted;         // TRUE if entry has been deleted and is eligible for recovery

   BANKSIZE allocated_bytes;    // # of bytes of space allocated to data
   BANKSIZE used_bytes;         // # of bytes actually used, not including front padding (0 is valid!)
   BANKSIZE offset_in_file;     // Offset from start of file (including container offset, if any)
   S32      align_bytes;        // If > 1, data is stored at this alignment relative to beginning of its data space
   S64      created_time;       // Blind data field dedicated to timestamp storage in app's format of choice
   S64      modified_time;      // ""
}
BANKENTRYINFO;

typedef RADSTRUCT _BANKENTRYATTRIBS
{
   char *original_filename;
   char *user_string;
   U64   user_val;
   S32   storage_alignment;
   S32   extra_bytes;
}
BANKENTRYATTRIBS;

// --------------------------------------------------------
// Bankfile public API
// --------------------------------------------------------

typedef UINTa HBANKENUM;        // Cookie used by BankEnumEntries()
#define HBANKENUM_FIRST 0

// TODO: Document

#ifndef BANK_RESULT_TEXT
  #define BANK_RESULT_TEXT BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_FLUSH
  #define BANK_FLUSH BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_CLOSE_FILE_HANDLE
  #define BANK_CLOSE_FILE_HANDLE BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_CLOSE
  #define BANK_CLOSE BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_CREATE
  #define BANK_CREATE BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_OPEN_READ_ONLY
  #define BANK_OPEN_READ_ONLY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_OPEN_READ_WRITE
  #define BANK_OPEN_READ_WRITE BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_WRITE_ENTRY
  #define BANK_WRITE_ENTRY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_READ_ENTRY
  #define BANK_READ_ENTRY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_READ_VERIFY_ENTRY
  #define BANK_READ_VERIFY_ENTRY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_FREE_READ_ENTRY
  #define BANK_FREE_READ_ENTRY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_DELETE_ENTRY
  #define BANK_DELETE_ENTRY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_UNDELETE_ENTRY
  #define BANK_UNDELETE_ENTRY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_RENAME_ENTRY
  #define BANK_RENAME_ENTRY BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_INFO
  #define BANK_INFO BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_ENTRY_INFO
  #define BANK_ENTRY_INFO BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_ENUM_ENTRIES
  #define BANK_ENUM_ENTRIES BANKFILE_FUNCTION_DECORATE
#endif

#ifndef BANK_COPY
  #define BANK_COPY BANKFILE_FUNCTION_DECORATE
#endif

BANK_RESULT_TEXT(char *, BankResultText) (BANKRESULT result);

BANK_FLUSH(BANKRESULT,BankFlush)(HBANKFILE BANK);

BANK_CLOSE_FILE_HANDLE(BANKRESULT,BankCloseFileHandle)(HBANKFILE BANK);

BANK_CLOSE(BANKRESULT,BankClose)(HBANKFILE BANK);

BANK_CREATE(BANKRESULT,BankCreate)
                     (HBANKFILE *handle, 
                      char      *filename,
                      BANKSIZE   initial_dirblk_bytes DEFPARM(DEFAULT_BF_INITDIR_BYTES),
                      BANKSIZE   default_dirblk_bytes DEFPARM(DEFAULT_BF_DIRBLK_BYTES));

BANK_OPEN_READ_WRITE(BANKRESULT,BankOpenReadWrite)
                     (HBANKFILE  *handle,
                      const char *filename);

BANK_OPEN_READ_ONLY(BANKRESULT,BankOpenReadOnly)
                     (HBANKFILE  *handle,
                      const char *filename,
                      BANKSIZE    embedded_at_offset DEFPARM(0));

BANK_WRITE_ENTRY(BANKRESULT,BankWriteEntry)
                         (HBANKFILE         BANK,
                          void             *src,
                          BANKSIZE          n_bytes,
                          char             *resource_name, 
                          BANKENTRYATTRIBS *attribs DEFPARM(NULL));

BANK_READ_ENTRY(BANKRESULT,BankReadEntry)
                        (HBANKFILE BANK,
                         char     *resource_name, 
                         void    **dest              DEFPARM(NULL),
                         BANKSIZE *n_bytes_read      DEFPARM(NULL),
                         BANKSIZE  n_bytes_requested DEFPARM(0),
                         BANKSIZE  read_at_offset    DEFPARM(0));

BANK_READ_VERIFY_ENTRY(BANKRESULT,BankReadVerifyEntry)
                              (HBANKFILE BANK,
                               char     *resource_name,
                               void    **dest         DEFPARM(NULL),
                               BANKSIZE *n_bytes_read DEFPARM(NULL));

BANK_FREE_READ_ENTRY(BANKRESULT,BankFreeReadEntry)
                            (HBANKFILE BANK,
                             void    **dest);

BANK_RENAME_ENTRY(BANKRESULT,BankRenameEntry)
                          (HBANKFILE BANK,
                           char     *resource_name,
                           char     *new_name);

BANK_DELETE_ENTRY(BANKRESULT,BankDeleteEntry)
                          (HBANKFILE BANK,
                           char     *resource_name);
                           
BANK_UNDELETE_ENTRY(BANKRESULT,BankUndeleteEntry)
                            (HBANKFILE BANK,
                             char     *resource_name);

BANK_INFO(BANKRESULT,BankInfo)
                   (HBANKFILE     BANK,
                    BANKFILEINFO *dest);

BANK_ENTRY_INFO(BANKRESULT,BankEntryInfo)
                        (HBANKFILE       BANK,
                         const char     *name,
                         BANKENTRYINFO  *dest);

BANK_ENUM_ENTRIES(BANKRESULT,BankEnumEntries)
                          (HBANKFILE       BANK,
                           HBANKENUM      *cookie,
                           BANKENTRYINFO  *dest);

BANK_COPY(BANKRESULT,BankCopy)
                   (HBANKFILE SRC,
                    char     *dest_filename,
                    BANKSIZE  initial_dirblk_bytes DEFPARM(0),
                    BANKSIZE  default_dirblk_bytes DEFPARM(0));

static void ____________________________________________________Private_declarations_______________________________________(){}

// TODO: endian conversion whenever loading a structure from disk!

#define BFE_SPACE_AVAILABLE 0x00000001    // Filespace pointed to by this entry is available for reclamation
#define BFE_UNUSED_ENTRY    0x00000002    // Directory entry is not in use and is available for reclamation.  Allocated data space is always 0 bytes
#define BFE_DELETED         0x00000004    // Entry has been explicitly deleted (not just moved to a different block), and can be recovered

typedef RADSTRUCT _BFHDR          // File header always uses 64-bit ints to store sizes regardless of BANKSIZE
{
   char    sig[8];                // 'RADBFILE'
   S32     bankfile_version;      // e.g., 0x100 = 1.00
   U32     BANKSIZE_bytes;        // 4 if BANKSIZE is S32, 8 if S64...
   U32     header_CRC;            // CRC for header only, updated at flush time
   S64     default_dirblk_bytes;  // Newly-allocated directory blocks will be this size
   S64     initial_dirblk_bytes;  // Size of directory block immediately following file header, or 0 if none 
   S64     created_time;          // Bankfile creation timestamp
   S64     modified_time;         // Time at last flush operation
   S64     file_size;             // Total size of bank file including BFHDR (updated on the fly)
   S64     data_bytes;            // # of bytes in allocated filespace (updated at flush time)
   S64     data_bytes_used;       // # of data bytes excluding abandoned filespace (updated at flush time; does not include space in directory blocks)
   S64     directory_bytes;       // # of bytes in all directory blocks, updated at flush time
   S64     directory_bytes_used;  // # of bytes actually used by valid directory entries, updated at flush time
   S32     n_entries;             // Total # of entries in bank file (updated at flush time)
   S32     n_dirblks;             // Total # of directory blocks in file (updated at flush time)
   S64     first_dirblk;          // Offset to first BFDIRBLK, if not -1
}                             
BFHDR;

typedef RADSTRUCT _BFDIRENT    // On-disk representation of bankfile entry
{
   S32       dir_entry_bytes;  // # of bytes needed to hold this BFDIRENT entry plus its ASCIIZ name strings
   U32       BFE_flags;        // Deleted, etc.
   BANKSIZE  data_offset;      // Offset to data, relative to beginning of file header
   S32       data_alignment;   // If > 1, actual data offset is rounded up to this alignment value
   BANKSIZE  allocated_bytes;  // # of bytes allocated to data storage
   BANKSIZE  used_bytes;       // # of bytes actually containing user source data (0 is valid!)
   U64       user;             // Application-specific user value
   U32       data_CRC;         // CRC for data block
   S64       created_time;     // Timestamps in app-specified format
   S64       modified_time;   
// char name[];               // ASCIIZ string: name of entry as stored in bankfile
// char original_filename[];  // ASCIIZ string: original filename (optional)
// char user_text[];          // ASCIIZ string: user text (optional)
}
BFDIRENT;

typedef RADSTRUCT _BFDIRBLK  // BFDIRBLK contains as many BFDIRENTs as will fit in dirblk_bytes
{                            // All BFDIRBLKs are rewritten when a modified file is committed
   BANKSIZE  next_dirblk;    // -1 when no more blocks remain (valid only on disk)
   BANKSIZE  dirblk_bytes;   // Disk/RAM space allocated for BFDIRBLK and any entries
   U32       dirblk_CRC;     // CRC for BFDIRBLK and all BFDIRENTs
   S32       n_entries;      // # of entries in use
   BANKSIZE  used_bytes;     // Actual size of BFDIRBLK and all its entries, must be <= dirblk_bytes
// BFDIRENT  first_entry;    // List of variable-size BFDIRENT entries follow the BFDIRBLK header
}
BFDIRBLK;

typedef RADSTRUCT _BFMENT      // In-memory representation of bankfile directory entry
{
   BFDIRENT       *entry;      // Directory entry
   struct _BFMBLK *block;      // Block it came from

   char     *name;             // Pointers to strings following BFDIRENT
   char     *original_filename;    
   char     *user_string;      
}
BFMENT;

typedef RADSTRUCT _BFMBLK    // In-memory representation of directory block
{
   BFDIRBLK  *block_image;   // Header plus list of contiguous BFDIRENTs (non-indexable, as many as will fit) 
   BANKSIZE   block_offset;  // Offset in file where block resides (-1 if not yet written)
   BFMENT    *entries;       // Indexable array of pointers to BFDIRENT entries and text fields
   S32        dirty;         // TRUE if block needs to be written when file is flushed

   _BFMBLK   *next_block;
}
BFMBLK;

RADSTRUCT _BANKFILE
{
   char    *filename;
   SINTa    file_handle;         // OS handle
   S32      file_handle_valid;   // TRUE unless the app told us to close the file
   S32      open_for_writing;
   S32      need_flush;
   BANKSIZE container_offset;    // Offset where bankfile resides in a larger container file

   BFHDR   header;               // Copy of header from file open/create call

   BFMBLK *first_block;          // First directory block in memory (which can be NULL)
};

#ifndef IS_PS2
  #pragma pack()
#endif

//
// [CMU] This is a standard 32-bit CRC table.  Originally I thought it was
// from GZip, but when asked to clarify its origin for licensing issues,
// I found that it was in fact a generic CRC table that is used everywhere
// from the RFC for PPP to the March 1995 CRC article in MSJ.  So it would
// appear that there is no "license" for this table, as far as I can tell,
// and therefore I am simply labeling it by its generator polynomial:
//
//     x^0 + x^1 + x^2 + x^4 + x^5 + x^7 + x^8 + x^10 + x^11 + x^12 +
//     x^16 + x^22 + x^23 + x^26 + x^32
//

static U32 CRC32Table[] =
{
   0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419,
   0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4,
   0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07,
   0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
   0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856,
   0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
   0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4,
   0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
   0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
   0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A,
   0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599,
   0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
   0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190,
   0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F,
   0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E,
   0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
   0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED,
   0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
   0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3,
   0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
   0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A,
   0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5,
   0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010,
   0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
   0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17,
   0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6,
   0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615,
   0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
   0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344,
   0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
   0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A,
   0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
   0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1,
   0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C,
   0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF,
   0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
   0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE,
   0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31,
   0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C,
   0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
   0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B,
   0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
   0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1,
   0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
   0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
   0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7,
   0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66,
   0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
   0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605,
   0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8,
   0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B,
   0x2D02EF8D
};

static void BeginCRC32(U32 *CRC)
{
   *CRC = 0xFFFFFFFF;
}

static void AddToCRC32(U32        *CRC, 
                       BANKSIZE     n_bytes, 
                       void const *src)
{
   U8 const *S = (U8 const *) src;

   while (n_bytes--)
      {
      *CRC = CRC32Table[((S32) (*CRC) ^ (*S++)) & 0xFF] ^ ((*CRC) >> 8);
      }
}

static void EndCRC32(U32 *CRC)
{
    *CRC ^= 0xFFFFFFFF;
}

//***************************************************************************
//
// BANK_padding()
//
//***************************************************************************

static inline S32 BANK_padding(S32 data_alignment, BANKSIZE data_offset)
{
   return (data_alignment-1) - (S32) ((data_offset + (data_alignment-1)) % data_alignment); 
}

//***************************************************************************
//
// BANK_set_entry_info()
//
//***************************************************************************

static void BANK_set_entry_info(HBANKFILE       BANK,
                                BFMENT         *E, 
                                BANKENTRYINFO  *dest)
{
   //
   // Fill out application-visible BANKENTRYINFO structure based on internal
   // entry properties
   //

   BFDIRENT *D = E->entry;

   S32 front_padding = BANK_padding(D->data_alignment, D->data_offset);  

   dest->name              =  E->name;            
   dest->original_filename =  E->original_filename;   
   dest->user_string       =  E->user_string;
   dest->user              =  D->user;            
   dest->is_deleted        = (D->BFE_flags & BFE_DELETED) != 0;
   dest->allocated_bytes   =  D->allocated_bytes; 
   dest->align_bytes       =  D->data_alignment;
   dest->offset_in_file    =  BANK->container_offset + D->data_offset + front_padding;
   dest->used_bytes        =  D->used_bytes;
   dest->created_time      =  D->created_time;    
   dest->modified_time     =  D->modified_time;    
}

//***************************************************************************
//
// BANK_returnable_entry()
//
//***************************************************************************

static inline S32 BANK_returnable_entry(BFMENT *E)
{
   // 
   // Skip placeholder entries unless they correspond to
   // deleted resources (which are returnable from search/enum functions)
   //
   // Unused entries are never returnable
   //

   if (E->entry->BFE_flags & BFE_UNUSED_ENTRY)
      {
      return 0;
      }

   if (E->entry->BFE_flags & BFE_SPACE_AVAILABLE)
      {
      if (!(E->entry->BFE_flags & BFE_DELETED))
         {
         return 0;
         }
      }

   return 1;
}

//***************************************************************************
//
// BANK_find_entry()
//
//***************************************************************************

static BANKRESULT BANK_find_entry(HBANKFILE   BANK,
                                  BFMENT    **info,
                                  const char *name)
{
   if ((BANK == NULL) || (name == NULL))
      {
      return BANK_INVALID_OBJECT;
      }
   
   if (info != NULL)
      {
      *info = NULL;
      }

   //
   // Find entry in file
   // (TODO: replace with hash LUT that indexes all in-use entries)
   //

   BFMENT *E = NULL;

   BFMBLK *MB = BANK->first_block;

   while (MB != NULL)
      {
      for (S32 i=0; i < MB->block_image->n_entries; i++)
         {
         E = &MB->entries[i];

         if (!BANK_returnable_entry(E))
            {
            continue;
            }

         if (!BANK_stricmp(E->name, name))
            {
            if (info != NULL)
               {
               *info = E;
               }

            if (E->entry->BFE_flags & BFE_DELETED)
               return BANK_ENTRY_DELETED;
            else
               return BANK_OK;
            }
         }
      
      MB = MB->next_block;
      }

   return BANK_ENTRY_NOT_FOUND;
}

//***************************************************************************
//
// BANK_write_entry()
//
// Used by both BankWriteEntry() and BankRenameEntry()
//
//***************************************************************************

#ifdef BANK_WRITE_SUPPORT

static BANKRESULT BANK_write_entry(HBANKFILE         BANK,
                                   void             *src,
                                   BANKSIZE          n_bytes,
                                   char             *resource_name, 
                                   char             *new_name,
                                   BANKENTRYATTRIBS *attribs)
{
   if ((BANK == NULL) || (resource_name == NULL))
      {
      return BANK_INVALID_OBJECT;
      }

   if (!BANK->file_handle_valid)
      {
      return BANK_INVALID_FILE_HANDLE;
      }

   if ((BANK->container_offset != 0) || (!BANK->open_for_writing))
      {
      return BANK_NOT_SUPPORTED;            // We don't support writing to embedded files 
      }

   S64 cur_time = BANK_system_time();

   char *original_filename = NULL;
   char *user_string       = NULL;
   U64   user_val          = 0;
   S32   storage_alignment = 0;
   S32   extra_bytes       = 0;

   if (attribs != NULL)
      {
      original_filename = attribs->original_filename;
      user_string       = attribs->user_string;
      user_val          = attribs->user_val;
      storage_alignment = attribs->storage_alignment;
      extra_bytes       = attribs->extra_bytes;
      }

   //
   // First, do fast lookup to see if the directory entry already exists
   // If it does, and the new entry needs <= the space reserved for the existing entry 
   // (including all variable-length text fields in the directory entry itself), we 
   // can reuse it
   //

   BFMENT *EM = NULL;
   BANKRESULT result = BANK_find_entry(BANK, &EM, resource_name);

   //
   // We must find or create a new directory entry in *any* of the following cases:
   //   - The entry does not already exist
   //   - The existing entry is not big enough to contain the newly-specified metadata
   //   - The existing entry's allocated data space is too small and must be abandoned
   //
   // (Even if the old directory entry is large enough to contain the new metadata in
   // the third case, it has to be kept around as a placeholder until the abandoned 
   // filespace it refers to is eventually reclaimed)
   // 

   BFDIRENT *existing = NULL;

   if (EM == NULL)
      {
      if (src == NULL)
         {
         return result;                // No existing entry to preserve!
         }

      if (new_name != NULL)
         {
         return BANK_ENTRY_NOT_FOUND;  // No existing entry to rename!
         }
      }
   else
      {
      existing = EM->entry;

      if (new_name != NULL)
         {
         resource_name = new_name;
         }

      //
      // If no attributes provided, use existing entry's attributes
      //

      if (attribs == NULL)
         {
         original_filename = EM->original_filename;
         user_string       = EM->user_string;
         user_val          = existing->user;
         storage_alignment = existing->data_alignment;    
         }

      EM->block->dirty = 1;

      if (src == NULL)
         {
         n_bytes = existing->used_bytes;
         }
      }

   if (storage_alignment < 1)
      {
      storage_alignment = 1;
      }

   BANKSIZE need_data_bytes = n_bytes;

   if (existing != NULL)
      {
      need_data_bytes += BANK_padding(storage_alignment, existing->data_offset);
      }

   //
   // Get size in bytes of directory entry needed to represent the
   // resource being written
   //

   S32 orgname_bytes = (original_filename == NULL) ? 1 : strlen(original_filename) + 1;
   S32 userstr_bytes = (user_string   == NULL)     ? 1 : strlen(user_string)   + 1;
   S32 resname_bytes =                                   strlen(resource_name) + 1;

   S32 need_entry_bytes = sizeof(BFDIRENT)  +
                          orgname_bytes     +
                          userstr_bytes     +
                          resname_bytes;

   S32 enough_data_space = (EM != NULL) && (need_data_bytes  <= existing->allocated_bytes);
   S32 enough_meta_space = (EM != NULL) && (need_entry_bytes <= existing->dir_entry_bytes);

   BFMENT   *NE = NULL;      // directory entry we'll be writing to
   BFDIRENT *ND = NULL;      // shortcut to its BFDIRENT

   if (enough_data_space && enough_meta_space)
      {
      //
      // Reuse existing directory entry and the space allocated to it
      // Don't write any extra padding, since we're not allocating new space 
      //

      ND = existing;
      NE = EM;

      extra_bytes = 0;     
      }
   else
      {
      //
      // Try to find an unused directory entry that's big enough to hold
      // all the metadata
      //
      // At the same time, look for an entry referring to an unused block
      // that's big enough to hold the new entry's data.  We'll need this if 
      // enough_data_space is FALSE
      //
      // Finally, we also want to log the first directory block we can find
      // that's big enough to accommodate a *new* entry, in case one needs to
      // be allocated
      //

      BFMENT *RE = NULL;               // entry with reclaimable data space
      BFMBLK *EB = NULL;               // existing directory block that can contain the new entry if necessary
      BFMBLK *MB = BANK->first_block;   // the last block in the chain (after the loop exits)

      while (MB != NULL)
         {
         for (S32 i=0; i < MB->block_image->n_entries; i++)
            {
            BFMENT *T = &MB->entries[i];

            if ((T->entry->BFE_flags & BFE_UNUSED_ENTRY) &&
                (T->entry->dir_entry_bytes >= need_entry_bytes))
               {
               if ((NE == NULL) ||     // (Use the smallest available entry that's big enough)
                   (T->entry->dir_entry_bytes < NE->entry->dir_entry_bytes))
                  {
                  NE = T;
                  ND = NE->entry;
                  }
               }

            if (T->entry->BFE_flags & BFE_SPACE_AVAILABLE)
               {
               BANKSIZE need_data_bytes = BANK_padding(storage_alignment, T->entry->data_offset) + n_bytes;

               if (T->entry->allocated_bytes >= need_data_bytes)
                  {
                  if ((RE == NULL) ||     // (Use the smallest available block that's big enough)
                      (T->entry->allocated_bytes < RE->entry->allocated_bytes))
                     {
                     RE = T;
                     }
                  }
               }
            }
         
         if ((EB == NULL) &&           // (Stop looking after first qualified block or entry found)
             (NE == NULL) &&
             (MB->block_image->used_bytes + need_entry_bytes <= MB->block_image->dirblk_bytes))
            {
            EB = MB;
            }

         if (MB->next_block == NULL)
            {
            break;   // (Exit here so newly-allocated block can be chained to the last one)
            }

         MB = MB->next_block;
         }

      if (NE == NULL)
         {
         //
         // Couldn't find an available entry -- we must allocate a new one
         //

         if (EB != NULL)
            {
            //
            // Reallocate existing block's index array to accommodate our additional entry
            //
            // If we're reclaiming space from an entry that lies within this block, 
            // fix up the entry pointer
            //

            S32 RE_index = -1; 

            if ((RE != NULL) && (RE->block == EB))
               {
               RE_index = RE - RE->block->entries;
               }

            S32 array_bytes = (EB->block_image->n_entries+1) * sizeof(BFMENT);

            BFMENT *new_entries = (BFMENT *) BANK_malloc(array_bytes);

            if (new_entries == NULL)
               {
               return BANK_OUT_OF_MEMORY;
               }
            
            memset(new_entries, 0, array_bytes);

            if (EB->entries != NULL)
               {
               BANK_memcpy(new_entries,
                           EB->entries,
                           array_bytes - sizeof(BFMENT)); // (all previously-existing entries)

               BANK_free(EB->entries);
               }

            EB->entries = new_entries;

            if (RE_index != -1)
               {
               RE = &new_entries[RE_index];
               }

            ND = (BFDIRENT *) (((char *) EB->block_image) + EB->block_image->used_bytes);
            NE = &EB->entries[EB->block_image->n_entries];

            ND->dir_entry_bytes = 0;   // (Make sure we don't interpret this as a shrunken entry)

            EB->block_image->n_entries++;
            EB->block_image->used_bytes += need_entry_bytes;
            }
         else
            {
            //
            // Allocate a new BFMBLK and single-entry BFDIRBLK after the last block 
            // in the list
            //

            EB = (BFMBLK *) BANK_malloc(sizeof(BFMBLK));

            if (EB == NULL)
               {
               return BANK_OUT_OF_MEMORY;
               }

            memset(EB, 0, sizeof(BFMBLK));

            EB->block_offset = -1;

            BANKSIZE dirblk_bytes   = (size_t) BANK->header.default_dirblk_bytes;
            BANKSIZE need_blk_bytes = need_entry_bytes + sizeof(BFDIRBLK);

            if (dirblk_bytes < need_blk_bytes)
               {
               dirblk_bytes = need_blk_bytes;
               }

            EB->block_image = (BFDIRBLK *) BANK_malloc(dirblk_bytes);

            if (EB->block_image == NULL)
               {
               BANK_free(EB);
               return BANK_OUT_OF_MEMORY;
               }
               
            memset(EB->block_image, 0, need_blk_bytes);

            EB->entries = (BFMENT *) BANK_malloc(sizeof(BFMENT));

            if (EB->entries == NULL)
               {
               BANK_free(EB->block_image);
               BANK_free(EB);
               return BANK_OUT_OF_MEMORY;
               }

            memset(EB->entries, 0, sizeof(BFMENT));

            EB->next_block = NULL;

            if (MB == NULL)
               {
               BANK->first_block = EB;
               }
            else
               { 
               MB->next_block = EB;
               MB->dirty = 1;
               BANK_assert(MB->block_image->next_dirblk == -1);
               }

            BFDIRBLK *NB = EB->block_image;

            NB->dirblk_bytes = dirblk_bytes;
            NB->used_bytes   = need_blk_bytes;
            NB->next_dirblk  = -1;      // (to be filled in later, when writing to disk)
            NB->n_entries    = 1;

            ND = (BFDIRENT *) (((char *) NB) + sizeof(BFDIRBLK));
            NE = EB->entries;
            }

         NE->entry = ND;
         NE->block = EB;
         }

      if (enough_data_space)
         {
         //
         // Entry's allocated space is large enough to hold the new data, but its 
         // metadata outgrew the old directory entry.  Copy the attributes of the 
         // existing directory entry to the new, larger one, leaving the data in place...
         // and mark the existing entry as unused so it can be reused later
         //

         ND->data_offset     = existing->data_offset;
         ND->allocated_bytes = existing->allocated_bytes;

         existing->BFE_flags       = BFE_UNUSED_ENTRY;
         existing->allocated_bytes = 0;

         extra_bytes = 0;
         }
      else
         {
         //
         // Abandon the existing entry's data space, if any -- it's no longer big enough
         // for this entry, but it can be reused for another one
         // 
         // (This also has the effect of clearing the deleted flag for 
         // replaced entries of the same name)
         //

         if (existing != NULL)
            {
            existing->BFE_flags = BFE_SPACE_AVAILABLE;
            }

         if (RE != NULL)
            {
            //
            // Reuse space assigned to the deleted/abandoned data block we found above
            //

            BFDIRENT *RD = RE->entry;
            BANK_assert(RD != ND);             // (We can't reuse space from an unused entry)

            ND->data_offset     = RD->data_offset;
            ND->allocated_bytes = RD->allocated_bytes;

            RD->BFE_flags       = BFE_UNUSED_ENTRY;
            RD->allocated_bytes = 0;

            extra_bytes = 0;
            RE->block->dirty = 1;
            }
         else
            {
            //
            // No qualified lost space available for reuse -- set up to add the data at EOF,
            // plus any extra padding for future growth
            // 

            ND->data_offset = (BANKSIZE) BANK->header.file_size;

            ND->allocated_bytes = BANK_padding(storage_alignment, ND->data_offset) + n_bytes + extra_bytes;

            BANK->header.file_size += ND->allocated_bytes;
            }
         }
      }

   //
   // Update the (new or reused) directory entry with the supplied metadata
   //

   NE->block->dirty = 1;

   ND->data_alignment = storage_alignment;

   NE->name              = ((char *) ND) + sizeof(BFDIRENT);
   NE->original_filename = NE->name              + resname_bytes;
   NE->user_string       = NE->original_filename + orgname_bytes;

   if (resource_name != NULL)
      BANK_memcpy(NE->name, resource_name, resname_bytes);
   else
      *NE->name = 0;

   if (original_filename != NULL)
      BANK_memcpy(NE->original_filename, original_filename, orgname_bytes);
   else
      *NE->original_filename = 0;
   
   if (user_string != NULL)
      BANK_memcpy(NE->user_string, user_string, userstr_bytes);
   else
      *NE->user_string = 0;

   S32 need_pad_bytes = BANK_padding(storage_alignment, ND->data_offset);

   ND->BFE_flags       = 0;
   ND->user            = user_val;
   ND->used_bytes      = n_bytes; 
   ND->created_time    = (existing != NULL) ? existing->created_time : cur_time;
   ND->modified_time   = cur_time;

   //
   // Did we shrink an existing directory entry?  If so, move all of the
   // subsequent entries down to free up space at the end of the block
   // 

   S32 delta = ND->dir_entry_bytes - need_entry_bytes;

   ND->dir_entry_bytes = need_entry_bytes;

   if (delta > 0)
      {
      BFDIRBLK *NB = NE->block->block_image;

      U8 *end_of_entry = ((U8 *) ND) + ND->dir_entry_bytes;
      U8 *next_entry   = end_of_entry + delta;

      U8 *end_of_block = ((U8 *) NB) + NB->used_bytes;
      NB->used_bytes -= delta;

      BANK_assert(end_of_block >= next_entry);

      memmove(end_of_entry,
              next_entry,
              end_of_block - next_entry);

      for (S32 i=0; i < NB->n_entries; i++)
         {
         BFMENT   *E = &NE->block->entries[i];
         BFDIRENT *D = E->entry;

         if (D > ND)
            {
            E->entry = (BFDIRENT *) (((U8 *) E->entry) - delta);

            E->name              -= delta;
            E->original_filename -= delta;
            E->user_string       -= delta;
            }
         }
      }

   //
   // Write the data block (unless no new data was submitted)
   //
   // Directory blocks are not written until the file is flushed
   //

   if (src != NULL)
      {
      //
      // Seek to data offset
      //

      BANKSIZE r = BANK_seek(BANK->file_handle,
                             ND->data_offset);

      if (result == -1)
         {
         return BANK_IO_ERROR;
         }

      //
      // Write any zero padding needed to enforce alignment
      //

      if (need_pad_bytes)
         {
         void *dummy = BANK_malloc(need_pad_bytes);

         if (dummy == NULL)
            {
            return BANK_OUT_OF_MEMORY;
            }

         memset(dummy, 0, need_pad_bytes);

         r = BANK_write(BANK->file_handle,
                        dummy,
                        need_pad_bytes);
             
         if (r != need_pad_bytes)
            {
            return BANK_WRITE_ERROR;
            }

         BANK_free(dummy);
         }

      //
      // Checksum and write the data
      //

      BeginCRC32(&ND->data_CRC);

      AddToCRC32(&ND->data_CRC,
                  n_bytes,
                  src);

      EndCRC32(&ND->data_CRC);

      r = BANK_write(BANK->file_handle,
                     src,
                     n_bytes);
          
      if (r != n_bytes)
         {
         return BANK_WRITE_ERROR;
         }

      //
      // Fill extra space, if any, with zeroes
      //

      if (extra_bytes)
         {
         void *dummy = BANK_malloc(extra_bytes);

         if (dummy == NULL)
            {
            return BANK_OUT_OF_MEMORY;
            }

         memset(dummy, 0, extra_bytes);

         r = BANK_write(BANK->file_handle,
                        dummy,
                        extra_bytes);
             
         if (r != extra_bytes)
            {
            return BANK_WRITE_ERROR;
            }

         BANK_free(dummy);
         }
      }

   //
   // Return OK if all bytes successfully written to file
   //

   BANK->need_flush = 1;

   return BANK_OK;
}

#endif

//***************************************************************************
//
// BANK_open()
//
// Used by both BankOpenReadOnly() and BankOpenReadWrite()
//
//***************************************************************************

static BANKRESULT BANK_open(HBANKFILE  BANK,
                            HBANKFILE *handle)      // (where to write the handle if open OK)
{
   //
   // Seek to header if it's not at the beginning of the physical file
   //
      
   if (BANK->container_offset > 0)
      {
      BANKSIZE r = BANK_seek(BANK->file_handle,
                             BANK->container_offset);
      if (r == -1)
         {
         BankClose(BANK);
         return BANK_IO_ERROR;
         }
      }

   //
   // Read and check the header
   //
   // Currently, there's no way to work with a file written by an API
   // compiled with a different BANKSIZE
   //
         
   BANKSIZE r = BANK_read(BANK->file_handle,
                         &BANK->header,
                          sizeof(BANK->header));

   if (r != sizeof(BANK->header))
      {
      BankClose(BANK);
      return BANK_READ_ERROR;
      }

   if (BANK->header.BANKSIZE_bytes > sizeof(BANKSIZE))
      {
      BankClose(BANK);
      return BANK_SIZE_TYPE_TOO_LARGE;
      }

   if (BANK->header.BANKSIZE_bytes < sizeof(BANKSIZE))
      {
      BankClose(BANK);
      return BANK_SIZE_TYPE_TOO_SMALL;
      }

   U32 header_CRC = BANK->header.header_CRC;
   BANK->header.header_CRC = 0;

   U32 CRC;
   BeginCRC32(&CRC);

   AddToCRC32(&CRC,
               sizeof(BANK->header),
              &BANK->header);

   EndCRC32(&CRC);

   if (CRC != header_CRC)
      {
      BankClose(BANK);
      return BANK_CORRUPT;
      }

   //
   // Make sure this is a supported bankfile
   //

   if (BANK_strnicmp(BANK->header.sig, BF_FILE_TYPE_ID, sizeof(BANK->header.sig)))
      {
      BankClose(BANK);
      return BANK_INVALID_TYPE;
      }

   if (BANK->header.bankfile_version != SUPPORTED_BF_VERSION)
      {
      BankClose(BANK);
      return BANK_NOT_SUPPORTED;
      }

   //
   // For each directory block in file...
   //
  
   BFMBLK **next_link = &BANK->first_block;
      
   BANKSIZE db_offset = TO_BANKSIZE(BANK->header.first_dirblk);
             
   while (db_offset != -1)
      {
      //
      // Allocate BFMBLK to maintain memory-resident copy of directory block and 
      // entry-pointer array
      //

      BFMBLK *B = (BFMBLK *) BANK_malloc(sizeof(BFMBLK));

      *next_link = B;
      next_link  = &B->next_block;

      if (B == NULL)
         {
         BankClose(BANK);
         return BANK_OUT_OF_MEMORY;
         }

      memset(B, 0, sizeof(BFMBLK));

      B->dirty = 0;
      B->block_offset = BANK->container_offset + db_offset;

      //
      // Read BFDIRBLK header to get the block's allocation size
      //

      BFDIRBLK header;
      memset(&header, 0, sizeof(header));

      BANKSIZE r = BANK_seek(BANK->file_handle,
                             B->block_offset);
      if (r == -1)
         {
         BankClose(BANK);
         return BANK_IO_ERROR;
         }

      r = BANK_read(BANK->file_handle,
                   &header,
                    sizeof(header));

      if (r != sizeof(header))
         {
         BankClose(BANK);
         return BANK_READ_ERROR;
         }

      //
      // Allocate the necessary space and copy our BFDIRBLK header to it
      //

      B->block_image = (BFDIRBLK *) BANK_malloc(header.dirblk_bytes);

      if (B->block_image == NULL)
         {
         BankClose(BANK);
         return BANK_OUT_OF_MEMORY;
         }

      memset(B->block_image, 0, header.dirblk_bytes);

      *B->block_image = header;

      //
      // Read rest of directory block
      // 

      BANKSIZE remnant_bytes = header.dirblk_bytes - sizeof(header);

      r = BANK_read(BANK->file_handle,
          ((char *) B->block_image) + sizeof(header),
                    remnant_bytes);

      if (r != remnant_bytes)
         {
         BankClose(BANK);
         return BANK_READ_ERROR;
         }

      BANK_assert(B->block_image->used_bytes <= header.dirblk_bytes);

      //
      // Checksum it
      //

      U32 dirblk_CRC = B->block_image->dirblk_CRC;
      B->block_image->dirblk_CRC = 0;

      U32 CRC;
      BeginCRC32(&CRC);

      AddToCRC32(&CRC,
                  header.dirblk_bytes,
                  B->block_image);

      EndCRC32(&CRC);

      if (CRC != dirblk_CRC)
         {
         BankClose(BANK);
         return BANK_CORRUPT;
         }

      //
      // Allocate block of BFMENT index entries and tie them to the 
      // list of BFDIRENTs following the BFDIRBLK header
      // 

      S32 entry_bytes = B->block_image->n_entries * sizeof(BFMENT);

      if (entry_bytes == 0)
         {
         B->entries = NULL;
         }
      else
         {
         B->entries = (BFMENT *) BANK_malloc(entry_bytes);

         if (B->entries == NULL)
            {
            BankClose(BANK);
            return BANK_OUT_OF_MEMORY;
            }

         memset(B->entries, 0, entry_bytes);

         BFDIRENT *DE = (BFDIRENT *) (((char *) B->block_image) + sizeof(BFDIRBLK));

         for (S32 i=0; i < B->block_image->n_entries; i++)
            {
            BFMENT *E = &B->entries[i];

            E->entry = DE;
            E->block = B;

            char *ptr = (char *) DE;

            ptr += sizeof(BFDIRENT);      // Locate name string after BFDIRENT
            E->name  = ptr;
            ptr += strlen(ptr)+1;         // Locate original_filename string after name
            E->original_filename = ptr;
            ptr += strlen(ptr)+1;         // Locate user_string after original_filename
            E->user_string = ptr;
            ptr += strlen(ptr)+1;         // Locate next BFDIRENT entry, if any

            BANK_assert((((char *) ptr) - ((char *) DE)) == DE->dir_entry_bytes);
            DE = (BFDIRENT *) ptr;
            }

         BANK_assert((((char *) DE) - ((char *) B->block_image)) == header.used_bytes);
         }

      db_offset = B->block_image->next_dirblk;                                                                          
      }

   //
   // List of in-memory directory blocks must be terminated at all times, so
   // close() can deallocate them
   //

   BANK_assert(*next_link == NULL);

   *handle = BANK;

   return BANK_OK;
}

static void ____________________________________________________Publics____________________________________________________(){}

//***************************************************************************
//
// BankResultText()
//
//***************************************************************************

BANK_RESULT_TEXT(char *, BankResultText) (BANKRESULT result) 
{
   switch (result)
      {
      default: break;
      case BANK_OK:                  return "No error";
      case BANK_OUT_OF_MEMORY:       return "Out of memory";
      case BANK_INVALID_OBJECT:      return "Bad/NULL handle";
      case BANK_INVALID_REQUEST:     return "Invalid request";
      case BANK_INVALID_TYPE:        return "Unsupported file type";
      case BANK_SIZE_TYPE_TOO_SMALL: return "32-bit bankfile cannot be opened by application compiled with BANKSIZE=S64";
      case BANK_SIZE_TYPE_TOO_LARGE: return "64-bit bankfile cannot be opened by application compiled with BANKSIZE=S32";
      case BANK_INVALID_FILE_HANDLE: return "File handle not valid";
      case BANK_NOT_SUPPORTED:       return "File version not supported";
      case BANK_CORRUPT:             return "File is corrupt (failed CRC or other integrity check)";
      case BANK_NOT_WRITABLE:        return "File is not open for writing";
      case BANK_IO_ERROR:            return "General file access error";
      case BANK_READ_ERROR:          return "Failed to read expected amount of data (e.g., file truncated)";
      case BANK_WRITE_ERROR:         return "Failed to write data (disk full?)";
      case BANK_FILE_NOT_FOUND:      return "File not found";
      case BANK_OPEN_FAILURE:        return "Couldn't open file for writing";
      case BANK_ENTRY_NOT_FOUND:     return "Entry not found";
      case BANK_ENTRY_DELETED:       return "Entry deleted";
      case BANK_GENERAL_FAILURE:     return "Unable to perform the requested operation";
      }

   return "Unknown error"; 
}

//***************************************************************************
//
// BankFlush()
//
//***************************************************************************

#ifdef BANK_WRITE_SUPPORT

BANK_FLUSH(BANKRESULT,BankFlush)(HBANKFILE BANK)
{
   if (BANK == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   if (!BANK->file_handle_valid)
      {
      return BANK_INVALID_FILE_HANDLE;
      }

   if (BANK->container_offset != 0)
      {
      return BANK_NOT_WRITABLE;             // We don't support writing to embedded files 
      }

   if (!BANK->need_flush)
      {
      return BANK_OK;
      }

   BANK->need_flush = 0;

   //
   // For all valid directory blocks in file...
   //

   BANK->header.directory_bytes      = 0;
   BANK->header.directory_bytes_used = 0;
   BANK->header.data_bytes           = 0;
   BANK->header.data_bytes_used      = 0;
   BANK->header.n_dirblks            = 0;
   BANK->header.n_entries            = 0;

   BANKSIZE *prev_offset_link = (BANKSIZE *) &BANK->header.first_dirblk;

   BFMBLK *MB = BANK->first_block;

   while (MB != NULL)
      {
      //
      // Assign this block a valid offset at EOF, if it has never
      // been written
      //

      if (MB->block_offset == -1)
         {
         MB->block_offset = (BANKSIZE) BANK->header.file_size;  
         BANK->header.file_size += MB->block_image->dirblk_bytes;

         BANK_assert(*prev_offset_link == -1);
         *prev_offset_link = MB->block_offset;
         }

      //
      // Audit # of entries and amount of data referenced by this directory block
      // 

      BANK->header.n_dirblks++;
      BANK->header.directory_bytes += MB->block_image->dirblk_bytes; 

      BANK->header.n_entries += MB->block_image->n_entries;

      for (S32 i=0; i < MB->block_image->n_entries; i++)
         {
         BFMENT *E = &MB->entries[i];

         if (!(E->entry->BFE_flags & BFE_UNUSED_ENTRY))
            {
            BANK->header.data_bytes += E->entry->allocated_bytes;

            if (!(E->entry->BFE_flags & BFE_SPACE_AVAILABLE))
               {
               BANK->header.data_bytes_used      += E->entry->used_bytes;
               BANK->header.directory_bytes_used += E->entry->dir_entry_bytes;
               }
            }
         }

      prev_offset_link = &MB->block_image->next_dirblk;
      MB = MB->next_block;
      }

   //
   // Write any 'dirty' directory blocks back to the file
   //

   MB = BANK->first_block;

   while (MB != NULL)
      {
      if (MB->dirty)
         {
         MB->dirty = 0;

         //
         // Calculate new CRC
         //

         MB->block_image->dirblk_CRC = 0;

         U32 CRC;
         BeginCRC32(&CRC);

         AddToCRC32(&CRC,
                     MB->block_image->dirblk_bytes,
                     MB->block_image);

         EndCRC32(&CRC);

         MB->block_image->dirblk_CRC = CRC;

         //
         // Write the block
         // 

         BANKSIZE r = BANK_seek(BANK->file_handle,
                                MB->block_offset);

         if (r == -1)
            {
            return BANK_IO_ERROR;
            }

         r = BANK_write(BANK->file_handle,
                        MB->block_image,
                        MB->block_image->dirblk_bytes);
          
         if (r != MB->block_image->dirblk_bytes)
            {
            return BANK_WRITE_ERROR;
            }
         }

      MB = MB->next_block;
      }

   //
   // Checksum and write updated file header
   //

   BANKSIZE r = BANK_seek(BANK->file_handle,
                          0);
   if (r == -1)
      {
      return BANK_IO_ERROR;
      }

   BANK->header.modified_time = BANK_system_time(); 

   BANK->header.header_CRC = 0;

   U32 CRC;
   BeginCRC32(&CRC);

   AddToCRC32(&CRC,
               sizeof(BANK->header),
              &BANK->header);

   EndCRC32(&CRC);

   BANK->header.header_CRC = CRC;

   r = BANK_write(BANK->file_handle,
                 &BANK->header,
                  sizeof(BANK->header));

   if (r != sizeof(BANK->header))
      {
      return BANK_WRITE_ERROR;
      }

   return BANK_OK;
}
#endif

//***************************************************************************
//
// BankCloseFileHandle()
//
//***************************************************************************

BANK_CLOSE_FILE_HANDLE(BANKRESULT,BankCloseFileHandle)(HBANKFILE BANK)
{
   if (BANK == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   if (!BANK->file_handle_valid)
      {
      return BANK_INVALID_FILE_HANDLE;
      }

   BANKRESULT result = BANK_OK;

#ifdef BANK_WRITE_SUPPORT
   if (BANK->need_flush)
      {
      result = BankFlush(BANK);
      }
#endif

   BANK_close(BANK->file_handle);
   BANK->file_handle = -1;

   BANK->file_handle_valid = 0;

   return result;
}

//***************************************************************************
//
// BankClose()
//
//***************************************************************************

BANK_CLOSE(BANKRESULT,BankClose)(HBANKFILE BANK)
{
   if (BANK == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   BANKRESULT result = BANK_OK;

   if (BANK->file_handle_valid)
      {
      BankCloseFileHandle(BANK);
      }

   BFMBLK *MB = BANK->first_block;

   while (MB != NULL)
      {
      BFMBLK *OB = MB;
      MB = MB->next_block;

      if (OB->block_image != NULL)
         {
         BANK_free(OB->block_image);
         OB->block_image = NULL;
         }

      if (OB->entries != NULL)
         {
         BANK_free(OB->entries);
         OB->entries = NULL;
         }

      BANK_free(OB);
      }

   if (BANK->filename != NULL)
      {
      BANK_free(BANK->filename);
      BANK->filename = NULL;
      }

   BANK_free(BANK);

   return result;
}

//***************************************************************************
//
// BankCreate()
//
//***************************************************************************

#ifdef BANK_WRITE_SUPPORT

BANK_CREATE(BANKRESULT,BankCreate) (HBANKFILE *handle, 
                                    char      *filename,
                                    BANKSIZE   initial_dirblk_bytes,
                                    BANKSIZE   default_dirblk_bytes)
{
   if (handle == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   *handle = NULL;

   HBANKFILE BANK = (HBANKFILE) BANK_malloc(sizeof(_BANKFILE));

   if (BANK == NULL)
      {
      return BANK_OUT_OF_MEMORY;
      }

   memset(BANK, 0, sizeof(_BANKFILE));

   //
   // Create new file (even if it already exists)
   //

   BANK->filename = (char *) BANK_malloc(strlen(filename)+1);

   if (BANK->filename == NULL)
      {
      BankClose(BANK);
      return BANK_OUT_OF_MEMORY;
      }

   strcpy(BANK->filename, filename);

   BANK->container_offset = 0;
   BANK->open_for_writing = 1;

   BANK->file_handle = BANK_open_create(filename); 

   if (BANK->file_handle == -1)
      {
      BankClose(BANK);
      return BANK_OPEN_FAILURE;
      }

   BANK->file_handle_valid = 1;

   //
   // Initialize in-memory header image
   //

   BANK_memcpy(BANK->header.sig,BF_FILE_TYPE_ID,sizeof(BANK->header.sig));
   BANK->header.bankfile_version = SUPPORTED_BF_VERSION;

   BANK->header.BANKSIZE_bytes       =  sizeof(BANKSIZE);
   BANK->header.file_size            =  sizeof(BFHDR);
   BANK->header.first_dirblk         = -1;
   BANK->header.default_dirblk_bytes =  default_dirblk_bytes;
   BANK->header.initial_dirblk_bytes =  initial_dirblk_bytes;
   BANK->header.created_time         =  BANK_system_time(); 
   BANK->header.modified_time        =  BANK->header.created_time;

   BANK->need_flush = 1; 

   //
   // If requested, create an initial directory block at the beginning
   // of the file, so it can be opened at runtime without extra seeking
   // overhead
   //

   if (initial_dirblk_bytes > 0)
      {
      BFMBLK *B = (BFMBLK *) BANK_malloc(sizeof(BFMBLK));

      if (B == NULL)
         {
         BankClose(BANK);
         return BANK_OUT_OF_MEMORY;
         }

      memset(B, 0, sizeof(BFMBLK));

      B->block_image = (BFDIRBLK *) BANK_malloc(initial_dirblk_bytes);

      if (B->block_image == NULL)
         {
         BANK_free(B);
         BankClose(BANK);
         return BANK_OUT_OF_MEMORY;
         }
               
      memset(B->block_image, 0, initial_dirblk_bytes);

      B->block_offset = (BANKSIZE) BANK->header.file_size;
      BANK->header.file_size += initial_dirblk_bytes;

      BANK->header.first_dirblk = B->block_offset;

      B->entries = NULL;      // No entries yet (NULL is legal)
      B->dirty = 1;

      B->next_block = NULL;
      BANK->first_block = B;

      B->block_image->next_dirblk  = -1;
      B->block_image->dirblk_bytes = initial_dirblk_bytes;
      B->block_image->n_entries    = 0;
      B->block_image->used_bytes   = sizeof(BFDIRBLK);
      }

   *handle = BANK;

   return BANK_OK;
}

//***************************************************************************
//
// BankOpenReadWrite()
//
//***************************************************************************

BANK_OPEN_READ_WRITE(BANKRESULT,BankOpenReadWrite) (HBANKFILE  *handle,
                                                    const char *filename)
{
   if (handle == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   *handle = NULL;

   HBANKFILE BANK = (HBANKFILE) BANK_malloc(sizeof(_BANKFILE));

   if (BANK == NULL)
      {
      return BANK_OUT_OF_MEMORY;
      }

   memset(BANK, 0, sizeof(_BANKFILE));

   //
   // Open existing bankfile for reading/writing
   //
   // Specification of a valid header offset implies that we're reading 
   // an existing embedded file
   //

   BANK->filename = (char *) BANK_malloc(strlen(filename)+1);

   if (BANK->filename == NULL)
      {
      BankClose(BANK);
      return BANK_OUT_OF_MEMORY;
      }

   strcpy(BANK->filename, filename);

   BANK->open_for_writing = TRUE;
   BANK->container_offset = 0;

   BANK->file_handle = BANK_open_read_write(filename);

   if (BANK->file_handle == -1)
      {
      BankClose(BANK);
      return BANK_OPEN_FAILURE;
      }

   BANK->file_handle_valid = 1;

   return BANK_open(BANK, handle);
}

#endif

//***************************************************************************
//
// BankOpenReadOnly()
//
//***************************************************************************

BANK_OPEN_READ_ONLY(BANKRESULT,BankOpenReadOnly) (HBANKFILE  *handle,
                                                  const char *filename,
                                                  BANKSIZE    embedded_at_offset)
{
   if (handle == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   *handle = NULL;

   HBANKFILE BANK = (HBANKFILE) BANK_malloc(sizeof(_BANKFILE));

   if (BANK == NULL)
      {
      return BANK_OUT_OF_MEMORY;
      }

   memset(BANK, 0, sizeof(_BANKFILE));

   //
   // Open existing bankfile for reading/writing
   //
   // Specification of a valid header offset implies that we're reading 
   // an existing embedded file
   //

   BANK->filename = (char *) BANK_malloc(strlen(filename)+1);

   if (BANK->filename == NULL)
      {
      BankClose(BANK);
      return BANK_OUT_OF_MEMORY;
      }

   strcpy(BANK->filename, filename);

   BANK->open_for_writing = FALSE;
   BANK->container_offset = embedded_at_offset;

   BANK->file_handle = BANK_open_read_only(filename);

   if (BANK->file_handle == -1)
      {
      BankClose(BANK);
      return BANK_FILE_NOT_FOUND;
      }

   BANK->file_handle_valid = 1;

   return BANK_open(BANK, handle);
}

//***************************************************************************
//
// BankWriteEntry()
//
//***************************************************************************

#ifdef BANK_WRITE_SUPPORT

BANK_WRITE_ENTRY(BANKRESULT,BankWriteEntry)(HBANKFILE         BANK,
                                            void             *src,
                                            BANKSIZE          n_bytes,
                                            char             *resource_name, 
                                            BANKENTRYATTRIBS *attribs)
{                                           
   return BANK_write_entry(BANK,
                           src,
                           n_bytes,
                           resource_name,
                           NULL,
                           attribs);
}

#endif

//***************************************************************************
//
// BankReadEntry()
//
//***************************************************************************

BANK_READ_ENTRY(BANKRESULT,BankReadEntry) (HBANKFILE BANK,
                                           char     *resource_name, 
                                           void    **dest,
                                           BANKSIZE *n_bytes_read,
                                           BANKSIZE  n_bytes_requested,
                                           BANKSIZE  read_at_offset)
{
   if (n_bytes_read) 
      {
      *n_bytes_read = 0;      // Assume failure
      }

   if ((BANK == NULL) || (resource_name == NULL) || (dest == NULL))
      {
      return BANK_INVALID_OBJECT;
      }

   if (!BANK->file_handle_valid)
      {
      return BANK_INVALID_FILE_HANDLE;
      }

   if ((read_at_offset < 0) || (n_bytes_requested < 0))
      {                          
      return BANK_INVALID_REQUEST;
      }

   //
   // Look up entry in RAM-resident file directory
   //

   BFMENT *E;
   BANKRESULT result = BANK_find_entry(BANK, &E, resource_name);

   if (result != BANK_OK)
      {
      return result;         // E.g., entry not found
      }

   //
   // Get size of data block to return, clamping it to the remaining portion of the
   // entry
   //

   BFDIRENT *D = E->entry;

   BANKSIZE bytes_left_at_offset = D->used_bytes - read_at_offset; 

   if (bytes_left_at_offset <= 0)
      {
      return BANK_INVALID_REQUEST;    // Specified offset was >= the data size
      }

   if ((n_bytes_requested == 0) || (n_bytes_requested > bytes_left_at_offset))
      {
      n_bytes_requested = bytes_left_at_offset;
      }

   //
   // Get amount of padding we need to skip to access the original resource data,
   // and seek to specified offset
   //

   BANKSIZE r = BANK_seek(BANK->file_handle, BANK->container_offset                          +                          
                                             D->data_offset                                  +                          
                                             BANK_padding(D->data_alignment, D->data_offset) +
                                             read_at_offset);
   if (r == -1)
      {
      return BANK_IO_ERROR;
      }

   //
   // If memory wasn't provided, allocate it
   // 

   if (*dest == NULL)
      {
      *dest = BANK_malloc(n_bytes_requested);
      }

   if (*dest == NULL)
      {
      return BANK_OUT_OF_MEMORY;
      }

   //
   // Read requested data 
   // A partial read is considered a failure
   //

   r = BANK_read(BANK->file_handle,
                *dest,
                 n_bytes_requested);

   if (n_bytes_read) 
      {
      *n_bytes_read = r;
      }

   if (r != n_bytes_requested)
      {
      return BANK_READ_ERROR;
      }
   
   return BANK_OK;
}

//***************************************************************************
//
// BankReadVerifyEntry()
//
//***************************************************************************

BANK_READ_VERIFY_ENTRY(BANKRESULT,BankReadVerifyEntry) (HBANKFILE BANK,
                                                        char     *resource_name,
                                                        void    **dest,
                                                        BANKSIZE *n_bytes_read)
{
   if (n_bytes_read) 
      {
      *n_bytes_read = 0;      // Assume failure
      }

   if ((BANK == NULL) || (resource_name == NULL) || (dest == NULL))
      {
      return BANK_INVALID_OBJECT;
      }

   if (!BANK->file_handle_valid)
      {
      return BANK_INVALID_FILE_HANDLE;
      }

   //
   // Look up entry in RAM-resident file directory
   //

   BFMENT *E;
   BANKRESULT result = BANK_find_entry(BANK, &E, resource_name);

   if (result != BANK_OK)
      {
      return result;         // E.g., entry not found
      }

   //
   // Get amount of padding we need to skip to access the original resource data
   //

   BFDIRENT *D = E->entry;

   //
   // Seek to specified offset
   //

   BANKSIZE r = BANK_seek(BANK->file_handle, BANK->container_offset + 
                                             D->data_offset         + 
                                             BANK_padding(D->data_alignment, D->data_offset)); 
   if (r == -1)
      {
      return BANK_IO_ERROR;
      }

   //
   // If memory wasn't provided, allocate it
   // 

   if (*dest == NULL)
      {
      *dest = BANK_malloc(D->used_bytes);
      }

   if (*dest == NULL)
      {
      return BANK_OUT_OF_MEMORY;
      }

   //
   // Read requested data 
   // A partial read is considered a failure
   //

   r = BANK_read(BANK->file_handle,
                *dest,
                 D->used_bytes);

   if (n_bytes_read) 
      {
      *n_bytes_read = r;
      }

   if (r != D->used_bytes)
      {
      return BANK_READ_ERROR;
      }

   //
   // Checksum it
   //

   U32 CRC;

   BeginCRC32(&CRC);

   AddToCRC32(&CRC,
               D->used_bytes,
              *dest);

   EndCRC32(&CRC);

   if (CRC != D->data_CRC)
      {
      return BANK_CORRUPT;
      }
   
   return BANK_OK;
}

//***************************************************************************
//
// BankFreeReadEntry()
//
//***************************************************************************

BANK_FREE_READ_ENTRY(BANKRESULT,BankFreeReadEntry) (HBANKFILE BANK,
                                                    void   **dest)
{
   if (dest == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   BANK_free(*dest);
   *dest = NULL;

   return BANK_OK;
}

#ifdef BANK_WRITE_SUPPORT

//***************************************************************************
//
// BankDeleteEntry()
//
//***************************************************************************

BANK_DELETE_ENTRY(BANKRESULT,BankDeleteEntry) (HBANKFILE BANK,
                                               char    *resource_name)
{
   if (BANK == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   if (!BANK->file_handle_valid)
      {
      return BANK_INVALID_FILE_HANDLE;
      }

   if ((BANK->container_offset != 0) || (!BANK->open_for_writing))
      {
      return BANK_NOT_SUPPORTED;            // We don't support writing to embedded files 
      }

   BFMENT *E;
   BANKRESULT result = BANK_find_entry(BANK, &E, resource_name); 

   if (result != BANK_OK)
      {
      return result;
      }

   //
   // Entry can be undeleted as long as BFE_DELETED is set
   //

   E->entry->BFE_flags = BFE_DELETED | BFE_SPACE_AVAILABLE;
   E->block->dirty = 1;

   BANK->need_flush = 1;

   return BANK_OK;
}

//***************************************************************************
//
// BankUndeleteEntry()
//
//***************************************************************************

BANK_UNDELETE_ENTRY(BANKRESULT,BankUndeleteEntry) (HBANKFILE BANK,
                                                   char    *resource_name)
{
   if (BANK == NULL)
      {
      return BANK_INVALID_OBJECT;
      }

   if (!BANK->file_handle_valid)
      {
      return BANK_INVALID_FILE_HANDLE;
      }

   if ((BANK->container_offset != 0) || (!BANK->open_for_writing))
      {
      return BANK_NOT_SUPPORTED;            // We don't support writing to embedded files 
      }

   BFMENT *E;
   BANKRESULT result = BANK_find_entry(BANK, &E, resource_name); 

   if (result != BANK_ENTRY_DELETED)
      {
      return BANK_INVALID_REQUEST;
      }

   E->entry->BFE_flags = 0;      // Restore the entry to health
   E->block->dirty = 1;

   BANK->need_flush = 1;

   return BANK_OK;
}

//***************************************************************************
//
// BankRenameEntry()
//
//***************************************************************************

BANK_RENAME_ENTRY(BANKRESULT,BankRenameEntry)(HBANKFILE         BANK,
                                              char             *resource_name, 
                                              char             *new_name)
{                                           
   return BANK_write_entry(BANK,
                           NULL,
                           0,
                           resource_name,
                           new_name,
                           NULL);
}

#endif

//***************************************************************************
//
// BankInfo()
//
//***************************************************************************

BANK_INFO(BANKRESULT,BankInfo)(HBANKFILE       BANK,
                               BANKFILEINFO *dest)
{
   if ((BANK == NULL) || (dest == NULL))
      {
      return BANK_INVALID_OBJECT;
      }

#ifdef BANK_WRITE_SUPPORT
   if (BANK->need_flush)
      {
      BANKRESULT result = BankFlush(BANK);

      if ((result != BANK_OK) && (result != BANK_INVALID_FILE_HANDLE))
         {
         return result;
         }
      }
#endif

   BANK_memcpy(dest->sig, BANK->header.sig, sizeof(dest->sig));                  

   dest->filename               =             BANK->filename;
   dest->file_handle            =             BANK->file_handle;
   dest->embedded_at_offset     =             BANK->container_offset;
   dest->bankfile_version       =             BANK->header.bankfile_version;     
   dest->created_time           =             BANK->header.created_time;
   dest->modified_time          =             BANK->header.modified_time;
   dest->n_entries              =             BANK->header.n_entries;         
   dest->n_dirblks              =             BANK->header.n_dirblks;            
   dest->file_size              = TO_BANKSIZE(BANK->header.file_size);
   dest->data_bytes             = TO_BANKSIZE(BANK->header.data_bytes);
   dest->data_bytes_used        = TO_BANKSIZE(BANK->header.data_bytes_used);           
   dest->directory_bytes        = TO_BANKSIZE(BANK->header.directory_bytes);
   dest->directory_bytes_used   = TO_BANKSIZE(BANK->header.directory_bytes_used);
   dest->minimum_directory_size = TO_BANKSIZE(BANK->header.directory_bytes_used) + sizeof(BFDIRBLK);
   dest->default_dirblk_bytes   = TO_BANKSIZE(BANK->header.default_dirblk_bytes); 
   dest->initial_dirblk_bytes   = TO_BANKSIZE(BANK->header.initial_dirblk_bytes);

   return BANK_OK;
}

//***************************************************************************
//
// BankEntryInfo()
//
//***************************************************************************

BANK_ENTRY_INFO(BANKRESULT,BankEntryInfo) (HBANKFILE        BANK,
                                           const char      *name,
                                           BANKENTRYINFO   *dest)
{
   if ((BANK == NULL) || (name == NULL) || (dest == NULL))
      {
      return BANK_INVALID_OBJECT;
      }

   BFMENT *info = NULL;

   BANKRESULT result = BANK_find_entry(BANK,
                                      &info,
                                       name);

   if ((result == BANK_OK) || (result == BANK_ENTRY_DELETED))
      {
      BANK_set_entry_info(BANK, 
                          info, 
                          dest);
      }

   return result;
}

//***************************************************************************
//
// BankEnumEntries()
//
//***************************************************************************

BANK_ENUM_ENTRIES(BANKRESULT,BankEnumEntries) (HBANKFILE        BANK,
                                               HBANKENUM       *cookie,
                                               BANKENTRYINFO   *dest)
{
   if ((BANK == NULL) || (cookie == NULL) || (dest == NULL))
      {
      return BANK_INVALID_OBJECT;
      }

   BFMBLK *MB;
   BFMENT *E = (BFMENT *) *cookie;

   if (E == NULL)                     // i.e., HBANKENUM_FIRST
      {
      //
      // Find first returnable entry in file
      //

      MB = BANK->first_block;

      while (MB != NULL)
         {
         S32 found = 0;

         for (S32 i=0; i < MB->block_image->n_entries; i++)
            {
            E = &MB->entries[i];

            if (BANK_returnable_entry(E))
               {
               found = 1;
               break;
               }
            }

         if (found)
            {
            break;
            }

         MB = MB->next_block;
         }

      if (MB == NULL)
         {
         *cookie = HBANKENUM_FIRST;
         return BANK_ENTRY_NOT_FOUND;  // No further directory blocks in file
         }
      }
   else                               // Valid entry was returned last time, find the next one...
      {
      do
         {
         MB = E->block;                         // Block in which last entry was found
         S32 i = (S32) (E - MB->entries) + 1;   // Index+1 of last entry found

         if (i < MB->block_image->n_entries)
            {
            //
            // Find next entry in this directory block
            //

            E = &MB->entries[i];
            }
         else
            {
            //
            // Find first entry in next (non-empty) directory block
            //

            do
               {
               MB = MB->next_block;

               if (MB == NULL)
                  {
                  *cookie = HBANKENUM_FIRST;
                  return BANK_ENTRY_NOT_FOUND;  // No further directory blocks in file
                  }
               }
            while (MB->block_image->n_entries == 0);

            E = &MB->entries[0];
            }
         }
      while (!BANK_returnable_entry(E));
      }

   BANK_set_entry_info(BANK,
                       E, 
                       dest);

   *cookie = (HBANKENUM) E;

   return BANK_OK;
}

//***************************************************************************
//
// BankCopy()  
//
//***************************************************************************

#ifdef BANK_WRITE_SUPPORT

BANK_COPY(BANKRESULT,BankCopy) (HBANKFILE SRC,
                                char     *dest_filename,
                                BANKSIZE  initial_dirblk_bytes,
                                BANKSIZE  default_dirblk_bytes)
{
   if ((SRC == NULL) || (dest_filename == NULL))
      {
      return BANK_INVALID_OBJECT;
      }

   BANKFILEINFO src_info;

   BANKRESULT result = BankInfo(SRC,
                               &src_info);
    
   if (result != BANK_OK)
      {
      return result;
      }

   HBANKFILE DEST;

   result = BankCreate(&DEST,
                        dest_filename,
                       (initial_dirblk_bytes == 0) ? src_info.minimum_directory_size : initial_dirblk_bytes,
                       (default_dirblk_bytes == 0) ? src_info.default_dirblk_bytes   : default_dirblk_bytes);

   if (result != NULL)
      {
      return result;
      }

   BANKENTRYINFO info;
   HBANKENUM cookie = HBANKENUM_FIRST;

   while ((result = BankEnumEntries(SRC,
                                   &cookie,
                                   &info)) == BANK_OK)
      {
      if (info.is_deleted)
         {
         continue;
         }

      void *data = NULL;
      BANKSIZE data_bytes = 0;

      result = BankReadVerifyEntry(SRC,
                                   info.name,
                                  &data,
                                  &data_bytes);

      if (result != BANK_OK)
         {
         BankClose(DEST);
         return result;
         }

      BANKENTRYATTRIBS attribs;
      memset(&attribs, 0, sizeof(attribs));

      attribs.original_filename = info.original_filename;
      attribs.user_string       = info.user_string;
      attribs.user_val          = info.user;
      attribs.storage_alignment = info.align_bytes;
      attribs.extra_bytes       = 0;

      result = BankWriteEntry(DEST,
                              data,
                              data_bytes,
                              info.name,
                             &attribs);

      BankFreeReadEntry(SRC,
                       &data);

      if (result != BANK_OK)
         {
         BankClose(DEST);
         return result;
         }
      }

   if (result != BANK_ENTRY_NOT_FOUND)
      {
      BankClose(DEST);
      return result;
      }

   return BankClose(DEST);
}

#endif   // BANK_WRITE_SUPPORT

#ifdef __cplusplus
}
#endif

