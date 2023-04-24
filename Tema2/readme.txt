Pasat Ionut - Grupa 324CC

    Pentru implementarea sistemului server-client am folosit ca schelet de 
plecare rezolvarea labului 8. Am ales sa fac urmatoarele:
---SUBSCRIBER---
    In subscriber singura schimbare fata de codul din lab a fost dezactivarea
algoritmului lui Nagle. In acest sistem subscriber-ul are rolul sa dea cele
2 comenzi de subscribe/unsubscribe, comanda de exit si sa primeasca string-ul
deja prelucrat de server si sa-l afiseze.
----SERVER-----(the main character)
    In prima instanta server-ul asteapta conectarea clientilor. Dupa ce un
client se conecteaza, acesta este adaugat in lista de clienti care sunt
conectati la server. Dar nu inainte de a verifica daca un client cu acelasi
id este deja conectat, caz in care clientul care incearca sa se conecteze este
inchis. Fiecare client este sub forma unei structuri care contine id-ul lui,
IP-ul de pe care s-a conectat, portul, socketul asociat, un camp de activ
(1 daca e activ, 0 daca e deconectat), un vector de stringuri in care se
pastreaza mesajele trimise de UDP pe un anumit topic la care clientul este
abonat, dar pe care nu le poate primi deoarece e deconectat, urmand sa le
primeasca la urmatoarea reconectare. Revenind, la conectarea/reconectarea unui 
client se afiseaza mesajul corespunzator in functie de eveniment.
    In al doilea rand, serverul poate sa dea o singura comanda la STDIN si
anume "exit". Caz in care se inchide serverul si toti clientii acestuia.
    Al treilea si cel mai important aspect a fost primirea pachetelor UDP.
Segmentarea acestor pachete am realizat-o folosind o structura ajutatoare,
ale carei campuri se modelau dupa tipul pachetului de la UDP: "topic" sir
de maxim 50 de caractere, "tip_date" octetul ce reprezenta tipul de date 
trimis si "payload" sir de caractere de maxim 1500 de octeti. Odata ce aveam
acces la aceste campuri am putut crea prima parte a stringului de trimis catre
subscriber ("<IP_CLIENT_UDP>:<PORT_CLIENT_UDP> - <TOPIC> -").
    In continuare, in functie de tipul de date completaez stringul final.
Pentru fiecare tip de data mai putin SHORT_REAL deoarece payloadul acestuia
contine un singur numar pe 2 octeti, am segmentat payloadul conform cerintei
folosind tot o structura pentru a putea accesa campurile asa cum trebuie.
Pentru transformarea numerelor in sir de caractere am folosit utilitarul
"sprintf".
    In final, daca serverul primea comenzi de la subscriber le asociam astfel:
"subscribe" - folosesc "sscanf" pentru segmentarea comenzii in
(comanda + topic + sf). Caut in vector, clientul al carui socket corespunde cu
cel de pe care s-a primit comanda, si adaug in vectorul lui de subscriptii
topicul preluat.
"unsubscribe" - fac aceeasi preluare a campurilor comenzii, caut in vector
socketul corespunzator clientului si sterg topicul din vectorul lui de
subscriptii.
    Motivul pentru care am ales aceasta abordare in care serverul face toata
munca nu e unul argumentat ci mai mult ca asa mi s-a parut logic si a fost
de asemenea si prima idee care mi-a venit in cap. Am incercat sa trimit direct
o structura peste TCP in client si sa prelucrez mesajul acolo insa o faceam
gresit si primeam niste erori neobisnuite.
    De asemenea am ales sa fac bufferele de receive statice cu o lungime 
prestabilita si in ceea ce tine de programare defensiva am incercat sa fiu
cat mai atent.



