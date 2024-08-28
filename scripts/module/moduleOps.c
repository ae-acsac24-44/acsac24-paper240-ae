#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sign_mod.h"

void layout_symtab(struct mod_info *mod)
{
	Elf64_Shdr *symsect = mod->sechdrs + mod->index.sym;
	Elf64_Shdr *strsect = mod->sechdrs + mod->index.str;

	symsect->sh_flags |= SHF_ALLOC;
	strsect->sh_flags |= SHF_ALLOC;
}


void init_info(struct mod_secinfo secinfo[], struct mod_info *mod, u32 checklists[], u32 entsize)
{
	for (int i = 0; i < entsize; i++)
	{
		secinfo[i] = init_sec(checklists[i], mod);
		mod->verify_size += sizeof(struct verinfo);
		if (secinfo[i].vinfo.type != SHT_NOBITS){
			mod->verify_size += secinfo[i].vinfo.size;
		}
	}

	return;
}

void setup_buff(u64 buff, struct mod_secinfo *secinfo, u32 entsize)
{
	int i;
	u64 base, size;
	u32 vinfo_size = sizeof(struct verinfo);

	for (i = 0; i < entsize; i++)
	{
		memcpy((void *)buff, &secinfo[i].vinfo, vinfo_size);
		buff += vinfo_size;

		if (secinfo[i].vinfo.type == SHT_NOBITS)
			continue;

		base = secinfo[i].base;
		size = secinfo[i].vinfo.size;

		secinfo[i].buff_offset = buff;

		memcpy((void *)buff, (void *)base, size);
		buff += size;
	}
}

void sign_module(u64 buff, size_t size)
{
	uint8_t public_key[32];
	uint8_t signature[64];
	uint8_t private_key[64];
	uint8_t signature_hex[130];

	unsigned char *public_key_hex =
		"07f8993d1a43239a925ad3d02124b931b7f1d0531122f35d63c85cf79f3f4eca";
	unsigned char *private_key_hex =
		"100df9e44a5516fd053dc9ede29914b05d162c3faebbd1fb1897f8169c77a77bee33356625802cb7512453cd2719fec42100215369e30520979e7498506f010e";

	hex2bin(public_key, public_key_hex, 32);
	hex2bin(private_key, private_key_hex, 64);

	Hacl_Ed25519_sign(signature, private_key, size, (uint8_t *)buff);

	bin2hex(signature_hex, signature, 64);
	signature_hex[128] = '\0';
	fprintf(stderr, "signature: %s\n", signature_hex);
	return;
}
