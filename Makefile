#
# Makefile - convenience makefile
#
# This file is Public Domain
#

.PHONY:	emumiga emumiga-clean frontend frontend-clean all clean

emumiga:
	make -C ../.. emumiga-quick

emumiga-clean:
	make -C ../.. emumiga-clean

frontend:
	make -C ../.. emumiga-frontend-quick

frontend-clean:
	make -C ../.. emumiga-frontend-clean

all:	emumiga

clean:	emumiga-clean
