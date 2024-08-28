#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sign_mod.h"

unsigned int find_sec(const struct mod_info *mod, const char *name)
{
	unsigned int i;

	for (i = 1; i < mod->hdr->e_shnum; i++)
	{
		Elf64_Shdr *shdr = &mod->sechdrs[i];
		if (strcmp(mod->secstrings + shdr->sh_name, name) == 0)
			return i;
	}
	return 0;
}

struct mod_secinfo init_sec(u32 idx, struct mod_info *mod)
{

	struct mod_secinfo secinfo = {};

	memset(&secinfo, 0, sizeof(secinfo));

	Elf64_Shdr *shdr = &mod->sechdrs[idx];
	struct verinfo *vinfo = &secinfo.vinfo;

	secinfo.base = shdr->sh_addr;
	
	vinfo->size = shdr->sh_size;
	vinfo->flags = shdr->sh_flags;
	vinfo->align_size = shdr->sh_addralign? :1;
	vinfo->info = shdr->sh_info;
	vinfo->type = shdr->sh_type;
	vinfo->link = shdr->sh_link;

	return secinfo;
}
