#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdint.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <strings.h>
#include "setter.h"

#define PORT 2023

struct stat sts;
extern int errno;

typedef struct thData
{
	int idThread;  //id-ul thread-ului tinut in evidenta de acest program
	int cl;       //descriptorul intors de accept
}thData;

//thData* clients[200];
//int nrClients = 0;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */

void raspunde(void *);

int main()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;

    /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));

   /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;	
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons (PORT);

  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
  {
    perror ("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
  {
    perror ("[server]Eroare la listen().\n");
    return errno;
  }
  //fiecare client e prelucrat concurent
  while(1)
  {
    int client;
    thData* td;
    int len = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);
    
    //face conexiunea propriu zisa: client socket la server socket
    if ((client = accept(sd, (struct sockaddr *) &from, &len)) < 0)
    {
      perror ("[server]Eroare la accept().\n");
      continue;
    }
	

    td = (struct thData*)malloc(sizeof(struct thData));	
    td->idThread = i++;
    td->cl = client;
   // td->idUser = 0;
    //clients[i] = td;
   // nrClients = i;
    pthread_create(&th[i], NULL, &treat, td); //

  }


};

static void* treat(void* arg)
{
  	struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
};

//---------------------------------------------- checkUser (NEW) ----------------------------------------------------------------
int checkUser(const char username[256])
{
  sqlite3 *db;
  char *err_msg = 0;
  sqlite3_stmt *res;
  //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
  if (rc != SQLITE_OK) 
  {
      
      fprintf(stderr, "Cannot open database: %s\n", 
              sqlite3_errmsg(db));
      sqlite3_close(db);
      
      return -2;
  }
  printf("suntem in fct checkUsers();\n");
  printf("username din check: %s\n", username);
  char* sql;
  sql = (char*) malloc(2048);
  
  strcpy(sql, "SELECT Name from Users WHERE Name = ?");
 
 // printf("[CREDENTIALS] Comanda before prepare_v2: %s\n", sql);
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  
  if (rc == SQLITE_OK) 
  {     
    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);
  } 
  else 
  {     
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
  }
  int step = sqlite3_step(res);
   
  if (step == SQLITE_ROW) 
  { 
    printf("[checkUser] Numele EXISTA deja!\n");
    sqlite3_finalize(res);
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_close(db);
    free(sql);
    return 1;
    
  }
  else if(step == SQLITE_DONE)
  {
    printf("[checkUser] Numele NU exista deja!\n");
    sqlite3_finalize(res);
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_close(db);
    free(sql);
    return 0;
  }
  else
  {
    fprintf(stderr, "Error executing SELECT statement: %s\n", sqlite3_errmsg(db));
    
    sqlite3_finalize(res);
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_close(db);
    free(sql);
    return -1;
  }


  //sqlite3_finalize(res);
  //sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  //sqlite3_close(db);
  //free(sql);
}


int checkCredentials(const char username[256], const char password[256])
{
  sqlite3 *db;
  char *err_msg = 0;
  sqlite3_stmt *res;
  //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
  if (rc != SQLITE_OK) 
  {
      
      fprintf(stderr, "Cannot open database: %s\n", 
              sqlite3_errmsg(db));
      sqlite3_close(db);
      
      return -2;
  }
  printf("suntem in fct checkUsers();\n");
  printf("username din check: %s\n", username);
  char* sql;
  sql = (char*) malloc(2048);
  
  strcpy(sql, "SELECT Name from Users WHERE Name = ");
  strcat(sql, "'");
  strcat(sql, username);
  strcat(sql, "'");
  strcat(sql, " and Password = ?");
  sql[strlen(sql)] = '\0';
  printf("[CREDENTIALS] Comanda before prepare_v2: %s\n", sql);
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  
  if (rc == SQLITE_OK) 
  {     
    sqlite3_bind_text(res, 1, password, -1, SQLITE_STATIC);
  } 
  else 
  {     
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
  }
  int step = sqlite3_step(res);
   
  if (step == SQLITE_ROW) 
  { 
    printf("[checkUser] Numele EXISTA deja!\n");
    sqlite3_finalize(res);
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_close(db);
    free(sql);
    return 1;
    
  }
  else if(step == SQLITE_DONE)
  {
    printf("[checkUser] Numele NU exista deja!\n");
    sqlite3_finalize(res);
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_close(db);
    free(sql);
    return 0;
  }
  else
  {
    fprintf(stderr, "Error executing SELECT statement: %s\n", sqlite3_errmsg(db));
    
    sqlite3_finalize(res);
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_close(db);
    free(sql);
    return -1;
  }


  //sqlite3_finalize(res);
  //sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  //sqlite3_close(db);
  //free(sql);
}

/*---------------------------------------------- checkPassword(OLD) ------------------------------------------------------------
int checkPass(const char password[256])
{
  sqlite3 *db;
  char *err_msg = 0;
  sqlite3_stmt *res;
  //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
  if (rc != SQLITE_OK) 
  {
      
      fprintf(stderr, "Cannot open database: %s\n", 
              sqlite3_errmsg(db));
      sqlite3_close(db);
      
      return -2;
  }
  //printf("suntem in fct checkPass();\n");
  //printf("password din check: %s\n", password);
  char* sql;
  sql = (char*) malloc(2048);
  
  strcpy(sql, "SELECT Name from Users WHERE Password = ?");
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

  if (rc == SQLITE_OK) 
  {     
    sqlite3_bind_text(res, 1, password, -1, SQLITE_STATIC);
  } 
  else 
  {     
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
  }
  int step = sqlite3_step(res);
  
  if (step == SQLITE_ROW) 
  { 
    printf("[checkPass] Parola EXISTA!\n");
    return 1;
    
  }
  else
  {
    printf("[checkPass] Numele NU exista!\n");
    return 0;
  }


  sqlite3_finalize(res);
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  sqlite3_close(db);
  free(sql);
}
*/

/*
void setLog(char ok[10], char username[256])
{
  sqlite3 *db;
  char *zErrMsg = 0;
  char *err_msg = 0;
  //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);
  //printf("[setTd]Am deschis din nou db-ul!\n");
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
  if (rc != SQLITE_OK) 
  {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db); 
    return 1;
  }

  char sql[2000] = "";
  //update users set Login = 1 where name = 'andi';
  // int name_len = strlen(username);
  //int mes_len = sizeof(int);
  //int insert_len = strlen("UPDATE Users SET Login='' WHERE name='';");
  //int insert_len2 = strlen("UPDATE Users SET Login=''");


  strcat(sql, "update users set login='");
  //char login[10] = "";
  //itoa(ok, login, 10);
  strcat(sql, ok);
  strcat(sql, "' where name='");
  strcat(sql, username);
  strcat(sql, "';");
  //sql[strlen(sql)] = '\0';
  //printf("[setLog]Am dat snprintf()!\n");
  printf("[setLog]Comm de setare(): %s\n", sql);
  //fflush(stdout);
  rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
  
  if (rc != SQLITE_OK ) 
  {
    fprintf(stderr, "2Log) SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);        
    sqlite3_close(db);
    return 1;
  } 
  //printf("[setLog]Did it work? %d", ok);
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  //free(sql);
  sqlite3_close(db);  
  //printf("[setLog]Finished\n");
  //fflush(stdout);
}
*/


int getTh(char username[256])
{
  sqlite3 *db;
  char *err_msg = 0;
  sqlite3_stmt *res;
  username[strlen(username)] = '\0';
  
  //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
  if (rc != SQLITE_OK) 
  {
      
      fprintf(stderr, "Cannot open database: %s\n", 
              sqlite3_errmsg(db));
      sqlite3_close(db);
      
      return -2;
  }
  //  printf("[getTh()]suntem aici!\n");
  printf("[getTh()] Username: %s\n", username);
  //sqlite3_busy_handler(db, busy_handler, NULL); //scapam de err "Database is locked"!
  char* sql;
  sql = (char*) malloc(2048);
  
  strcpy(sql, "SELECT td from Users WHERE Name = ?");
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

  if (rc == SQLITE_OK) 
  {     
    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);
  } 
  else 
  {     
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
  }
  int step = sqlite3_step(res);
  
  if (step == SQLITE_ROW) 
  { 
    printf("[getTh] ALL GUD!\n");
    //return 1;
  }
  else
  {
    fprintf(stderr, "[getTh]Failed to execute in Th...\n");
    return 0;
  }
  int value = sqlite3_column_int(res, 0);

  sqlite3_finalize(res);
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  sqlite3_close(db);
  free(sql);

  return value;
}

int getLog(char username[256])
{
  sqlite3 *db;
  char *err_msg = 0;
  sqlite3_stmt *res;
  //prelucrare username primit:
  username[strlen(username)] = '\0';
  
  
 // sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
  if (rc != SQLITE_OK) 
  {
      fprintf(stderr, "Cannot open database: %s\n", 
              sqlite3_errmsg(db));
      sqlite3_close(db);
      
      return -2;
  }
  //printf("[getLog()]suntem aici!\n");
  printf("[getLog()] Username: %s\n", username);
  
  fflush(stdout);
  printf("\n");
  //sqlite3_busy_handler(db, busy_handler, NULL); //scapam de err "Database is locked"!
  char* sql;
  sql = (char*) malloc(2048);
  
  strcpy(sql, "SELECT Login from Users WHERE Name = ?");
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

  if (rc == SQLITE_OK) 
  {     
    sqlite3_bind_text(res, 1, username, -1, SQLITE_STATIC);
  } 
  else 
  {     
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
  }
  int step = sqlite3_step(res);
  
  if (step == SQLITE_ROW) 
  { 
    printf("[getLog] ALL GUD!\n");
    //return 1;
  }
  else
  {
    //printf("[checkPass] Numele NU exista!\n");
    fprintf(stderr, "[getLog]Failed to execute in getlog!...\n");
    return 0;
  }
  int value = sqlite3_column_int(res, 0);

  sqlite3_finalize(res);
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  sqlite3_close(db);
  free(sql);

  return value;
}

//-------------------------------------------------- RASPUNDE -------------------------------------------------------------
void raspunde(void* arg)
{
  int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
  char cmdReceived[2000];
  int exista = 1;
  int logged = 0;
  char username[256];
  while(1)
  {
    bzero(cmdReceived, sizeof(cmdReceived));
    if (read (tdL.cl, cmdReceived, sizeof(cmdReceived)) <= 0)
    {
      printf("[Thread %d]\n",tdL.idThread);
      perror ("Eroare la read() de la client.\n"); 
    }

    //-->daca comanda primita nu e una tip register/login cerem mai intai logarea

    printf ("[Thread %d]Mesajul a fost receptionat...%s\n", tdL.idThread, cmdReceived);
    cmdReceived[strlen(cmdReceived)] = '\0';
    fflush(stdout);
    
    while(strcmp(cmdReceived, "|register") == 0)
    {
      username[256] = '\0';
      char password[256] = "";

      if(read(tdL.cl, username, sizeof(username)) <= 0)
      {
          perror("[server]Eroare la read la username\n");
      }
      printf("Username receptionat: %s\n", username);
      fflush(stdout);
      username[strlen(username)] = '\0';
      char copie[256] = "";
      strcpy(copie, username);
      if(checkUser(copie) == 0) //daca numele NU exist deja: 
      {
        //printf("checkUser: %d\n", checkUser(username));
        //trimitem mesaj catre client ca user-ul e bun si poate introduce parola
        exista = 0;
        fflush(stdout);
        if(write(tdL.cl, &exista, sizeof(int)) <= 0)
        {
          perror("[server]Eroare la trimis catre client ca poate introduce parola\n");
        }
        if(read(tdL.cl, password, sizeof(password)) <= 0)
        {
            perror("[server]Eroare la read la password\n");
        }
        
        printf("[server]Am ajuns sa primim parola: %s\n", password);
        fflush(stdout);
        //exista = 0;
        sqlite3 *db;
        char *err_msg = 0;
        //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
        int rc = sqlite3_open("messenger.db", &db);
        // printf("Am deschis DB pt register!\n");
        rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) 
        {
          fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
          sqlite3_close(db); 
          return 1;
        }

        //sqlite3_busy_handler(db, busy_handler, NULL); //scapam de err "Database is locked"!
        //printf("I began transaction!\n");
        char* sql;
        sql = (char*) malloc(2048);
        
        int name_len = strlen(username);
        int pass_len = strlen(password);
        int insert_len = strlen("INSERT INTO Users VALUES ('', '', '', '');");
        int Log = 1;
        int threadID = tdL.cl;
        snprintf(sql, name_len + pass_len + insert_len + 3, "INSERT INTO Users VALUES ('%s', '%s', '%d', '%d');", username, password, Log, threadID);
        sql[strlen(sql)] = '\0';
        printf("Am dat snprintf() la INREGISTRARE: %s\n", sql);
        fflush(stdout);
        //printf("Ultimul caracter pt register: %c\n", sql[strlen(sql) - 1]);
        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
        if (rc != SQLITE_OK ) 
        {
          fprintf(stderr, "2) SQL error: %s\n", err_msg);
          sqlite3_free(err_msg);        
          sqlite3_close(db);
          return 1;
        } 
        //for(int i = 0; i < strlen(sql); i++)
          //printf("%c", sql[i]);
        //printf("\n");
        free(sql);
        sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &err_msg);
        sqlite3_close(db);
        //printf("Te-ai inregistrat cu succes(sper)...\n");
        //printf("[register/DB]Penultimul char: %c\n", sql[strlen(sql) - 2]);
        //fflush(stdout);
        int succes = 1;
        if(write(tdL.cl, &succes, sizeof(succes)) <= 0) 
        {
          perror("[server]Eroare la write register succes\n");
          exit(1);
        }   
        logged = 1;  
        //setLog(1, username);
        break; //break pt while(register)
      }
      
      else
      {
        printf("[server]Utilizatorul exista deja\n");
        //aici trimitem la client sa introduca un nou username
        exista = 1;
        if(write(tdL.cl, &exista, sizeof(int)) <= 0)
        {
          perror("[server]Eroare la trimis user existent\n");
        }
        //break;
      }
      
    }
    
    
    while(strcmp(cmdReceived, "|login") == 0)
    {
      username[0] = '\0';
      char password[256] = "";
      int login = 0;
      printf("SUNTEM IN |login\n");
      if(read(tdL.cl, username, sizeof(username)) <= 0)
      {
          perror("[server|login]Eroare la read la username\n");
      }
      username[strlen(username)] = '\0';
      char usr[256];
      strcpy(usr, username);
      printf("[login] Username receptionat: %s\n", username);
      fflush(stdout);
      if(read(tdL.cl, password, sizeof(password)) <= 0)
            perror("[server|login]Eroare la read la password\n");
      printf("[login] Password receptionat: %s\n", password);
      password[strlen(password)] = '\0';
      if(checkCredentials(usr, password) == 1) //&& checkPass(password) == 1)
      {
        username[strlen(username)] = '\0';
        //printf("[server]Logare corecta!\n");
        //fflush(stdout);
        logged = 1;
        login = 1;
        //set in DB logged = 1/0 && tdL.cl['name']
        //printf("tdl.cl= %d\n", tdL.cl);
        //char username_copy[500] = "";
        //strcpy(username_copy, username);
        //username_copy[strlen(username_copy)] = '\0';
        int thread_no = tdL.cl;
        char log[10] = "1";
        

        setLog(log, username);
        setThread(thread_no, username);
       
        //printf("[login] Am trimis setat thread=%d si login=%d\n", thread_no, 1);
        //fflush(stdout);
        if(write(tdL.cl, &login, sizeof(int)) <= 0)
          perror("[server|login]Eroare la write la login ok\n");
        break;
      }  
      else
      {
        login = 0;
        if(write(tdL.cl, &login, sizeof(int)) <= 0)
          perror("[server|login]Eroare la write la login NOT ok\n");
      }
      
    }


    if(logged)
    {
      printf("Logged in!\n");
      if(strstr(cmdReceived, "|send"))
      {
          // fortnite: buna siua
          strcpy(cmdReceived, cmdReceived+strlen("|send ")); //acum avem doar: "username: <mesaj>"
          char nume[256] = "";
          char delimiter = ':';
          int k = 0, n = 0;
          while(1)
          {
            if(delimiter == cmdReceived[k]) //mergem pana dam de ':'
              break;
            nume[n++] = cmdReceived[k];
            ++k;
          }
          printf("USERNAME extras: %s\n", nume);
          int isOn = getLog(nume);
          //int id = getTh(nume);
          //printf("USERNAME threadID: %d\n", id);
          //fflush(stdout);
          char output[2000] = "[from: ";
          strcat(output, username);
          strcat(output, "]");
          strcat(output, cmdReceived + strlen(nume) + 2); //aici extram mesajul propriu-zis

          if(isOn) //trimitem mesaj online direct
          {
            int id = getTh(nume);
            if(write(id, output, sizeof(output)) <= 0)
            {
              perror("Eroare la trimis MESAJUL BUN\n");
            }
          }
          else
          {
            char error[200] = "User offline! Try again later!";
            
            if(write(tdL.cl, error, sizeof(error)) <= 0)
            {
              perror("Eroare la trimis MESAJUL BUN\n");
            }
            //continue;
          }

      }

      if(strcmp(cmdReceived, "|exit") == 0)
      {
        //char quit[50] = "";
        //if(read(tdL.cl, quit, sizeof(quit)) <= 0)
        //{
           // perror("[server|login]Eroare la read la username\n");
        //}
        //if(strcmp(quit, "") != 0)
        //{
        char logger[10] = "0";
        logged = 0;
        setLog(logger, username);
        setThread(0, username);
        return 0;
        //}
      }

      if(strcmp(cmdReceived, "|listUsers") == 0)
      {
          sqlite3* db;
          sqlite3_stmt* stmt;
          int rc;
          char *err_msg = 0;
          // Open the database
          rc = sqlite3_open("messenger.db", &db);
          rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
          if (rc != SQLITE_OK) {
            fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
            return 1;
          }
          // Prepare the SQL statement
          rc = sqlite3_prepare_v2(db, "SELECT * FROM users where login='1';", -1, &stmt, NULL);
          if (rc != SQLITE_OK) {
            fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
            return 1;
          }
          // Iterate over the result set and extract the text value of the first column
          char* names = malloc(1);
          names[0] = '\0';
          while (sqlite3_step(stmt) == SQLITE_ROW) {
            char* name = (char*)sqlite3_column_text(stmt, 0);
            names = realloc(names, strlen(names) + strlen(name) + 2);
            strcat(names, name);
            strcat(names, " ");
          }
          
          sqlite3_finalize(stmt);
          sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
          sqlite3_close(db);

          if(names != NULL)
          {
            names[strlen(names) - 1] = '\0';
            for(int i = 0; i < strlen(names); i++)
            {
              if(names[i] == ' ')
                names[i] = '|';
            }
            char output[500] = "";
            strcpy(output, names);
            printf("[server]Useri ONLINE: %s\n", output);
            if(write(tdL.cl, output, sizeof(output)) <= 0)
            { 
              perror("[server|onlineUsers]Eroare la write catre client...\n");
            }
          }
          else
          {
            printf("[server]Niciun user online!\n");
            char none[100] = "";
            strcpy(none, "Niciun user online!");
            if(write(tdL.cl, none, sizeof(none)) <= 0)
            {
              perror("[server|onlineUsers(none)]Eroare la write catre client...\n");
            }
          }
          free(names);
      }

      if(strcmp(cmdReceived, "|logout") == 0)
      {

        logged = 0;
        int ok = 1;
        char logger[10] = "0";
        setLog(logger, username);
        setThread(0, username);
        printf("[logout/server]Am dat LOGOUT\n");
        fflush(stdout);
        if(write(tdL.cl, &ok, sizeof(ok)) <= 0) 
        {
          perror("[server]Eroare la write logout\n");
          exit(1);
        }   
        
      }


    }
    


  }

}