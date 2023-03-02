# Loader-de-executabile
Imlementare sub forma unei biblioteci partajate/dinamice un loader de fișiere executabile în format ELF pentru Linux.
Loader-ul încărca fișierul executabil în memorie pagină cu pagină, folosind un mecanism de tipul demand paging - o pagină va fi
încărcată doar în momentul în care este nevoie de ea.
______________________________________________________________________________________
                        
                                   Tema so readme
                         
 _____________________________________________________________________________________

Proiectul presupune realizarea unui loader de executabile care trateaza semnalele si le interpreteaza
pentru a directiona 
sigsegv-urile, si a asigura maparea corecta a memoriei.

Se respecta protocolul de tratare a semnalului astfel:
->verificam daca este sigsegv(139),altfel apelam default_handlerul

->verificam daca segmenul la dare apar probleme este in segmenul nostru de memorie,daca nu,apelam 
default_handlerul

->gasim segmenul din memoria noastra la care e problema, verificam daca e mapata(daca nu e mapata,
o mapam si marcam maparea acesteia si folosim mprotect pentru a i seta corect permisiunile)
(daca e mapata,apelam default_handlerul).
