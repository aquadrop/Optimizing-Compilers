/*
 * This file was created automatically from SUIF
 *   on Sun Apr  7 17:02:56 2013.
 *
 * Created by:
 * s2c 5.138 (plus local changes) compiled Thu Oct 9 05:14:25 EDT 2008 by kingyen on ug153
 *     Based on SUIF distribution 1.1.2
 *     Linked with:
 *   libsuif 5.228 (plus local changes) compiled Thu Oct 9 05:12:27 EDT 2008 by kingyen on ug153
 *   libuseful 1.243 (plus local changes) compiled Thu Oct 9 05:12:38 EDT 2008 by kingyen on ug153
 */


struct _IO_marker;

struct __tmp_struct1 { int __val[2]; };
struct _IO_FILE { int _flags;
                  char *_IO_read_ptr;
                  char *_IO_read_end;
                  char *_IO_read_base;
                  char *_IO_write_base;
                  char *_IO_write_ptr;
                  char *_IO_write_end;
                  char *_IO_buf_base;
                  char *_IO_buf_end;
                  char *_IO_save_base;
                  char *_IO_backup_base;
                  char *_IO_save_end;
                  struct _IO_marker *_markers;
                  struct _IO_FILE *_chain;
                  int _fileno;
                  int _flags2;
                  int _old_offset;
                  unsigned short _cur_column;
                  char _vtable_offset;
                  char _shortbuf[1];
                  void *_lock;
                  struct __tmp_struct1 _offset;
                  void *__pad1;
                  void *__pad2;
                  void *__pad3;
                  void *__pad4;
                  unsigned int __pad5;
                  int _mode;
                  char _unused2[40]; };
struct _IO_marker { struct _IO_marker *_next;
                    struct _IO_FILE *_sbuf;
                    int _pos; };

extern int fclose(struct _IO_FILE *);
extern struct _IO_FILE *fopen(const char *, const char *);
extern int fprintf(struct _IO_FILE *, const char *, ...);
extern void matrix_multiply(int (*)[100], int (*)[100], int (*)[100], int);
extern void main();
extern int exit();

extern void matrix_multiply(int (*C)[100], int (*A)[100], int (*B)[100], int n)
  {
    int i;
    int j;
    int k;
    int *suif_tmp1;

    if (!(0 < n))
        goto L12;
    i = 0;
  L13:
    if (!(0 < n))
        goto L9;
    j = 0;
  L10:
    *(int *)((char *)C + i * 400 + j * 4) = 0;
    if (!(0 < n))
        goto L7;
    k = 0;
  L8:
    suif_tmp1 = (int *)((char *)C + i * 400 + j * 4);
    *suif_tmp1 = *suif_tmp1 + *(int *)((char *)A + i * 400 + k * 4) * *(int *)((char *)B + k * 400 + j * 4);
  L5:
    k = k + 1;
    if (!(n <= k))
        goto L8;
  L6:
    goto __done9;
  L7:
    k = 0;
  __done9:
  L3:
    j = j + 1;
    if (!(n <= j))
        goto L10;
  L4:
    goto __done11;
  L9:
    j = 0;
  __done11:
  L1:
    i = i + 1;
    if (!(n <= i))
        goto L13;
  L2:
    goto __done14;
  L12:
    i = 0;
  __done14:
    return;
  }

extern void main()
  {
    int i;
    int j;
    int (A[100])[100];
    int (B[100])[100];
    int (C[100])[100];
    struct _IO_FILE *f;

    i = 0;
  L10:
    j = 0;
  L9:
    *(int *)((char *)A[0] + i * 400 + j * 4) = i * j;
    if (!(j != 0))
        goto L5;
    *(int *)((char *)B[0] + i * 400 + j * 4) = i / j;
    goto __done8;
  L5:
    *(int *)((char *)B[0] + i * 400 + j * 4) = 0;
  __done8:
  L3:
    j = j + 1;
    if (!(100 <= j))
        goto L9;
  L4:
  L1:
    i = i + 1;
    if (!(100 <= i))
        goto L10;
  L2:
    matrix_multiply((int (*)[100])C[0], (int (*)[100])A[0], (int (*)[100])B[0], 100);
    f = fopen("mmult.out", "w");
    i = 0;
  L11:
    fprintf(f, "C[%d][%d] = %d\n", i, i, *(int *)((char *)C[0] + i * 400 + i * 4));
  L6:
    i = i + 1;
    if (!(100 <= i))
        goto L11;
  L7:
    fclose(f);
    exit(0);
    return;
  }
