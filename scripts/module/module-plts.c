#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sign_mod.h"

static bool duplicate_rel(const Elf64_Rela *rela, int num)
{
	/*
	 * Entries are sorted by type, symbol index and addend. That means
	 * that, if a duplicate entry exists, it must be in the preceding
	 * slot.
	 */
	return num > 0 && cmp_rela(rela + num, rela + num - 1) == 0;
}


static unsigned int count_plts(Elf64_Sym *syms, Elf64_Rela *rela, int num,
							   Elf64_Word dstidx, Elf64_Shdr *dstsec)
{
	unsigned int ret = 0;
	Elf64_Sym *s;
	int i;

	for (i = 0; i < num; i++)
	{
		u64 min_align;

		switch (ELF64_R_TYPE(rela[i].r_info)) {
		case R_AARCH64_JUMP26:
		case R_AARCH64_CALL26:
#ifdef RANDOMIZE_BASE
			break;
#endif
			s = syms + ELF64_R_SYM(rela[i].r_info);
			if (s->st_shndx == dstidx)
				break;

			if (rela[i].r_addend != 0 || !duplicate_rel(rela, i))
				ret++;
			break;
		case R_AARCH64_ADR_PREL_PG_HI21_NC:
		case R_AARCH64_ADR_PREL_PG_HI21:
			break;

			min_align = 2ULL << ffz(rela[i].r_offset | 0x7);

			if (min_align > 4096UL)
				ret++;
			else
				dstsec->sh_addralign = max(dstsec->sh_addralign,
										   min_align);
			break;
		}
	}
	return ret;
}

int module_frob_arm64_section(struct mod_info *mod)
{
	u64 core_plts = 0;
	u64 init_plts = 0;
	Elf64_Sym *syms = NULL;
	Elf64_Shdr *tramp = NULL;
	int i;

	/*
	 * Find the empty .plt section so we can expand it to store the PLT
	 * entries. Record the symtab address as well.
	 */
	for (i = 0; i < mod->hdr->e_shnum; i++)
	{
		Elf64_Shdr *hdr = &mod->sechdrs[i];
		if (!strcmp(mod->secstrings + hdr->sh_name, ".plt")) {
			mod->arch.core.plt = hdr;
		} else if (!strcmp(mod->secstrings + hdr->sh_name, ".init.plt")) {
			mod->arch.init.plt = hdr;
		} else if (DYNAMIC_FTRACE &&
				!strcmp(mod->secstrings + hdr->sh_name, ".text.ftrace_trampoline")) {
			tramp = hdr;
		} else if (mod->sechdrs[i].sh_type == SHT_SYMTAB)
			syms = (Elf64_Sym *)mod->sechdrs[i].sh_addr;
	}

	if (!mod->arch.core.plt || !mod->arch.init.plt)
	{
		fprintf(stderr, "module PLT section(s) missing\n");
		return -1;
	}
	if (!syms)
	{
		fprintf(stderr, "module symtab section missing\n");
		return -1;
	}

	for (i = 0; i < mod->hdr->e_shnum; i++) {
		Elf64_Rela *rels = (void *)mod->hdr + mod->sechdrs[i].sh_offset;
		int numrels = mod->sechdrs[i].sh_size / sizeof(Elf64_Rela);
		Elf64_Shdr *dstsec = mod->sechdrs + mod->sechdrs[i].sh_info;

		if (mod->sechdrs[i].sh_type != SHT_RELA)
			continue;

		/* ignore relocations that operate on non-exec sections */
		if (!(dstsec->sh_flags & SHF_EXECINSTR))
			continue;

		sort(rels, numrels, sizeof(Elf64_Rela), cmp_rela, NULL);
		
		if (strncmp(mod->secstrings + dstsec->sh_name, ".init", 5) != 0)
			core_plts += count_plts(syms, rels, numrels,
									mod->sechdrs[i].sh_info, dstsec);
		else
			init_plts += count_plts(syms, rels, numrels,
									mod->sechdrs[i].sh_info, dstsec);
	}
	

	mod->arch.core.plt->sh_type = SHT_NOBITS;
	mod->arch.core.plt->sh_flags = SHF_EXECINSTR | SHF_ALLOC;
	mod->arch.core.plt->sh_addralign = L1_CACHE_BYTES;
	mod->arch.core.plt->sh_size = (core_plts + 1) * sizeof(struct plt_entry);
	mod->arch.core.plt_num_entries = 0;
	mod->arch.core.plt_max_entries = core_plts;

	mod->arch.init.plt->sh_type = SHT_NOBITS;
	mod->arch.init.plt->sh_flags = SHF_EXECINSTR | SHF_ALLOC;
	mod->arch.init.plt->sh_addralign = L1_CACHE_BYTES;
	mod->arch.init.plt->sh_size = (init_plts + 1) * sizeof(struct plt_entry);
	mod->arch.init.plt_num_entries = 0;
	mod->arch.init.plt_max_entries = init_plts;
	
	if (tramp) {
		tramp->sh_type = SHT_NOBITS;
		tramp->sh_flags = SHF_EXECINSTR | SHF_ALLOC;
		tramp->sh_addralign = __alignof__(struct plt_entry);
		tramp->sh_size = sizeof(struct plt_entry);
	}

	return 0;
}
