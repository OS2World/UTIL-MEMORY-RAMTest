/* RAmTst.cpp */
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <builtin.h>
#include <time.h>
#define INCL_DOS
#include <os2.h>


unsigned int ram_info(void);
APIRET16 APIENTRY16 Dos16MemAvail ( PULONG pulAvailMem ) ;

int testram(int MemMb, int Nraz);
int TestRamCicle(int *p, int n, int v, int vset);
int AllocHiMemory(void * *pMem, unsigned int memsize);
int IsKeyPressed(void);
int WriteINI(char *progName);
int CheckINI(char *progName);


static int UseHiMemory = 0;
static void * pMem = NULL;
static int MAX_RAM = 0, isSkip = 0, isSilent=0;
static double dNrw=0.;
unsigned int MinTestPeriod=0;
time_t LastTestTime = 0;

char version[]="RamTest 0.4a" __DATE__;

int main(int n, char *par[])
{  int rc, Nraz=4,i;
   int t0,t1, isIni;
   unsigned int FreeRam;

   Dos16MemAvail ( (PULONG) &FreeRam ) ;

   if(n >= 2)
   {   if(!stricmp(par[1],"-help") || !stricmp(par[1],"-?") || !stricmp(par[1],"-h") || !stricmp(par[1],"/?") || !stricmp(par[1],"/h") )
       {  printf("%s\n", version);
          printf("Usage: RamTest [Mb [N]] [-s]\n");
          printf("Mb - How many Mb to test, 0 = All Free mem. Default = 0\n");
          printf("N  - number of runs, if N > 8 then use long rigorous bit test. Default = 4\n");
          printf("-s - silent mode\n");
          exit(2);
       }
       if(!stricmp(par[1],"-s"))
       {  isSilent = 1;
       } else {
          MAX_RAM = atoi(par[1]);
          if(n > 2)
          {    if(!stricmp(par[2],"-s"))
               {  isSilent = 1;
               } else {
                  Nraz = atoi(par[2]);
                  if(n > 3)
                  {    if(!stricmp(par[3],"-s"))
                                isSilent = 1;
                  }
               }
          }
       }
   }
   isIni = CheckINI(par[0]);
   if(isIni == 1)
      return 0;

   if(MAX_RAM == 0)
      MAX_RAM = FreeRam/(1024*1024);
   if(MAX_RAM <1) MAX_RAM = 1;
   t0 = clock();
   rc = testram(MAX_RAM, Nraz);
   t1 = clock();
   if(rc == 0)
   {   if(isSkip) printf("TestRam skipped + Ok\n");
       else       printf("TestRam Ok          \n");
       if(!isSkip)
         printf("Take %.1f sec with %.fMbyte/sec read+write\n", double(t1-t0)/1000.,  dNrw/(double(t1-t0)/1000.)/(1024.*1024.));
   } else {
      printf("\aTestRam ERROR!!! %i errors count\n\a",rc);
     if(!isSilent)
      for(i=0;i<16;i++)
      {  DosBeep(1000+i*10,500);
         DosBeep(700,500);
      }
      rc = 1;
      if(!isSilent)
      {  printf("Press any key to continue\n");
         getch();
      }
   }
   if(pMem)
      DosFreeMem(pMem);

   if(isIni == 0 && !isSkip) WriteINI(par[0]);

   exit(rc);
}

/* Check Ini file */
int CheckINI(char *progName)
{
    char IniFile[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char str[_MAX_FNAME], *pstr, *ptok1, *ptok2;
    char ext[_MAX_EXT];
    FILE *fp;
    int arg,rc, is=0;
    double dt;

    _splitpath(progName, drive, dir, str, ext);
    _makepath(IniFile, drive, dir, str, "ini");
    fp = fopen(IniFile,"r");
    if(fp == NULL)
       return -1;
    for(;;)
    {
      pstr = fgets(str,_MAX_FNAME-2,fp);
      if(pstr == NULL)
          break;
      if(*pstr == ';') continue;
      if(*pstr == '#') continue;
      ptok1 = strtok(pstr,"=");
      if(ptok1)
      {  ptok2=strtok(NULL,"=");
         if(ptok2)
         {   if(!stricmp(ptok1,"MinTestPeriod"))
             { rc = sscanf(ptok2, "%u", &MinTestPeriod);
               if(rc == 1) is++;
             }
             if(!stricmp(ptok1,"LastTestTime"))
             { rc=sscanf(ptok2, "%u", &LastTestTime);
               if(rc == 1) is++;
             }
         }
      }
    }
    fclose(fp);

    if(is == 2)
    {  if(MinTestPeriod > 0)
       {  time_t timeNow;
          time(&timeNow);
          dt = difftime(timeNow,LastTestTime);
          if(dt < MinTestPeriod)
          {   if(dt > 3600.*2)
                   printf("Previous test %i hours early\n", int(dt/3600));
              return 1;
          } 
       }
    }
    return 0;
}

int WriteINI(char *progName)
{   char IniFile[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char str[_MAX_FNAME], *pstr;
    char ext[_MAX_EXT];
    FILE *fp;
    _splitpath(progName, drive, dir, str, ext);
    _makepath(IniFile, drive, dir, str, "ini");
    fp = fopen(IniFile,"w");
    if(fp == NULL)
       return 1;
    time(&LastTestTime);

    fprintf(fp,";%s ini\n", version);
    fprintf(fp,";Minimum RAM test period in sec, 86400=day\n");
    fprintf(fp,"MinTestPeriod=%u\n",MinTestPeriod);
    fprintf(fp,";Time of last RAM test\n");
    fprintf(fp,"LastTestTime=%u\n",LastTestTime);
    fclose(fp);
    return 0;
}

/* RAM test */
int testram(int MemMb, int Nraz)
{   int *p,i,v0,vv,v1;
    int n,rc=0,rc0=0, rc01;
    int t0,t1;
    printf("Test RAM %iMb Ncounts %i ",MemMb,Nraz);
    printf("Press any key to skip test\n");
    rc = AllocHiMemory(&pMem, MemMb*1024*1024);
    if(rc )
    {   printf("Error allocation memory rc=%i\n", rc);
        DosBeep(700,500);
        printf("Press any key to continue\n");
        getch();
        exit(1);
    }
//    printf("Allocated at %p %uMb\n", pMem, MemMb);

    n =  MemMb*1024*1024/sizeof(int);
    p = (int *)pMem;
//    p = (int *) calloc(n + 16,sizeof(int));
//    if(p == NULL)
//    {   printf("Error Alloc mem\n");
//        return -1;
//    }
/* все должны быть нулями */
    v0  = 0;
    for(i=0;i< Nraz;i++)
    {  rc01 = 0;
         t0 = clock();
           printf("Zeros...        \r");  fflush(stdout);
           v1 = 0xffffffff;
           rc = TestRamCicle(p, n, v0, v1);
           if(rc) rc01++;
           t1=clock();
           printf("%i Ones...      \r",rc);  fflush(stdout);
           if(IsKeyPressed()) break;
           t0 = t1;
           v0 = v1;
           v1 = 0x55555555;
           rc = TestRamCicle(p, n, v0, v1);
           if(rc) rc01++;
           printf("%i 0x55555555...\r",rc);  fflush(stdout);
           if(IsKeyPressed()) break;
           v0 = v1;
           v1 = 0xaaaaaaaa;
           rc = TestRamCicle(p, n, v0, v1);
           if(rc) rc01++;
           printf("%i 0xaaaaaaaa...\r",rc);  fflush(stdout);
           if(IsKeyPressed()) break;
           v0 = v1;
           v1 = 0x0;
           rc = TestRamCicle(p, n, v0, v1);
           if(rc) rc01++;
          //  printf("%i  (%i)\n",rc);  fflush(stdout);
          //  DosSleep(100);
            if(rc01 && !isSilent)
            {   rc0 += rc01;
                DosBeep(1000+i*100,10+rc0%200);
            }
           v0 = v1;
           if(IsKeyPressed()) break;
    }
    if(i != Nraz) isSkip = 1;
    if(rc0 || Nraz > 8)
    { v0 = v1;
      if(isSkip) { printf("Press any key to skip test "); isSkip = 0; }
      printf("\n");
      for (i=0;i<256;i++)
       {    v1 = i | (i<<8) | (i<<16) | (i <<24);
         t0 = clock();
           rc = TestRamCicle(p, n, v0, v1);
            if(rc == 0)
            {     printf("%i %i%% %x \r", rc0, i*100/255, v1); fflush(stdout);
            } else {
              if(!isSilent) DosBeep(1000+i*10,10+rc0%200);
               rc0++;
            }
            v0 = v1;
         t1 = clock();
         if(t1 - t0 > 1000)
            DosSleep(10);
         if(IsKeyPressed())
         {  isSkip++;
            break;
         }
       }
    }
//    free(p);
    return rc0;
}

int TestRamCicle(int *p, int n, int v, int vset)
{  int i,  rc = 0, *pp;
    pp = p;
    for(i=0;i<n;i++)
    {  if(*pp != v)
       {  //pp = p + i;
          if(!isSilent)
               printf("ErrMem at %i (%p) (%x, must be %x)\n", i, pp, *pp, v);
          rc = 1;
//          break;
       }
       *pp++ = vset;
    }
    dNrw += double(n)*sizeof(int);
    return rc;
}

int AllocHiMemory(void * * pMem, unsigned int memsize)
{
    APIRET  rc;
    char    *ptr;

    /* Call DosAllocMem to get the initial block of memory */
    if (0 != (rc = DosAllocMem(pMem, memsize,
                               PAG_WRITE | PAG_READ | PAG_COMMIT| OBJ_ANY))) {
       UseHiMemory = 0;
       if (0 != (rc = DosAllocMem(pMem, memsize,
                               PAG_WRITE | PAG_READ | PAG_COMMIT)))
       {
          return rc;
       }
       return 0;
    }
    UseHiMemory = 1;

    return 0;
}

int IsKeyPressed(void)
{  int rc = 0;
   if(_kbhit())
   {  rc = _getch();
    //  printf("rc 0 %x \n", rc);
      while(_kbhit())
      { rc = _getch();
        //printf("rc 1 %x \n", rc);
      }
   }
   return rc;
}
