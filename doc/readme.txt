RamTest 0.4 June 16 2005

Preface.

....Much knowledge multiplies the grief

RamTest is utility for fighting with rakes from memory manufactures...
Even if you have so called "industrial PC" and you have tested it for many
hours and memory chips are marked with many labels and so on, nevertheless
there is non-zero possibility that at some moment it will break.
--------------------------------------------------------------------------

Usage: RamTest [Mb [N]] [-s]

Mb - How many Mb to test, 0 = All Free mem. Default = 0
N  - number of runs, if N > 8 then use long rigorous bit test. Default = 4
-s - silent mode

RamTest can be used from command line as from CONFIG.SYS with CALL like
REM Fast free RAM test
CALL=C:\RAMTEST.EXE  

or

REM rigorous free RAM test
CALL=C:\RAMTEST.EXE  0 16

If Mb is greate that free RAM, than you will test RAM + disk read/write,
the more difference, the slowly test speed will be.

If you are unlucky, then you will see something like at
Captured1.gif and Captured2.gif

Typical RamTest output will be like:
==============================
E:\Evgen\RAMTEST>ramtest.exe
Test RAM 783Mb Ncounts 4 Press any key to skip test
TestRam Ok
Take 30.6 sec with 409Mbyte/sec read+write
==============================

It is possible to test RAM ones a day with ramtest.ini file placed in the 
same directory as ramtest.exe

;RamTest 0.4 Jun 16 2005 ini
;Minimum RAM test period in sec, 86400=day
MinTestPeriod=3600
;Time of last RAM test
LastTestTime=1118903926



For regular memory testing please use utilits like Memtest86+ from
http://www.memtest.org
