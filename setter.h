#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//BUBA!
void setLog(char ok[10], char username[256])
{
  sqlite3 *db;
  char *zErrMsg = 0;
  sqlite3_stmt *res;
  //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);
  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
  if (rc != SQLITE_OK) 
  {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db); 
    return 1;
  }

  char* sql;
  sql = (char*) malloc(2048);
  /* 
  snprintf(sql, mes_len + insert_len2, "UPDATE Users SET Login='%d'", ok);
  strcat(sql, " WHERE name='");
  strcat(sql, username);
  strcat(sql, "';");
  */ 
  strcpy(sql, "update users set login = ? where name ='");

  //strcat(sql, ok);
  //strcat(sql, "' where name='");
  strcat(sql, username);
  strcat(sql, "'");
  //sql[strlen(sql)] = '\0';
  //printf("[setLog]Am dat snprintf()!\n");
  
  //fflush(stdout);
  //rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
  
  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  if (rc != SQLITE_OK ) 
  {
    fprintf(stderr, "2Log) SQL error: %s\n", zErrMsg);
    //sqlite3_free(zErr_Msg);        
    sqlite3_close(db);
    return 1;
  } 
  else
  {
    
    sqlite3_bind_text(res, 1, ok, -1, SQLITE_STATIC);
  }
  
  int step = sqlite3_step(res);
  printf("[setLog]Comm dupa prepare_v2(): %s\n", sql);
  if (rc != SQLITE_DONE) 
  {
      fprintf(stderr, "SETLOG Sql error... %s\n", zErrMsg);
  }
  //printf("[setLog]Did it work? %d", ok);
  //sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  
  sqlite3_finalize(res);
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  sqlite3_close(db);  
  free(sql);
  //printf("[setLog]Finished\n");
  //fflush(stdout);
}

void setThread(int td, char username[256])
{
  sqlite3 *db;
  char *err_msg = 0;
  //fflush(stdout);
  fflush(stdin);
  //sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  int rc = sqlite3_open("messenger.db", &db);

  rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
  
  if (rc != SQLITE_OK) 
  {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db); 
    return 1;
  }
  
  char* sql;
  sql = (char*) malloc(2048);
  
  //update users set td = 1 where name = 'andi';
  int name_len = strlen(username);
  int mes_len = sizeof(int);
  //int insert_len = strlen("UPDATE Users SET Login='' WHERE name='';");
  int insert_len2 = strlen("UPDATE Users SET td=''");

  //snprintf(sql, name_len + mes_len + insert_len, "UPDATE Users SET Login='%d' WHERE name='%s';", ok, username);
  snprintf(sql, mes_len + insert_len2, "UPDATE Users SET td='%d'", td);
  strcat(sql, " WHERE name='");
  strcat(sql, username);
  strcat(sql, "';");
  sql[strlen(sql) - 1] = '\0';
  strcat(sql, ";");
  printf("[setTD]Command: %s\n", sql);
  //printf("[setTd]Comm_last: %c\n", sql[strlen(sql) - 1]);
  fflush(stdout);
  rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
  if (rc != SQLITE_OK ) 
  {
    fprintf(stderr, "2Thread) SQL error: %s\n", err_msg);
    //fflush(stdout);
    sqlite3_free(err_msg);        
    sqlite3_close(db);
    return 1;
  } 
  //printf("Am terminat de updatat tdCl!!!\n");
  //fflush(stdout);
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  free(sql);
  sqlite3_close(db);  
}


