CC=gcc
default:
	@echo
	@echo  MIG Logcleaner by no1 \(greyhats.za.net\)
	@echo  -----------------------------------------
	@echo
	@echo  "usage: make <system type>"
	@echo
	@echo  "<system type> -  linux"
	@echo  "              -  bsd"
	@echo  "              -  sun"
	@echo
linux: mig-logcleaner.c
	$(CC) -DLINUX -Wall -o mig-logcleaner mig-logcleaner.c
bsd: mig-logcleaner.c
	$(CC) -DBSD -Wall -o mig-logcleaner mig-logcleaner.c
sun: mig-logcleaner.c
	$(CC) -DSUN -Wall -o mig-logcleaner mig-logcleaner.c
clean:
	rm -f mig-logcleaner *~
