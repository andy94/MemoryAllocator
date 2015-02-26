// Ursache Andrei - 312CA

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/*DEFINE-uri*/

#define L 50

/*VARIABILE GLOBALE*/

int N; // Dimensiunea arenei
unsigned char *arena; // Arena

/*DEFINIRE STRUCTURI DE DATE*/

typedef struct cmd{ // structura comenzii
	char *nume; // numele comenzii
	int prm[3]; // parametrii numerici ai comenzii (maxim 3)
	char *cm; // pentru comanda SHOW (parametri de tip char - MAP,FREE...)
}COMANDA;

typedef struct bloc{ // structura zonelor de memorie-vezi comanda SHOW ALLOCATIONS
	char *tip; // FREE sau OCCUPIED
	int octeti; // parametrii comenzii (maxim 3)
}BLOC;

/*ANTET FUNCTII*/

void modifica(int index, int valoare); // pune valoarea "valoare" de tip int la adresa "index" din arena

void citire_comanda(COMANDA *com); // citeste comanda

void INITIALIZE(int l); // alocare de memorie dinamic a arenei de dimensiune N

void FINALIZE(); // elibereaza arena, incheie programul

void DUMP(); // afiseaza arena

int ALLOCALIGNED(int size, int align); // aloca size octeti aliniati la align

int ALLOC(int size); // caz particular ALLOCALIGNED (align = 1)

int FREE(int index); // schimba datele de gestiune a blocurilor vecine

void FILL(int index, int size, int value); // pune valoarea value in size octeti de la index

void SHOW_MAP(BLOC date[L], int length); // auxiliar al comenzii SHOW MAP

void SHOW(char *info, int length);

void REALLOC(int index, int size); // pentru eficientizarea gestiunii blocurilor din arena

int main(){
	COMANDA com; // comanda primita
	
	while(1){
		citire_comanda(&com); // citire comanda
		if(strcmp(com.nume, "INITIALIZE") == 0){
			INITIALIZE(com.prm[0]);
		}
		if(strcmp(com.nume, "FINALIZE") == 0){
			FINALIZE();
			break;
		}
		if(strcmp(com.nume, "DUMP") == 0){
			DUMP();
		}
		if(strcmp(com.nume, "ALLOC") == 0){
			ALLOC(com.prm[0]);
		}
		if(strcmp(com.nume, "FREE") == 0){
			FREE(com.prm[0]);
		}
		if(strcmp(com.nume, "FILL") == 0){
			FILL(com.prm[0],com.prm[1],com.prm[2]);
		}
		if(strcmp(com.nume, "SHOW") == 0){
			SHOW(com.cm,com.prm[1]);
		}
		if(strcmp(com.nume, "ALLOCALIGNED") == 0){
			ALLOCALIGNED(com.prm[0],com.prm[1]);
		}
		if(strcmp(com.nume, "REALLOC") == 0){
			REALLOC(com.prm[0],com.prm[1]);
		}
	}

	return 0;
}

/* CODUL FUNCTIILOR */

void modifica(int index, int valoare){ // pune valoarea "valoare" de tip int la adresa "index" din arena
	int* curent=(int*)(arena+index);
	*curent=valoare;
}

void citire_comanda(COMANDA *com){ // citeste comanda
	char date[L], *sep;
	int i=0, test=1;
	fgets(date,30,stdin);
	sep=strtok(date, " \n");
  	while(sep != NULL){
		if(i==0){
			com->nume=strdup(sep);
		}
		else{
			if(strcmp(com->nume,"SHOW") == 0 && test){
				com->cm=strdup(sep);
				test=0;
			}
			else
				com->prm[i-1]=atoi(sep);	
		}
		i++;
		sep=strtok(NULL, " \n");
	}
}

void INITIALIZE(int l){ // alocarea dinamica a memoriei pentru arena de dimensiune N
	N=l; // initializare N
	arena=calloc(N, sizeof(unsigned char));
	modifica(0,0); // initializare index de start
}

void FINALIZE(){ // elibereaza arena
	free(arena);
}

void DUMP(){ // afiseaza arena
	int k=0, i, j;
	for(i=0 ; i<N-16; i+=16){
		printf("%08X\t",i);
		for(j=0 ; j<16 ; j++){
			printf("%02X ",*(arena+k));
			k++;
		}
		printf("\n");
	}
	printf("%08X\t",i);
	while(k < N){
		printf("%02X ",*(arena+k));
		k++;
	}
	printf("\n%08X\n",N);

}

int ALLOCALIGNED(int size, int align){ // aloca size octeti aliniati la align
	int i,j;
	if(*((int*)(arena)) == 0){ // cazul in care arena este libera
		if(N-4 < size+12){ // daca nu exista spatiu
			printf("0\n");
			return 0;
		}
		else {
			for(j=0;j<N-4-size-12+1;j++){
				if(((16+j)%align) == 0){
					modifica(4+j,0);
					modifica(8+j,0);
					modifica(12+j,size+12);
					modifica(0,4+j);
					printf("%d\n",16+j);
					return (16+j);
				}
			}
			printf("0\n"); // este spatiu dar nu corespunde valorii align
			return 0;
		}
		
	}
	if(*((int*)(arena))-4 >= size+12){ // cazul in care este spatiu intre 0 si primul bloc
		for(j=0;j<*((int*)(arena))-4-size-12+1;j++){
			if((16+j)%align == 0){ // OK
				modifica(4+j,*((int*)(arena)));
				modifica(0,4+j);
				modifica(8+j,0);
				modifica(12+j,size+12);
				modifica(*((int*)(arena+4+j))+4,4+j);
				printf("%d\n",16+j);
				return 16+j;
			}
		} // daca nu e OK se continua cautarea
	}
	for(i=*((int*)(arena));i<N;){
		if(*((int*)(arena+i))==0){ // cazul in care nu este spatiu pana la ultimul bloc
			if(N-i-*((int*)(arena+i+8)) < size+12){ // nu este spatiu intre ultimul bloc si N
				printf("0\n");
				return 0;
			}
			else { // este spatiu intre ultimul bloc si final
				for(j=0;j<N-i-*((int*)(arena+i+8))-size-12+1;j++){
					if((i+*((int*)(arena+i+8))+j+12)%align == 0){ // ok
						modifica(i+*((int*)(arena+i+8))+j,0);
						modifica(i+*((int*)(arena+i+8))+4+j,i);
						modifica(i+*((int*)(arena+i+8))+8+j,size+12);
						modifica(i,i+*((int*)(arena+i+8))+j);
						printf("%d\n",i+*((int*)(arena+i+8))+12+j); 
						return i+*((int*)(arena+i+8))+12+j;
					}
				}
				printf("0\n"); // este spatu, dar nu corespunde
				return 0;
			}
			
		}
		if(*((int*)(arena+i))-i-*((int*)(arena+i+8)) >= size+12) { // daca este spatiu intre 2 blocuri
			for(j=0;j<*((int*)(arena+i))-i-*((int*)(arena+i+8))-size-12+1;j++){
				if((i+*((int*)(arena+i+8))+j+12)%align == 0){ // ok
					modifica(i+*((int*)(arena+i+8))+j,*((int*)(arena+i)));
					modifica(i+*((int*)(arena+i+8))+4+j,i);
					modifica(i+*((int*)(arena+i+8))+8+j,size+12);
					modifica(*((int*)(arena+i))+4,i+*((int*)(arena+i+8))+j);
					modifica(i,i+*((int*)(arena+i+8))+j);
					printf("%d\n",i+*((int*)(arena+i+8))+12+j);
					return i+*((int*)(arena+i+8))+12+j;
				}
			}
		}
		i=*((int*)(arena+i)); // daca nu e niciunul din cazur, continua cautarea
	}
	return -1;
}

int ALLOC(int size){ // caz particular ALLOCALIGNED
	return ALLOCALIGNED(size,1); // obtinem acelasi rezultat
}

int FREE(int index){ // schimba datele de gestiune a blocurilor vecine
	if(*((int*)(arena+index-8)) == *((int*)(arena+index-12))){ // cazul in care exista un singur bloc
		modifica(0,0);
		return 0;
	}
	modifica(*((int*)(arena+index-8)),*((int*)(arena+index-12)));// in restul cazurilor
	if(*((int*)(arena+index-12)) != 0){ // daca nu e ultimul
		modifica(*((int*)(arena+index-12))+4,*((int*)(arena+index-8)));
	}
	return 0;
}

void FILL(int index, int size, int value){ // pune valoarea value in size octeti de la index
	int i;
	for(i=0;i<size;i++){
		arena[index+i]=value;
	}
}

void SHOW_MAP(BLOC date[L], int length){ // auxiliar al comenzii SHOW MAP
	double parte=(float)N/length, proc; // procent, auxiliar pentru procent
	int *aux; // 1 daca este ocupat, 0 daca este liber
	int i, k=0, m=date[0].octeti, test=0, j=0, stop=1, trecut=1;
	aux=(int*)malloc(N*sizeof(int));
	for(i=0;i<N;i++){ // pune in aux[] pentru fiecare bit, tipul acestuia (1=ocp, 0=free)
		if(i+1 > m){
			k++;
			m+=date[k].octeti;
		}
		if(strcmp(date[k].tip,"OCCUPIED") == 0)aux[i]=1;
		else
			aux[i]=0;
	}
	test=0;
	k=0;
	stop=1;
	proc=parte;
	for(i=0;i<length;i++){ // pentru fiecare caracter (* sau .)
		stop=1;
		while(stop){
			if(aux[j] == 1)test=1; // daca bitul este ocupat
			if(trecut){ // daca nu a mai "trecut" de la acest bit
				if(proc-1 > 0){ // daca proc > 1
					proc=proc-1;
				}
				else {
					if(proc-1 < 0){ // daca proc < 1
						if(parte > 1){
							stop=0;
							j--; // ramai la acelasi bit
							proc=parte-(1-proc);
							trecut=0;
						}
						else { 
							proc+=parte;
							j--;// ramai la acelasi bit
							stop=0;
						}
					}
					else { // daca proc == 1
						stop=0;
						proc=parte;
					}
				}
			}
			else { // daca a mai trecut pe la acest bit
				trecut=1;
			}
			j++;
		}
		printf("%s",test?"*":"."); // afiseaza caracterul corespunzator
		if((i+1)%(80) == 0)printf("\n"); // pentru linii mai lungi de 80 de caractere
		test=0;
	}
	printf("\n");
	free(aux);
}

void SHOW(char *info, int length){ 
	int i,k=1,test=1;
	BLOC date[L]; // va contine blocurile si tipul lor
	date[0].tip=strdup("OCCUPIED");
	date[0].octeti=4;
	if(*((int*)(arena)) == 0){ // Daca nu este alocat nimic
		date[k].tip=strdup("FREE");
		date[k].octeti=N-4;
		k++;
		test=0;
	}
	if(test){
		if(*((int*)(arena))-4 > 0){ // Daca exista un spatiu liber intre indexul de start si primul bloc alocat
			date[k].tip=strdup("FREE");
			date[k].octeti=*((int*)(arena))-4;
			k++;
		}
		for(i=*((int*)(arena)) ; i<N ; ){
			date[k].tip=strdup("OCCUPIED");
			date[k].octeti=*((int*)(arena+i+8));
			k++;
			if(*((int*)(arena+i)) == 0){ // Daca e ultimul bloc
				if(N-i-*((int*)(arena+i+8)) > 0){ 
					date[k].tip=strdup("FREE");
					date[k].octeti=N-i-*((int*)(arena+i+8));
					k++;
				}
				break;
			}
			if(*((int*)(arena+i))-i-*((int*)(arena+i+8)) > 0){ // Daca exista spatii intre blocuri
				date[k].tip=strdup("FREE");
				date[k].octeti=*((int*)(arena+i))-i-*((int*)(arena+i+8));
				k++;
			}
			i=*((int*)(arena+i));
		}	
	}
	if(strcmp(info,"FREE") == 0){ // FREE
		int oct_lib=0, reg_lib=0; // octeti si regiuni libere
		for(i=0;i<k;i++){
			if(strcmp(date[i].tip,"FREE") == 0){
				oct_lib+=date[i].octeti;
				reg_lib++;
			}
		}
		printf("%d blocks (%d bytes) free\n",reg_lib,oct_lib);
	}
	if(strcmp(info,"USAGE") == 0){ // USAGE
		int oct_ocp=0, reg_ocp=0, oct_fol, reg_lib; // octeti, regiuni ocupate, octeti folositi, regiuni libere
		float eff, fragm;
		for(i=0;i<k;i++){
			if(strcmp(date[i].tip,"OCCUPIED") == 0){
				oct_ocp+=date[i].octeti;
				reg_ocp++;
			}
		}
		oct_fol=oct_ocp-((reg_ocp-1)*12)-4; // scad octetii din zonele de gestiune
		printf("%d blocks (%d bytes) used\n",reg_ocp-1,oct_fol); // -1 de la indexul de start
		
		eff=((float)oct_fol/oct_ocp)*100;
		printf("%d%% efficiency\n", (int)eff);
		
		reg_lib=k-reg_ocp;
		reg_lib--;
		reg_ocp--;
		if(*((int*)(arena)) == 0){
			printf("0%% fragmentation\n");
		}
		else {
		fragm=((float)reg_lib/reg_ocp)*100;
		printf("%d%% fragmentation\n", (int)fragm);
		}
	}
	if(strcmp(info,"ALLOCATIONS") == 0){ // ALLOCATIONS
		for(i=0;i<k;i++)
			printf("%s %d bytes\n",date[i].tip,date[i].octeti);
	}
	if(strcmp(info,"MAP") == 0){ // MAP - prin functie auxiliara
		SHOW_MAP(date,length);
	}
}

void REALLOC(int index, int size){
	int i,n;
	unsigned char *date; // valorile deja stocate ce trebuie sa fie "transferate"
	date=(unsigned char*)malloc(size*sizeof(unsigned char));
	if(size <= (*((int*)(arena+index-4))-12) ) // alegerea dimensiunii mai mici
		n=size;
	else
		n=(*((int*)(arena+index-4))-12);
	for(i=0;i<n;i++){
		date[i]=*(arena+index+i);
	}
	FREE(index);
	index=ALLOC(size);
	for(i=0;i<n;i++){
		*(arena+index+i)=date[i];
	}
	free(date);
}
