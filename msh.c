#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*
  Function Declarations for builtin shell commands:
 */
int msh_cd(char **args);
int msh_help(char **args);
int msh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

//an array of function pointers 
//that take array of strings and return an int
int (*builtin_func[]) (char **) = {
  &msh_cd,
  &msh_help,
  &msh_exit
};


int msh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int msh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "msh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("msh");
    }
  }
  return 1;
}

int msh_help(char **args)
{
  int i;
  printf("Invinciblenerd's MSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < msh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int msh_exit(char **args)
{
  return 0;
}


#define MSH_RL_BUFSIZE 1024

char *msh_read_line(void){
    int bufsize=MSH_RL_BUFSIZE;
    int position=0;
    char *buffer=malloc(sizeof(char)*bufsize);
    int c;

    if(!buffer){
        fprintf(stderr, "msh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        //read a char
        c=getchar();

        //if we hit EOF, replace it with a null char and return
        //notice: EOF is an integer, not a character! 
        //sizeof char: 1; sizeof int: 4
        if(c==EOF || c=='\n'){
            buffer[position]='\0';
            return buffer;
        }else{
            buffer[position]=c;
        }
        position++;

        //if we have exceeded the buffer, reallocate
        if(position>=bufsize){
            bufsize+=MSH_RL_BUFSIZE;

            //virtually append new space contiguously
            //physically: we don't know, and no need to care
            buffer=realloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr, "msh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


#define MSH_TOK_BUFSIZE 64
#define MSH_TOK_DELIM " \t\r\n\a"
char **msh_split_line(char *line){
    int bufsize=MSH_TOK_BUFSIZE, position=0;

    //allocate number of pointers
    char **tokens=malloc(bufsize*sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "msh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    //returns a pointer to the first token
    token=strtok(line,MSH_TOK_DELIM);
    while(token!=NULL){
        tokens[position++]=token;

        if(position>=bufsize){
            bufsize+=MSH_TOK_BUFSIZE;
            tokens=realloc(tokens,bufsize);
            if(!tokens){
                fprintf(stderr, "msh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        //The first call to strtok should include the string to be tokenized. 
        //In subsequent calls, this argument should be set to NULL, 
        //indicating that it should continue tokenizing the same string.
        token=strtok(NULL,MSH_TOK_DELIM);
    }
    tokens[position]=NULL;
    return tokens;
}


int msh_launch(char **args){
    pid_t pid, wpid;
    int status;

    pid=fork();
    if(pid==0){
        //child process
        //how to start a process:
        //1. PC boot & kernel init
        //2. fork(copy parent) and exec(replace with another program)
        if(execvp(args[0],args)==-1){
            perror("msh");
        }
        //after exec, should never return
        exit(EXIT_FAILURE);
    }else if(pid<0){
        //error forking
        perror("msh");
    }else{
        //parent process
        do{
            wpid=waitpid(pid,&status,WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int msh_execute(char **args){
    int i;

    if(args[0]==NULL){
        //an empty command was entered
        return 1;
    }

    //if match a builtin, run within shell
    for(i=0; i<msh_num_builtins(); i++){
        if(strcmp(args[0],builtin_str[i])==0){
            return (*builtin_func[i])(args);
        }
    }

    //else, launch a new process
    return msh_launch(args);
}


void msh_loop(void){
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line=msh_read_line();
        args=msh_split_line(line);

        //determine when to exit
        status=msh_execute(args);

        free(line);
        free(args);
    }while(status);
}

int main(int argc, char **argv){
    //load config, if any

    // printf("Hello, World!\n");
    // return 0;
    //run command loop
    msh_loop();
    
    //return EXIT_SUCCESS;
    //perform shutdown/cleanup
}