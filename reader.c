#include "seacc.h"

struct FILE {
  int dummy_data;
};

int errno;
int SEEK_END = 2;
int SEEK_SET = 0;

struct FILE *fopen(char *path, char *mode);
char *strerror(int errno);
int fseek(struct FILE *fp, long offset, int whence);
long ftell(struct FILE *fp);
void *calloc(long nmemb, long size);
long fread(void *buf, long size, long nmemb, struct FILE *stream);
int fclose(struct FILE *stream);

char *read_file(char* path) {
  struct FILE *fp = fopen(path, "r");
  if(fp==0) error("cannot open %s: %s", path, strerror(errno));

  if(fseek(fp, 0, SEEK_END) == -1)
    error("%s: fseek: %s", path, strerror(errno));
  long size = ftell(fp);
  if(fseek(fp, 0, SEEK_SET) == -1)
    error("%s: fseek: %s", path, strerror(errno));

  char *buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  if(size==0 + (buf[size-1] != '\n')) {
    buf[size] = '\n';
    ++size;
  }
  buf[size] = '\0';
  // XXX: fclose(fp) fails probably because of the wrong stack align
  // fclose(fp);
  return buf;
}

