#pragma once

// taken from UVLoader
// https://github.com/yifanlu/UVLoader

/** \name Fixed width integers
 *  @{
 */
typedef unsigned char u8_t;             ///< Unsigned 8-bit type
typedef unsigned short int u16_t;       ///< Unsigned 16-bit type
typedef unsigned int u32_t;             ///< Unsigned 32-bit type
typedef unsigned long long int u64_t;        ///< Unsigned 64-bit type
typedef char s8_t;             ///< Signed 8-bit type
typedef short int s16_t;       ///< Signed 16-bit type
typedef int s32_t;             ///< Signed 32-bit type
typedef long long int s64_t;        ///< Signed 64-bit type
/** @}*/

/**
 * \brief SCE module information section
 * 
 * Can be found in an ELF file or loaded in 
 * memory.
 */
typedef struct module_info // thanks roxfan
{
    u16_t   modattribute;  // ??
    u16_t   modversion;    // always 1,1?
    char    modname[27];   ///< Name of the module
    u8_t    type;          // 6 = user-mode prx?
    void    *gp_value;     // always 0 on ARM
    u32_t   ent_top;       // beginning of the export list (sceModuleExports array)
    u32_t   ent_end;       // end of same
    u32_t   stub_top;      // beginning of the import list (sceModuleStubInfo array)
    u32_t   stub_end;      // end of same
    u32_t   module_nid;    // ID of the PRX? seems to be unused
    u32_t   field_38;      // unused in samples
    u32_t   field_3C;      // I suspect these may contain TLS info
    u32_t   field_40;      //
    u32_t   mod_start;     // module start function; can be 0 or -1; also present in exports
    u32_t   mod_stop;      // module stop function
    u32_t   exidx_start;   // ARM EABI style exception tables
    u32_t   exidx_end;     //
    u32_t   extab_start;   //
    u32_t   extab_end;     //
} module_info_t;

typedef struct module_imports_3x
{
    u16_t   size;               // size of this structure; 0x24 for Vita 3.x
    u16_t   lib_version;        //
    u16_t   attribute;          //
    u16_t   num_functions;      // number of imported functions
    u16_t   num_vars;           // number of imported variables
    u16_t   unknown1;
    u32_t   module_nid;         // NID of the module to link to
    char    *lib_name;          // name of module
    u32_t   *func_nid_table;    // array of function NIDs (numFuncs)
    void    **func_entry_table; // parallel array of pointers to stubs; they're patched by the loader to jump to the final code
    u32_t   *var_nid_table;     // NIDs of the imported variables (numVars)
    void    **var_entry_table;  // array of pointers to "ref tables" for each variable
} module_imports_3x_t;

/**
 * \brief SCE module export table
 * 
 * Can be found in an ELF file or loaded in 
 * memory.
 */
typedef struct module_exports // thanks roxfan
{
    u16_t   size;           // size of this structure; 0x20 for Vita 1.x
    u8_t    lib_version[2]; //
    u16_t   attribute;      // ?
    u16_t   num_functions;  // number of exported functions
    u32_t   num_vars;       // number of exported variables
    u32_t   num_tls_vars;   // number of exported TLS variables?  <-- pretty sure wrong // yifanlu
    u32_t   module_nid;     // NID of this specific export list; one PRX can export several names
    char    *lib_name;      // name of the export module
    u32_t   *nid_table;     // array of 32-bit NIDs for the exports, first functions then vars
    void    **entry_table;  // array of pointers to exported functions and then variables
} module_exports_t;
