#include <elf.h>
#include <linux/types.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

/* KERNEL CONFIG */
#define KALLSYMS 1
#define MODVERSIONS 1
#define MODULE_UNLOAD 1
#define RANDOMIZE_BASE 0
#define DYNAMIC_FTRACE 0

#define ARM64_ERRATUM_843419 1
#define ARM64_MODULE_PLTS 1
#define MAX_VERIFY_SECTION_SIZE 100
#define VERIFY_SECTION_SIZE 21
#define VERIFY_FLAG (1UL << 63)

#define L1_CACHE_SHIFT (6)
#define L1_CACHE_BYTES (1 << L1_CACHE_SHIFT)

#define SZ_64K 0x00010000
#define SZ_2M 0x00200000
#define SZ_128M 0x08000000

#define USHRT_MAX ((u16)(~0U))
#define SHRT_MAX ((s16)(USHRT_MAX >> 1))
#define SHRT_MIN ((s16)(-SHRT_MAX - 1))
#define INT_MAX ((int)(~0U >> 1))
#define INT_MIN (-INT_MAX - 1)
#define UINT_MAX (~0U)
#define LONG_MAX ((long)(~0UL >> 1))
#define LONG_MIN (-LONG_MAX - 1)
#define ULONG_MAX (~0UL)
#define LLONG_MAX ((long long)(~0ULL >> 1))
#define LLONG_MIN (-LLONG_MAX - 1)
#define ULLONG_MAX (~0ULL)

#define U8_MAX ((u8)~0U)
#define S8_MAX ((s8)(U8_MAX >> 1))
#define S8_MIN ((s8)(-S8_MAX - 1))
#define U16_MAX ((u16)~0U)
#define S16_MAX ((s16)(U16_MAX >> 1))
#define S16_MIN ((s16)(-S16_MAX - 1))
#define U32_MAX ((u32)~0U)
#define S32_MAX ((s32)(U32_MAX >> 1))
#define S32_MIN ((s32)(-S32_MAX - 1))
#define U64_MAX ((u64)~0ULL)
#define S64_MAX ((s64)(U64_MAX >> 1))
#define S64_MIN ((s64)(-S64_MAX - 1))

#define max(a, b) a > b ? a : b
#define BIT(nr) (1UL << (nr))
#define BITS_PER_LONG 64

#define SHN_LIVEPATCH	0xff20
#define SHF_RELA_LIVEPATCH 0x00100000

#define PAGE_SIZE getpagesize()
#define page_align(x) ((x + PAGE_SIZE - 1UL) & ~(PAGE_SIZE - 1UL))
#define ALIGN(x, align) ((x + align - 1UL) & ~(align - 1UL))
#define MAX_ERRNO 4095
#define IS_ERR_VALUE(x) (unsigned long)(void *)(x) >= (unsigned long)-MAX_ERRNO

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#ifndef MODVERSIONS
#define symversion(base, idx) NULL
#else
#define symversion(base, idx) ((base != NULL) ? ((base) + (idx)) : NULL)
#endif

struct kernel_symbol
{
	unsigned long value;
	const char *name;
};

struct symsearch
{
	const struct kernel_symbol *start, *stop;
	const s32 *crcs;
	enum
	{
		NOT_GPL_ONLY,
		GPL_ONLY,
		WILL_BE_GPL_ONLY,
	} licence;
	bool unused;
};

struct plt_entry
{
	__le32 mov0; /* movn	x16, #0x....			*/
	__le32 mov1; /* movk	x16, #0x...., lsl #16		*/
	__le32 mov2; /* movk	x16, #0x...., lsl #32		*/
	__le32 br;	 /* br	x16				*/
};

struct mod_tabs
{
	u64 syms;
	u64 num_syms;
	u64 crcs;
	u64 gpl_syms;
	u64 num_gpl_syms;
	u64 gpl_crcs;
	u64 gpl_future_syms;
	u64 num_gpl_future_syms;
	u64 gpl_future_crcs;
};

struct mod_plt_sec
{
	Elf64_Shdr *plt;
	int plt_num_entries;
	int plt_max_entries;
};

struct mod_arch_spec
{
	struct mod_plt_sec core;
	struct mod_plt_sec init;
};

struct verinfo {
	u64 size;
	u64 flags;
	u64 align_size;
	u64 info;
	u64 type;
	u64 link;
};

/* Module section info */
struct mod_secinfo
{
	u64 base;
	u64 buff_offset;
	struct verinfo vinfo;
};

/* Load module info */
struct mod_info
{
	Elf64_Ehdr *hdr;
	unsigned long len;
	Elf64_Shdr *sechdrs;
	char *secstrings, *strtab;
	u64 buff;
	u64 verify_size;
	u64 percpu;
	u64 percpu_size;
	struct mod_tabs modtabs;
	struct mod_arch_spec arch;
	struct
	{
		unsigned int sym, str, pcpu, info, vers;
	} index;
};

struct find_symbol_arg
{
	/* Input */
	const char *name;
	bool gplok;
	bool warn;

	/* Output */
	const s32 *crc;
	const struct kernel_symbol *sym;
};

/* module.c */
int simulate_load_module(void *usermod, unsigned long len, struct mod_info *mod);

/* moduleOps.c */
void layout_symtab(struct mod_info *mod);
void init_info(struct mod_secinfo *secinfo, struct mod_info *mod, u32 checklists[], u32 entsize);
void setup_buff(u64 buff, struct mod_secinfo *secinfo, u32 entsize);
void sign_module(u64 buff, size_t size);

/* moduleAux.c */
struct mod_secinfo init_sec(u32 idx, struct mod_info *mod);
unsigned int find_sec(const struct mod_info *mod, const char *name);

/* module-plts.c */
int module_frob_arm64_section(struct mod_info *mod);

/* helper.c */
void bin2hex(unsigned char *dst, const char *src, int count);
int hex2bin(unsigned char *dst, const char *src, int count);
int cmp_rela(const void *a, const void *b);
void sort(void *base, size_t num, size_t size,
	  int (*cmp_func)(const void *, const void *),
	  void (*swap_func)(void *, void *, int size));
int ffs(int x);
bool strstarts(const char *str, const char *prefix);
#define ffz(x) ffs(~(x))
