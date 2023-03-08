#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>

int port;

void block_ctrl_c() {
  signal(SIGINT, SIG_IGN);
}

void* doReceiving(void* sockID)
{
  
  int clientSocket = *((int *) sockID);
	while(1)
  {
		char data[2000];
		int read = recv(clientSocket, data, sizeof(data), 0);
		data[read] = '\0';
		printf("\n%s\n", data);
    //fflush(stdout); //POATE FI GRESIT
	}

}

int main(int argc, char* argv[])
{
  block_ctrl_c();
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi (argv[2]);

  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("Eroare la socket...\n");
    exit(1);
  }

  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  //de com?
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
    perror ("[client]Eroare la connect().\n");
    exit(1);
  }
  printf("Connection established..........\n");
  /*
  int pid = getpid();
  printf("pid: %d\n", pid);
  if(write(sd, &pid, sizeof(pid)) <= 0)
  {
    perror("Eroare la trimis PID");
  }
  */
  pthread_t thread;
  pthread_create(&thread, NULL, doReceiving, (void *) &sd);


  char command[1000];
  bzero(command, sizeof(command));
  int exista = 1;
  //int login = 0;
  int logged = 0;
 
  printf("Welcome to Offline Messenger!\n");
  fflush(stdout);
  int c;
  while(1)
  {
    //fflush(stdout);
    if(logged == 0)
    {
      printf("Choose an option: \n|login \n|register \n|send <nume>: <mesaj> \n|listUsers \n|logout \n|exit \n\n");
    }
    bzero(command, sizeof(command));
    
    if (fgets(command, sizeof(command), stdin) == NULL) 
    {
      perror("eroare la fgets[7]\n");
    }
    command[strlen(command) - 1] = '\0';
    //printf("Ultimul char din commanda: %c\n", command[strlen(command) - 1]);
    //scanf("%s", command);
    //fflush(stdout);
  
    
    if (write(sd, command, sizeof(command)) <= 0)
    {
      perror("[client]Eroare la write la comanda\n");
      exit(1);
    }
    //fflush(stdin);
    if(strcmp(command, "|register") == 0)
    {
    
      char username[256]="";
      char password[256]="";
      while(exista == 1)
      {
        bzero(username, sizeof(username));
        bzero(password, sizeof(password));
        bzero(command, sizeof(command));
        printf("\nEnter username: ");
        /*
        if (fgets(username, sizeof(username), stdin) == NULL) 
        {
          perror("eroare la fgets[2]\n");
        }
        username[strlen(username) - 1] = '\0';*/
        scanf("%s", username);
        fflush(stdin);
        
        if (write(sd, username, sizeof(username)) <= 0)
        {
          perror("[client]Eroare la write username\n");
          exit(1);
        }
       
        //aici primim raspuns de la server daca utilizatorul exista deja sau nu
        if(read(sd, &exista, sizeof(exista)) <= 0)
        {
          perror("[client]Eroare la read daca exista user\n");
          exit(1);
        }
        fflush(stdout);
        //while ((c = getchar()) != '\n' && c != EOF);
       // fflush(stdin);
        if(exista)
          printf("Username already exists!\n");
        else
          break;
        
      }
      fflush(stdout);
      printf("\nEnter password: ");
      bzero(password, sizeof(password));
      /*
      if (fgets(password, sizeof(password), stdin) == NULL) 
      {
        perror("eroare la fgets[3]\n");
      }
      password[strlen(password) - 1] = '\0';*/
      scanf("%s", password);
      fflush(stdin);
      
      //trimitem si parola la server
      if (write(sd, password, sizeof(password)) <= 0)
      {
        perror("[client]Eroare la write password\n");
        exit(1);
      }
      //daca am ajuns aici probabil ne-am inregistrat ok
      int succes = 0;
      if(read(sd, &succes, sizeof(succes)) <= 0)
      {
        perror("[client]Eroare la read register succes\n");
        exit(1);
      }
      //fflush(stdin);
      if(succes)
      {
        printf("%s registered successfully!\n", username);
        logged = 1;
      }
      //fflush(stdin);
      //while ((c = getchar()) != '\n' && c != EOF);
    }
    

    if(strcmp(command, "|login") == 0 && logged == 0)
    {
    
      char username[256];
      char password[256];
      bzero(username, sizeof(username));
      bzero(password, sizeof(password));
      while(logged == 0)
      {
        bzero(username, sizeof(username));
        bzero(password, sizeof(password));
        
        printf("\nEnter username: ");
        /*
        if (fgets(username, sizeof(username), stdin) == NULL) 
        {
          perror("eroare la fgets[4]\n");
        }
        username[strlen(username) - 1] = '\0';*/
        scanf("%s", username);
        fflush(stdin);
        fflush(stdout);
        if (write(sd, username, sizeof(username)) <= 0)
        {
          perror("[client|login]Eroare la write username\n");
          exit(1);
        }
        //fflush(stdin);
        
        printf("\nEnter password: ");
        //while ((c = getchar()) != '\n' && c != EOF);
        //bzero(command, sizeof(command));
        /*
        if (fgets(password, sizeof(password), stdin) == NULL) 
        {
          perror("eroare la fgets[5]\n");
        }
        password[strlen(password) - 1] = '\0';*/
        scanf("%s", password);
        fflush(stdin);
        // while ((c = getchar()) != '\n' && c != EOF); //fflush stdin
        // fgets(password, sizeof(password), stdin);
        if (write(sd, password, sizeof(password)) <= 0)
        {
          perror("[client|login]Eroare la write password\n");
          exit(1);
        }
        int login = 0;
        if(read(sd, &login, sizeof(login)) <= 0)
          perror("[client|login]Eroare la login ok\n");
       // fflush(stdin);
        if(login == 0)
          printf("Username/password incorrect! Try again!\n");
        else
        {
          logged = 1;
          login = 1;
          bzero(command, sizeof(command));
          printf("Login successfully!\n");
        }
        //while ((c = getchar()) != '\n' && c != EOF);
        //fflush(stdin);

      }
    }

    if(strcmp(command, "|exit") == 0)
    {
      printf("Exiting...\n");
      fflush(stdout);
      
      exit(0);   
    }
    
    if(logged == 0)
    {
      printf("You are not logged in!\n");
      continue;
    }
    else
    {
      //bzero(command, sizeof(command));d
      
      if(strstr(command, "|send"))
      {
        //trimitem comanda propriu zisa
        
        if (write(sd, command, sizeof(command)) <= 0)
        {
          perror("[|send3]Eroare la write catre server\n");
          exit(1);
        } 
        

      }
      //FIX!!!!!!!! (se afiseaza doar andi bla)
      else if(strcmp(command, "|listUsers") == 0)
      {
        char users[2000];
        users[0] = '\0';
        /*
        if (write(sd, command, sizeof(command)) <= 0)
        {
          perror("[onlineUsers]Eroare la write catre server\n");
          exit(1);
        } */
        //FIX!!!!!!!! (se afiseaza doar andi bla)

        if(read(sd, users, sizeof(users)) <= 0)
          perror("[client|online]Eroare la primit online users\n");
        
        printf("Online users: %s\n", users);
        
        fflush(stdout);
        //fflush(stdin);
        //printf("Useri ONLINE\n");
        
      }
      //de rezolvat!!!!!
      else if(strstr(command, "|logout"))
      {
        int ok = -1;
        logged = 0;
        //comanda de logout
        /*
        if (write(sd, command, sizeof(command)) <= 0)
        {
          perror("[logout]Eroare la logout catre server\n");
          exit(1);
        }
        */ 

        //fflush(stdout);
        if(read(sd, &ok, sizeof(ok)) <= 0)
        {
          perror("[logout]Eroare la logout\n");
          exit(1);
        }
        printf("(logout=%d)Logging out...\n", ok);
        
        fflush(stdout);
        //fflush(stdin);
      }
      //fflush(stdin);
      //while ((c = getchar()) != '\n' && c != EOF);
    }
    
  }

  return 0;
}
