#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <elf.h>

#include "sign_mod.h"

u32 checklists[MAX_VERIFY_SECTION_SIZE];
static u32 verify_entsize = 0;

static void divide_section(struct mod_info *mod)
{
	static unsigned long const masks[][2] = {
		{ SHF_EXECINSTR | SHF_ALLOC, 0 },
		{ SHF_ALLOC, SHF_WRITE },
	};

	unsigned int m, i, k;
	unsigned int symidx, stridx;
	symidx = mod->index.sym;
	stridx = mod->index.str;

	k = 0;

	for(m = 0; m < ARRAY_SIZE(masks); ++m) {

		for (i = 0; i < mod->hdr->e_shnum; ++i) {
			Elf64_Shdr *s = &mod->sechdrs[i];
			const char *sname = mod->secstrings + s->sh_name;

			if ((s->sh_flags & masks[m][0]) != masks[m][0]
			    || (s->sh_flags & masks[m][1])
				|| (s->sh_entsize & VERIFY_FLAG)
				|| strstarts(sname, ".init")
				|| (i == symidx) || (i == stridx))
				continue;
			checklists[k++] = i;
			s->sh_entsize |= VERIFY_FLAG;
		}
	}

	for(m = 0; m < ARRAY_SIZE(masks); ++m) {
		for (i = 0; i < mod->hdr->e_shnum; ++i) {
			Elf64_Shdr *s = &mod->sechdrs[i];
			const char *sname = mod->secstrings + s->sh_name;

			if ((s->sh_flags & masks[m][0]) != masks[m][0]
			    || (s->sh_flags & masks[m][1])
				|| (s->sh_entsize & VERIFY_FLAG)
				|| !strstarts(sname, ".init")
				|| (i == symidx) || (i == stridx))
				continue;

			checklists[k++] = i;
			s->sh_entsize |= VERIFY_FLAG;
		}
	}

	checklists[k++] = symidx;
	checklists[k++] = stridx;

	for(i = 0; i < mod->hdr->e_shnum; ++i){
		Elf64_Shdr *s = &mod->sechdrs[i];
		if (s->sh_type == SHT_RELA) {
			for (m = 0; m < k; ++m) {
				if (s->sh_info == checklists[m]) {
					checklists[k++] = i;
					break;
				}
			}
		}

	}
	verify_entsize = k;
}

static void init_section_headers(struct mod_info *mod)
{
	unsigned int i;
	mod->sechdrs[0].sh_addr = 0;

	for (i = 1; i < mod->hdr->e_shnum; i++) {
		Elf64_Shdr *shdr = &mod->sechdrs[i];
		if (shdr->sh_type != SHT_NOBITS 
				&& mod->len < shdr->sh_offset + shdr->sh_size) {
			fprintf(stderr, "Module len %lu truncated\n", mod->len);
			exit(1);
		}

		/* Mark all sections sh_addr with their address in the
		   temporary image. */
		shdr->sh_addr = (size_t)mod->hdr + shdr->sh_offset;
	
#ifndef MODULE_UNLOAD
		if (strstarts(mod->secstrings + shdr->sh_name, ".exit"))
			shdr->sh_flags &= ~(unsigned long)SHF_ALLOC;
#endif
	}

	mod->index.vers = find_sec(mod, "__versions");
	mod->sechdrs[mod->index.vers].sh_flags &= ~(unsigned long)SHF_ALLOC;
	mod->index.info = find_sec(mod, ".modinfo");
	mod->sechdrs[mod->index.info].sh_flags &= ~(unsigned long)SHF_ALLOC;

	for (i = 1; i < mod->hdr->e_shnum; i++)
	{
		if (mod->sechdrs[i].sh_type == SHT_SYMTAB)
		{
			mod->index.sym = i;
			mod->index.str = mod->sechdrs[i].sh_link;
			mod->strtab = (char *)mod->hdr + mod->sechdrs[mod->index.str].sh_offset;
			break;
		}
	}	
}

static void init_elf_header(void *usermod, struct mod_info *mod)
{
	if (mod->len <= 0)
	{
		fprintf(stderr, "module size is lesser than 0.\n");
		exit(1);
	}

	mod->hdr = (Elf64_Ehdr *)malloc(mod->len);
	if (!mod->hdr)
	{
		fprintf(stderr, "malloc: Unable to allocate memory.\n");
		exit(1);
	}

	memcpy(mod->hdr, usermod, mod->len);
	mod->sechdrs = (void *)mod->hdr + mod->hdr->e_shoff;
	mod->secstrings = (void *)mod->hdr + mod->sechdrs[mod->hdr->e_shstrndx].sh_offset;
}

int simulate_load_module(void *usermod, unsigned long len, struct mod_info *mod)
{
	int ret = 0;

	struct mod_secinfo secinfo[MAX_VERIFY_SECTION_SIZE];

	mod->len = len;

	init_elf_header(usermod, mod);
	init_section_headers(mod);
	
	ret = module_frob_arm64_section(mod);
	if (ret < 0)
			return -1;

	layout_symtab(mod);
	divide_section(mod);
	
	init_info(secinfo, mod, checklists, verify_entsize);
		
	/* First signature w/o relocation */
	size_t alloc_sz = page_align(mod->verify_size);
	assert(mod->verify_size < alloc_sz);
	mod->buff = (u64)malloc(alloc_sz);

	setup_buff(mod->buff, secinfo, verify_entsize);
	sign_module(mod->buff, mod->verify_size);
	/* End of signature */

	return ret;
}
