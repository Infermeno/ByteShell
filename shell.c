
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#define ByteShell_TOK_BUFSIZE 64
#define ByteShell_TOK_DELIM " \t\r\n\a"

int ByteShell_cd(char **args);
int ByteShell_help(char **args);
int ByteShell_exit(char **args);
int sh_bg(char **args);
int ByteShell_history(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "bg",
    "history"
    };

int (*builtin_func[])(char **) = {
    &ByteShell_cd,
    &ByteShell_help,
    &ByteShell_exit,
    &sh_bg,
    &ByteShell_history
    };

int ByteShell_num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}

int ByteShell_cd(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "ByteShell: expected argument to \"cd\"\n");
  }
  else
  {
    if (chdir(args[1]) != 0)
    {
      perror("ByteShell");
    }
  }
  return 1;
}

int ByteShell_help(char **args)
{
  int i;
  printf("My own shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < ByteShell_num_builtins(); i++)
  {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

struct Node {
    char *str;
    struct Node* next;
};
struct Node* head = NULL;
struct Node* cur = NULL;

char* strAppend(char* str1, char* str2)
{
	char* str3 = (char*)malloc(sizeof(char*)*(strlen(str1)+strlen(str2)));
  strcpy(str3, str1);
  strcat(str3, str2);
	return str3;
}
void add_to_hist(char **args){
  if(head==NULL){
    head = (struct Node *)malloc(sizeof(struct Node));
      head->str = (char *)malloc(0x1000);
  char *str1 = " ";
  if (args[1] == NULL) 
     strcpy(head->str,strAppend(args[0],str1));
  else
{  
  strcpy(head->str,strAppend(args[0],str1));
  strcpy(head->str, strAppend(head->str, args[1]));
  }
  head->next = NULL;
  cur = head;
  }
else{
    struct Node *ptr = (struct Node *)malloc(sizeof(struct Node));
    cur->next = ptr;
    ptr->str = (char *)malloc(0x1000);
      char *str1 = " ";
  if (args[1] == NULL) 
     strcpy(ptr->str,strAppend(args[0],str1));
  else
{  
  strcpy(ptr->str,strAppend(args[0],str1));
  strcpy(ptr->str, strAppend(ptr->str, args[1]));
  }
    ptr->next = NULL;
    cur = ptr;
}
}

int ByteShell_history(char **args){  
   struct Node* ptr = head;
    int i = 1;
    while (ptr != NULL)
    {
    printf(" %d %s\n",i++,ptr->str);
    ptr = ptr->next;
    }
  return 1; 
  }

int ByteShell_exit(char **args)
{
  return 0;
}

int sh_bg(char **args)
{
  ++args;
  char *firstCmd = args[0]; // echo
  int childpid = fork();
  if (childpid >= 0)
  {
    if (childpid == 0)
    {
      if (execvp(firstCmd, args) < 0)
      {
        perror("Error on execvp\n");
        exit(0);
      }
    }
  }
  else
  {
    perror("fork() error");
  }
  return 1;
}


int ByteShell_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  {
  
    if (execvp(args[0], args) == -1)
    {
      perror("ByteShell");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    perror("ByteShell");
  }
  else
  {
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int ByteShell_execute(char **args)
{

  int i;

  if (args[0] == NULL)
  {
    return 1;
  }
  for (i = 0; i < ByteShell_num_builtins(); i++)
  {
    if (strcmp(args[0], builtin_str[i]) == 0)
    {
      return (*builtin_func[i])(args);
    }
  }

  return ByteShell_launch(args);
}
char *ByteShell_read_line(void)
{
#define ByteShell_RL_BUFSIZE 1024
  int bufsize = ByteShell_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer)
  {
    fprintf(stderr, "ByteShell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    c = getchar();

    if (c == EOF)
    {
      exit(EXIT_SUCCESS);
    }
    else if (c == '\n')
    {
      buffer[position] = '\0';
      return buffer;
    }
    else
    {
      buffer[position] = c;
    }
    position++;
    if (position >= bufsize)
    {
      bufsize += ByteShell_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer)
      {
        fprintf(stderr, "ByteShell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **ByteShell_split_line(char *line)
{
  int bufsize = ByteShell_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token, **tokens_backup;

  if (!tokens)
  {
    fprintf(stderr, "ByteShell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, ByteShell_TOK_DELIM);
  while (token != NULL)
  {
    tokens[position] = token;
    position++;

    if (position >= bufsize)
    {
      bufsize += ByteShell_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens)
      {
        free(tokens_backup);
        fprintf(stderr, "ByteShell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, ByteShell_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
void ByteShell_loop(void)
{
  char *line;
  char **args;
  int status;

  do
  {
    printf("> ");
    line = ByteShell_read_line();
    args = ByteShell_split_line(line);
          add_to_hist(args);
    status = ByteShell_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  ByteShell_loop();
  return EXIT_SUCCESS;
}
