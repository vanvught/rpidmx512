#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>

static FILE *file_content;
static FILE *file_includes;

static const char supported_extensions[3][8] = {
		"html",
		"js",
		"css"
};

static int is_supported(const char *file_name) {
	int i, l, e;
	for (i = 0; i < sizeof(supported_extensions) / sizeof(supported_extensions[0]); i++) {
		l = strlen(file_name);
		e = strlen(supported_extensions[i]);
		if (l > (e + 2)) {
			if (file_name[l - e - 1] == '.') {
				if (strcmp(&file_name[l - e], supported_extensions[i]) == 0) {
					return 1;
				}
			}
		}
	}

	return 0;
}

static int convert_to_h(const char *file_name) {
	FILE *file_in, *file_out;
	char buffer[32];
	int c, i;
	unsigned offset;

	printf("file_name=%s ", file_name);

	file_in = fopen(file_name, "r");

	if (file_in == NULL) {
		return 0;
	}

	char *file_out_name = malloc(sizeof(file_name) + 3);
	assert(file_out_name != NULL);

	strcpy(file_out_name, file_name);
	strcat(file_out_name, ".h");

	file_out = fopen(file_out_name, "w");

	if (file_out == NULL) {
		free(file_out_name);
		fclose(file_in);
		return 0;
	}

	i = snprintf(buffer, sizeof(buffer) - 1, "#include \"%s\"\n", file_out_name);
	assert(i < sizeof(buffer));
	fwrite(buffer, sizeof(char), i, file_includes);

	fwrite("static constexpr char ", sizeof(char), 22, file_out);

	char *const_name = malloc(sizeof(file_name));
	assert(const_name != NULL);

	strcpy(const_name, file_name);
	char *p = strstr(const_name, ".");
	if (p != NULL) {
		*p = '_';
	}

	printf("const_name=%s\n", const_name);
	fwrite(const_name, sizeof(char), strlen(const_name), file_out);
	fwrite(const_name, sizeof(char), strlen(const_name), file_content);
	fwrite("[] = {\n", sizeof(char), 7, file_out);

	offset = 0;

	while ((c = fgetc (file_in)) != EOF) {
		i = snprintf(buffer, sizeof(buffer) - 1, "0x%02X,%c", c, ++offset % 16 == 0 ? '\n' : ' ');
		assert(i < sizeof(buffer));
		fwrite(buffer, sizeof(char), i, file_out);
	}

	fwrite("0x00\n};\n", sizeof(char), 8, file_out);

	free(file_out_name);
	free(const_name);

	fclose(file_in);
	fclose(file_out);

	return 1;
}

static const char content_header[] =
		"\n"
		"struct FilesContent {\n"
		"\tconst char *pFileName;\n"
		"\tconst char *pContent;\n"
		"};\n\n"
		"static constexpr struct FilesContent HttpContent[] = {\n";

int main(void) {
	DIR *d;
	struct dirent *dir;

	file_includes = fopen("includes.h", "w");

	file_content = fopen("content.h", "w");
	fwrite(content_header, sizeof(char), strlen(content_header), file_content);

	d = opendir(".");

	if (d) {
		while ((dir = readdir(d)) != NULL) {
			int do_handle = is_supported(dir->d_name);
			printf("%s -> %c\n", dir->d_name, do_handle == 0 ? 'N' : 'Y');

			if (do_handle) {
				char *file_name = malloc(strlen(dir->d_name) + 9);
				assert(file_name != NULL);

				int i = snprintf(file_name, strlen(dir->d_name) + 8, "\t{ \"%s\", ", dir->d_name);
				fwrite(file_name, sizeof(char), i, file_content);
				fflush(file_content);
				free(file_name);

				convert_to_h(dir->d_name);

				fwrite(" },\n", sizeof(char), 4, file_content);
			}
		}

		closedir(d);
	}

	fclose(file_includes);

	fwrite("};\n", sizeof(char), 2, file_content);
	fclose(file_content);

	system("cat includes.h > tmp.h");
	system("cat content.h >> tmp.h");
	system("rm content.h");
	system("mv tmp.h content.h");

	return EXIT_SUCCESS;
}
