/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include "exec_parser.h"
#include <unistd.h>

#define pagesize 4096
#define SIGSEGV_ERROR 139
static so_exec_t *exec;
static void *default_handler;
static int fisier;
so_seg_t * FOUND_IT = NULL;

//set permissions and flags
int _prot = PROT_READ | PROT_WRITE |PROT_EXEC;
int _flags = MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS;
int index_pagina;

static void segv_handler(int signum, siginfo_t *info, void *context)
{	int test_err = 0;
	int test = -1;
	so_seg_t ** search_segment = malloc(exec->segments_no * sizeof(so_seg_t*));
	
	if(signum == SIGSEGV){
		//construim un vector cu adresele la are se alfa fiecare segment
		for(int i = 0; i < exec->segments_no; i++){
			search_segment[i] = &(exec->segments[i]);
		}

		//pentru fiecare segment din exec, verificam daca adresa cu probleme e in segmentul curent
		for(int i = 0; i < exec->segments_no; i++){
			if((int)info->si_addr >= search_segment[i]->vaddr){
				FOUND_IT = search_segment[i];
				continue;
			}
		}

		//tratam semnalul
		if(FOUND_IT){

			//aflam unde in segment se afla pagina cu probleme(a cata pagina)
			index_pagina = ((int)(info->si_addr) - FOUND_IT->vaddr) / pagesize;
			
			//caz 3
			//verificam ca e mapata deja
			if(FOUND_IT->data){

				//daca e mapata => default handler
				if(((int*)(FOUND_IT->data))[index_pagina] == 1){
					signal(SIGSEGV,default_handler);	
				}

				//daca nu e mapata
				if(((int*)(FOUND_IT->data))[index_pagina] == 0){

					//mapam adresa care a generat SIGSEGV
					char *pageAddress = mmap((void *)FOUND_IT->vaddr + index_pagina * pagesize, pagesize, _prot, _flags, -1, 0);

					//markMapping
					((int*)(FOUND_IT->data))[index_pagina] = 1;
			
					if(FOUND_IT->file_size - index_pagina * pagesize > pagesize){

						//trebuie sa citesc pagesize
						if(index_pagina * pagesize < FOUND_IT->file_size){
							lseek(fisier,FOUND_IT->offset + index_pagina * pagesize,SEEK_SET);
							read(fisier, pageAddress, pagesize);
						}

					}
					else{

						//trebuie sa citesc file_size - offset
						if(index_pagina * pagesize < FOUND_IT->file_size){
							lseek(fisier,FOUND_IT->offset + index_pagina * pagesize,SEEK_SET);
							read(fisier, pageAddress, FOUND_IT->file_size - index_pagina * pagesize);
						}
					}

					//ne asiguram ca nu se acceseaza cu drepturi pe care nu le avem
					test_err = mprotect(pageAddress, pagesize, FOUND_IT->perm);
					if(test_err == -1){
						signal(SIGSEGV,default_handler);
					}
				}
			}
			test = 0;	
		}
		if(test == -1){
			signal(SIGSEGV,default_handler);
		}
	}else{

		//caz in care nu e sigsegv sau nu se gaseste problema in segment
		signal(SIGSEGV,default_handler);
	}
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;
	
	default_handler = signal(SIGSEGV,NULL);

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	int pages_no;
	exec = so_parse_exec(path);

	//luam fisierul
	fisier = open(path, O_RDONLY);

	if (!exec)
		return -1;

	//alocam peste tot data ca sa vedem unde am mapat si unde nu
	for (int i = 0; i < exec->segments_no; i++) {
		pages_no = exec->segments[i].mem_size / pagesize;
		exec->segments[i].data = (int *)calloc(pages_no, sizeof(int));
	}

	so_start_exec(exec, argv);

	return -1;
}
