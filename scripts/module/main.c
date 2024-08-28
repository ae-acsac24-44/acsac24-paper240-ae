#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <elf.h>
#include <sys/types.h>

#include "sign_mod.h"

/* Copied from 
 * https://github.com/vadmium/module-init-tools/blob/master/insmod.c
 */
static void *grab_file(const char *filename, unsigned long *size)
{
	unsigned int max = 16384;
	int ret, fd, err_save;
	void *buffer;

	fd = open(filename, O_RDONLY, 0);

	if (fd < 0)
		return NULL;

	buffer = malloc(max);
	if (!buffer)
		goto out_error;

	*size = 0;
	while ((ret = read(fd, buffer + *size, max - *size)) > 0)
	{
		*size += ret;
		if (*size == max)
		{
			void *p;

			p = realloc(buffer, max *= 2);
			if (!p)
				goto out_error;
			buffer = p;
		}
	}
	if (ret < 0)
		goto out_error;

	close(fd);
	return buffer;

out_error:
	err_save = errno;
	free(buffer);
	close(fd);
	errno = err_save;
	return NULL;
}

int main(int argc, char **argv)
{
	char *filename;
	void *usermod;
	unsigned long len;
	struct mod_info mod = {};

	if (argc != 2)
	{
		fprintf(stderr, "Usage: ./sign_mod filename\n");
		exit(1);
	}

	filename = argv[1];

	usermod = grab_file(filename, &len);
	if (!usermod)
	{
		fprintf(stderr, "Read %s failed: %s\n", filename, strerror(errno));
	}

	simulate_load_module(usermod, len, &mod);

	free(usermod);

	return 0;
}
