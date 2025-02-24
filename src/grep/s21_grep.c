#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct flags {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
};

int options_parser(int argc, char **argv, int *num, struct flags *flag,
                   char **templates, int *templates_count);
int options_parser_switch(int *num, struct flags *flag, char **templates,
                          int *templates_count);
void grep(int argc, char **argv, struct flags flag, char **templates,
          int templates_count);
void grep_getliner(int *line_len, char **line, size_t *buffer_size, FILE **f,
                   int *line_num, char **result, char **template,
                   int templates_count, char **argv, struct flags flag,
                   int *correct_line_counter, int not_one_file);
int grep_print(char **argv, char **result, struct flags flag, int *line_num,
               int *correct_line_counter, int not_one_file);
char *search_regex(char *line, char **template, int templates_count,
                   struct flags flag);
int is_templates_finded(char *line, char **template, int templates_count,
                        struct flags flag, char **result, regmatch_t *pmatch);
char *search_regex_in_line(char *line, char *template, struct flags flag,
                           regmatch_t *pmatch);
void post_print(char **argv, struct flags flag, FILE *f, char *result,
                int not_one_file, int correct_line_counter);

int main(int argc, char **argv) {
  int num = 0;
  struct flags flag = {0};
  char *template[argc];
  int templates_count = 0;
  if (!options_parser(argc, argv, &num, &flag, template, &templates_count)) {
    grep(argc, argv, flag, template, templates_count);
  }
  return 0;
}

int options_parser(int argc, char **argv, int *num, struct flags *flag,
                   char **templates, int *templates_count) {
  int err_flag = 0;
  while ((*num = getopt(argc, argv, "e:ivclnhs")) != -1) {
    err_flag += options_parser_switch(num, flag, templates, templates_count);
  }
  if (flag->e == 0 && optind < argc - 1) {
    templates[*templates_count] = argv[optind];
    *templates_count += 1;
    optind++;
  } else if (flag->e == 0) {
    if (flag->s != 0) {
      fprintf(stderr, "Error in options_parser function\n");
    }
    err_flag++;
  }
  return err_flag;
}

int options_parser_switch(int *num, struct flags *flag, char **templates,
                          int *templates_count) {
  int err_flag = 0;
  switch (*num) {
    case 'e':
      flag->e = 1;
      templates[*templates_count] = optarg;
      *templates_count += 1;
      break;
    case 'i':
      flag->i = 1;
      break;
    case 'v':
      flag->v = 1;
      break;
    case 'c':
      flag->c = 1;
      break;
    case 'l':
      flag->l = 1;
      break;
    case 'n':
      flag->n = 1;
      break;
    case 'h':
      flag->h = 1;
      break;
    case 's':
      flag->s = 1;
      break;
    default:
      fprintf(stderr, "Usage: ./s21_grep [OPTION]... PATTERNS [FILE]...\n");
      err_flag++;
  }
  return err_flag;
}

void grep(int argc, char **argv, struct flags flag, char **templates,
          int templates_count) {
  int line_len = 0;
  int not_one_file = 0;
  char *result = NULL;
  char *line = NULL;
  size_t buffer_size = 0;
  if (optind < argc - 1 && !flag.h) {
    not_one_file = 1;
  }
  for (; optind < argc; optind++) {
    FILE *f = fopen(argv[optind], "r");
    if (f != NULL) {
      int correct_line_counter = 0;
      int line_num = 0;
      grep_getliner(&line_len, &line, &buffer_size, &f, &line_num, &result,
                    templates, templates_count, argv, flag,
                    &correct_line_counter, not_one_file);
      post_print(argv, flag, f, result, not_one_file, correct_line_counter);
    } else if (!flag.s) {
      fprintf(stderr, "Erorr in grep function\n");
    }
    free(line);
    line = NULL;
  }
}

void post_print(char **argv, struct flags flag, FILE *f, char *result,
                int not_one_file, int correct_line_counter) {
  if (flag.c && not_one_file && !flag.l) {
    printf("%s:", argv[optind]);
  }
  if (flag.c && !flag.l) {
    printf("%d\n", correct_line_counter);
  }
  if (!flag.l && !flag.c && result && result[strlen(result) - 1] != '\n') {
    printf("\n");
  }
  fclose(f);
}

void grep_getliner(int *line_len, char **line, size_t *buffer_size, FILE **f,
                   int *line_num, char **result, char **template,
                   int templates_count, char **argv, struct flags flag,
                   int *correct_line_counter, int not_one_file) {
  int is_find = 1;
  while ((*line_len = getline(line, buffer_size, *f)) != EOF && is_find) {
    *line_num += 1;
    *result = search_regex(*line, template, templates_count, flag);
    is_find = grep_print(argv, result, flag, line_num, correct_line_counter,
                         not_one_file);
  }
}

int grep_print(char **argv, char **result, struct flags flag, int *line_num,
               int *correct_line_counter, int not_one_file) {
  int find = 1;
  if (*result != NULL) {
    if (flag.l) {
      printf("%s\n", argv[optind]);
      find = 0;
    }
    if (flag.c && find) {
      *correct_line_counter += 1;
    } else if (find) {
      if (not_one_file) {
        printf("%s:", argv[optind]);
      }
      if (flag.n && find) {
        printf("%d:", *line_num);
      }
      printf("%s", *result);
    }
  }
  return find;
}

char *search_regex(char *line, char **template, int templates_count,
                   struct flags flag) {
  char *result = NULL;
  regmatch_t pmatch[1];
  is_templates_finded(line, template, templates_count, flag, &result, pmatch);
  return result;
}

int is_templates_finded(char *line, char **template, int templates_count,
                        struct flags flag, char **result, regmatch_t *pmatch) {
  int is_find = 0;
  for (int i = 0; i < templates_count; i++) {
    *result = search_regex_in_line(line, template[i], flag, pmatch);
    if (*result != NULL && !flag.v) {
      is_find = 1;
      break;
    } else if (*result != NULL && flag.v) {
      *result = NULL;
      is_find = 1;
      break;
    } else if (*result == NULL && flag.v) {
      *result = line;
    }
  }
  return is_find;
}

char *search_regex_in_line(char *line, char *template, struct flags flag,
                           regmatch_t *pmatch) {
  char *result = NULL;
  regex_t regex_expression;
  int err_flag = 0;
  char message[128] = "Error in function search_regex_in_line";
  if (flag.i) {
    err_flag = regcomp(&regex_expression, template, REG_EXTENDED | REG_ICASE);
  } else {
    err_flag = regcomp(&regex_expression, template, REG_EXTENDED);
  }
  if (err_flag != 0) {
    regerror(err_flag, &regex_expression, message, 128);
    printf("%s\n", message);
  } else {
    if ((err_flag = regexec(&regex_expression, line, 1, pmatch, 0)) == 0) {
      result = line;
    } else if (err_flag != REG_NOMATCH) {
      regerror(err_flag, &regex_expression, message, 128);
      printf("%s\n", message);
    }
    regfree(&regex_expression);
    if (strlen(template) == 1 && template[0] == '.' && strlen(line) == 1 &&
        line[0] == '\n') {
      result = NULL;
    }
  }
  return result;
}