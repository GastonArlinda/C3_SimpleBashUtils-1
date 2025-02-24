#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_valid_flag(char flag);
void append_flags(char *flags, char flag);
void append_flag(char *flags, char flag);
void flag_parser(char *flags, int argc, char **argv);
void print(char *name, char *flags, int *index, int *is_eof_enter, int *cnt);
void print_symb(int c, int *prev, char *flags, int *index, int *eline_printed,
                int *is_eof_enter);

int main(int argc, char **argv) {
  char flags[7] = "\0";
  int index = 0, is_eof_enter = 1, cnt = 1;
  flag_parser(flags, argc, argv);
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      print(argv[i], flags, &index, &is_eof_enter, &cnt);
      cnt += 1;
    }
  }
  return 0;
}

void flag_parser(char *flags, int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-' || strcmp(argv[i], "--") == 0 ||
        strcmp(argv[i], "-") == 0) {
    } else {
      if (!strcmp(argv[i], "--number-nonblank")) argv[i] = "-b";
      if (!strcmp(argv[i], "--number")) argv[i] = "-n";
      if (!strcmp(argv[i], "--squeeze-blank")) argv[i] = "-s";
      for (size_t j = 1; j < strlen(argv[i]); j++) {
        if (is_valid_flag(argv[i][j])) {
          exit(1);
        }
        append_flags(flags, argv[i][j]);
      }
    }
  }
}

int is_valid_flag(char flag) {
  int err_flag = 0;
  if (strchr("bEenstTv", flag) == NULL) err_flag = 1;
  return err_flag;
}

struct s_avi_flags {
  char flag;
  char *equivalent_flags;
};

void append_flags(char *flags, char flag) {
  struct s_avi_flags avi_flags[8] = {{'b', "b"},  {'E', "E"}, {'e', "Ev"},
                                     {'n', "n"},  {'s', "s"}, {'T', "T"},
                                     {'t', "Tv"}, {'v', "v"}};
  for (int i = 0; i < 8; i++) {
    if (flag == avi_flags[i].flag) {
      for (size_t j = 0; j < strlen(avi_flags[i].equivalent_flags); j++) {
        append_flag(flags, avi_flags[i].equivalent_flags[j]);
      }
      break;
    }
  }
}

void append_flag(char *flags, char flag) {
  if (strchr(flags, flag) == NULL) {
    char temp[2] = "-";
    temp[0] = flag;
    strcat(flags, temp);
  }
}

void print(char *name, char *flags, int *index, int *is_eof_enter, int *cnt) {
  FILE *f = fopen(name, "r");
  if (f != NULL) {
    int eline_printed = 0;
    int c = fgetc(f), prev = '\n';
    if (*cnt > 1) prev = '\0';
    while (c != EOF) {
      print_symb(c, &prev, flags, index, &eline_printed, is_eof_enter);
      c = fgetc(f);
      if (c == EOF && prev != '\n') *is_eof_enter = 0;
    }
    fclose(f);
  } else {
    exit(1);
  }
}

void print_symb(int c, int *prev, char *flags, int *index, int *eline_printed,
                int *is_eof_enter) {
  if (!(strchr(flags, 's') != NULL && *prev == '\n' && c == '\n' &&
        *eline_printed)) {
    if (*prev == '\n' && c == '\n')
      *eline_printed = 1;
    else
      *eline_printed = 0;
    if (((strchr(flags, 'n') != NULL && strchr(flags, 'b') == NULL) ||
         (strchr(flags, 'b') != NULL && c != '\n')) &&
        *prev == '\n') {
      if (*is_eof_enter == 1) {
        *index += 1;
        printf("%6d\t", *index);
      }
    }
    if (c == '\n') *is_eof_enter = 1;
    if (strchr(flags, 'E') != NULL && c == '\n') printf("$");
    if (strchr(flags, 'T') != NULL && c == '\t') {
      printf("^");
      c = '\t' + 64;
    }
    if (strchr(flags, 'v') != NULL && c != '\n' && c != '\t') {
      if (c >= 0 && c <= 31) {
        printf("^");
        c = c + 64;
      } else if (c == 127) {
        printf("^");
        c = c - 64;
      } else if (c > 127 && c < 160) {
        c -= 64;
        printf("M-^");
      } else if (c > 160) {
        c += 128;
        printf("M-");
      }
    }
    fputc(c, stdout);
  }
  *prev = c;
}