/*
 * $Id: cdk_symbols.h,v 1.28 Broadcom SDK $
 * $Copyright: Copyright 2013 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * Chip symbol table definitions.
 */

#ifndef __CDK_SYMBOLS_H__
#define __CDK_SYMBOLS_H__

/*******************************************************************************
 *
 * CDK SYMBOL FORMATS
 *
 *
 ******************************************************************************/

#include <cdk/cdk_types.h>


/*******************************************************************************
 *
 * Resolve compile depedencies for optimized code size reduction.
 *
 *
 ******************************************************************************/

#if CDK_CONFIG_INCLUDE_CHIP_SYMBOLS == 0
#undef CDK_CONFIG_INCLUDE_FIELD_INFO
#define CDK_CONFIG_INCLUDE_FIELD_INFO 0
#endif

#if CDK_CONFIG_INCLUDE_FIELD_INFO == 0
#undef CDK_CONFIG_INCLUDE_FIELD_NAMES
#define CDK_CONFIG_INCLUDE_FIELD_NAMES 0
#endif


/*
 * Symbol Flags
 *
 * The first 24 bits are reserved for architecture-specific purposes
 * such as block types and address extensions.
 *
 * The final 8 bits are defined as follows:
 * 
 */
#define CDK_SYMBOL_FLAG_START           0x01000000
#define CDK_SYMBOL_FLAG_REGISTER        0x01000000
#define CDK_SYMBOL_FLAG_PORT            0x02000000
#define CDK_SYMBOL_FLAG_COUNTER         0x04000000
#define CDK_SYMBOL_FLAG_MEMORY          0x08000000
#define CDK_SYMBOL_FLAG_R64             0x10000000
#define CDK_SYMBOL_FLAG_BIG_ENDIAN      0x20000000
#define CDK_SYMBOL_FLAG_MEMMAPPED       0x40000000
#define CDK_SYMBOL_FLAG_SOFT_PORT       0x80000000
#define CDK_SYMBOL_FLAG_LAST            0x80000000
#define CDK_SYMBOL_FLAGS_INVALID        0xffffffff


/*
 * Field Information Structure
 *
 * This structure defines a single field within a symbol
 */
typedef struct cdk_field_info_s {
    const char *name; 
    int fid; 
    uint16_t minbit;
    uint16_t maxbit; 
} cdk_field_info_t; 


/*
 * Index Information
 *
 * Some entries, such as memories and register arrays, have a size and minimum and 
 * maximum index value. 
 * 
 * These are combined in one 32 bit word to save space as follows:
 *
 *      size:   8 bits
 *      min:    5 bits
 *      max:   16 bits
 *      flags:  3 bits
 *
 * The ENCODE macros are used to create the fields in the symbol table entry. 
 * Use the GET macro to retrieve it when parsing the symbol. 
 *
 * The following flags affect the interpretation of min and max:
 *
 * EXP
 * Used if real max cannot be contained within max field.
 * Real max is encoded as ((1 << max) * min) - 1)
 * Real min is always zero
 * Ex.: (max,min)=(12,17) => real max = ((1 << 12) * 17) - 1) = 0x10fff
 *
 * VAR
 * Used e.g. if register array size is port-dependent.
 * Real max and min depend on configuration
 * min is encoding key which may be passed to an arch-specific function
 * 
 * STEP
 * Used e.g. if address step for a register array is non-standard.
 * Real min is always zero
 * min is the address step - 1
 * Ex.: (max,min)=(11,3) => 12 elements with addr(i+1) = addr(i)+4
 *
 */

#define CDK_SYMBOL_INDEX_F_EXP  0x1
#define CDK_SYMBOL_INDEX_F_VAR  0x2
#define CDK_SYMBOL_INDEX_F_STEP 0x4

#define CDK_SYMBOL_INDEX_SIZE_ENCODE(s) __F_ENCODE((uint32_t)s, 0, 8)
#define CDK_SYMBOL_INDEX_SIZE_GET(w) __F_GET(w, 0, 8)
#define CDK_SYMBOL_INDEX_MIN_ENCODE(s) __F_ENCODE((uint32_t)s, 8, 5)
#define CDK_SYMBOL_INDEX_MIN_GETRAW(w) __F_GET(w, 8, 5)
#define CDK_SYMBOL_INDEX_MAX_ENCODE(s) __F_ENCODE((uint32_t)s, 13, 16)
#define CDK_SYMBOL_INDEX_MAX_GETRAW(w) __F_GET(w, 13, 16)
#define CDK_SYMBOL_INDEX_FLAGS_ENCODE(s) __F_ENCODE((uint32_t)s, 29, 3)
#define CDK_SYMBOL_INDEX_FLAGS_GET(w) __F_GET(w, 29, 3)

#define CDK_SYMBOL_INDEX_MIN_GET(w) \
    (CDK_SYMBOL_INDEX_FLAGS_GET(w) ? \
       0 : \
       CDK_SYMBOL_INDEX_MIN_GETRAW(w))

#define CDK_SYMBOL_INDEX_ENC_GET(w) CDK_SYMBOL_INDEX_MIN_GETRAW(w)

#define CDK_SYMBOL_INDEX_MAX_GET(w) \
    ((CDK_SYMBOL_INDEX_FLAGS_GET(w) & CDK_SYMBOL_INDEX_F_EXP) ? \
       (CDK_SYMBOL_INDEX_MIN_GETRAW(w) * (1 << CDK_SYMBOL_INDEX_MAX_GETRAW(w))) - 1 : \
       CDK_SYMBOL_INDEX_MAX_GETRAW(w))

#define CDK_SYMBOL_INDEX_STEP_GET(w) \
    ((CDK_SYMBOL_INDEX_FLAGS_GET(w) & CDK_SYMBOL_INDEX_F_STEP) ? \
       (CDK_SYMBOL_INDEX_MIN_GETRAW(w) + 1) : \
       1)


/*
 * Single Word Field Format
 *
 * Format:
 *
 * {LastField:31:31}{Ext:30:30}{FieldId:29:16}{Maxbit:15:8}{Minbit:7:0}
 *
 * Fields:
 *    LastField:1 Indicates this is the last field descriptor word in a field array
 *    Ext:1       Indicates that this is the first word in a Double Word Field, not
 *                a Single Word Field. This word and the next form a Double Word Field. 
 *    FieldId:14  This is the unique field id for this field. 
 *    Maxbit:8    This is the field's maxbit. 
 *    Minbit:8    This is the field's minbit. 
 *
 *
 *
 * Double Word Field Format
 *
 * Format:
 *
 *  {LastField:31:31}{Ext:30:30}{FieldId:29:0}
 *  {Maxbit:31:16}{Minbit:15:0}
 *
 * Fields:
 *     LastField:1 Indicates this is the last field descriptor in a field array. 
 *     Ext:1       Indicates this is the start of a Double Word field. 
 *     FieldId:30  This is the unique field id for this field. 
 *     Maxbit:16   This is the field's maxbit. 
 *     Minbit:16   This is the field's minbit. 
 *
 */

#define CDK_SYMBOL_FIELD_FLAG_LAST      (1L<<31)
#define CDK_SYMBOL_FIELD_FLAG_EXT       (1L<<30)

#define CDK_SYMBOL_FIELD_LAST(w) ((w) & CDK_SYMBOL_FIELD_FLAG_LAST)
#define CDK_SYMBOL_FIELD_EXT(w) ((w) & CDK_SYMBOL_FIELD_FLAG_EXT)

/* Single Word Fields */
#define CDK_SYMBOL_FIELD_ID_ENCODE(id) __F_ENCODE(id, 16, 14)
#define CDK_SYMBOL_FIELD_MAX_ENCODE(b) __F_ENCODE(b, 8, 8)
#define CDK_SYMBOL_FIELD_MIN_ENCODE(b) __F_ENCODE(b, 0, 8)

#define CDK_SYMBOL_FIELD_ID_GET(w) __F_GET(w, 16, 14)
#define CDK_SYMBOL_FIELD_MAX_GET(w) __F_GET(w, 8, 8)
#define CDK_SYMBOL_FIELD_MIN_GET(w) __F_GET(w, 0, 8)

#define CDK_SYMBOL_FIELD_ENCODE(id, max, min) \
CDK_SYMBOL_FIELD_ID_ENCODE(id) | \
CDK_SYMBOL_FIELD_MAX_ENCODE(max) | \
CDK_SYMBOL_FIELD_MIN_ENCODE(min)

/* Double Word Fields */
#define CDK_SYMBOL_FIELD_EXT_ID_ENCODE(id) __F_ENCODE(id, 0, 30)
#define CDK_SYMBOL_FIELD_EXT_MAX_ENCODE(b) __F_ENCODE(b, 16, 16)
#define CDK_SYMBOL_FIELD_EXT_MIN_ENCODE(b) __F_ENCODE(b, 0, 16)

#define CDK_SYMBOL_FIELD_EXT_ENCODE(id, max, min) \
CDK_SYMBOL_FIELD_FLAG_EXT | CDK_SYMBOL_FIELD_EXT_ID_ENCODE(id), \
CDK_SYMBOL_FIELD_EXT_MAX_ENCODE(max) | \
CDK_SYMBOL_FIELD_EXT_MIN_ENCODE(min)


#define CDK_SYMBOL_FIELD_EXT_ID_GET(w) __F_GET(w, 0, 30)
#define CDK_SYMBOL_FIELD_EXT_MAX_GET(w) __F_GET(w, 16, 16)
#define CDK_SYMBOL_FIELD_EXT_MIN_GET(w) __F_GET(w, 0, 16)

/* Operate on Field Descriptors */
extern uint32_t* cdk_field_info_decode(uint32_t* fp, 
                                       cdk_field_info_t* finfo, 
                                       const char** fnames);
extern uint32_t
cdk_field_info_count(uint32_t* fp);  

#define CDK_SYMBOL_FIELDS_ITER_BEGIN(start, finfo, fnames)     \
{                                                              \
    uint32_t* __fp = start;                                    \
    for(;;) {                                                  \
      if(!__fp) {                                              \
        break;                                                 \
      }                                                        \
      else {                                                   \
        __fp = cdk_field_info_decode(__fp, &(finfo), (fnames));    

#define CDK_SYMBOL_FIELDS_ITER_END() }}}
    

/*
 * Symbol Information Structure (single symbol)
 *
 */
typedef struct cdk_symbol_s {

    
    uint32_t addr; 
#if CDK_CONFIG_INCLUDE_FIELD_INFO == 1
    uint32_t* fields; 
#endif
    uint32_t index;     
    uint32_t flags; 
    const char *name; 
#if CDK_CONFIG_INCLUDE_ALIAS_NAMES == 1
    const char *alias; 
    const char *ufname; 
#endif

} cdk_symbol_t; 


/*
 * Symbols Information Structure (plural)
 */
typedef struct cdk_symbols_s {    
    
    const cdk_symbol_t *symbols; 
    const uint32_t size; 
    const char** field_names; 

} cdk_symbols_t; 


/*
 * Search a symbol array. Utility function. 
 */
extern const cdk_symbol_t *cdk_symbol_find(const char *name, 
                                           const cdk_symbol_t *table, 
                                           int size); 

/*
 * Primary symbol search function. Takes a pointer to the symbols structure, 
 * finds the name in either one, and returns a full symbol structure. 
 */

extern int cdk_symbols_find(const char *name, const cdk_symbols_t *symbols, 
                            cdk_symbol_t *rsym); 

/*
 * Get index of a specific symbol
 */
extern int cdk_symbols_index(const cdk_symbols_t *symbols,
                             const cdk_symbol_t *symbol);


/*
 * Get a specific symbol (by index)
 */
extern int cdk_symbols_get(const cdk_symbols_t *symbols, uint32_t sindex, 
                           cdk_symbol_t *rsym); 


/*
 * Search symbol table and call a user-defined function for each
 * matched symbol. Useful if multiple partial matches may occur.
 */

/* Matching modes */
#define CDK_SYMBOLS_ITER_MODE_EXACT     0
#define CDK_SYMBOLS_ITER_MODE_START     1
#define CDK_SYMBOLS_ITER_MODE_STRSTR    2

typedef struct cdk_symbols_iter_s {

    const char* name;   /* String to match */

    int matching_mode;  /* Defines valid matches */

    uint32_t pflags;    /* Flags that must be present */
    uint32_t aflags;    /* Flags that must be absent */

    const cdk_symbols_t* symbols; 

    int (*function)(const cdk_symbol_t *sym, void *vptr);
    void* vptr; 

} cdk_symbols_iter_t; 

typedef int (*cdk_symbol_filter_cb_t)(const cdk_symbol_t *symbol,
                                      const char **field_names,
                                      const char *encoding,
                                      void *cookie);


extern int cdk_symbols_iter(cdk_symbols_iter_t *iter);

extern int cdk_symbol_field_filter(const cdk_symbol_t *symbol,
                                   const char **fnames,
                                   const char *encoding,
                                   void *cookie);

extern int cdk_symbol_show_fields(const cdk_symbol_t *symbol,
                                  const char **fnames, uint32_t *data, int nz,
                                  cdk_symbol_filter_cb_t fcb, void *cookie);

extern int cdk_symbol_dump(const char *name, const cdk_symbols_t *symbols,
                           uint32_t *data);

#endif /* __CDK_SYMBOLS_H__ */
