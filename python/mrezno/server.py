#bind((host,port)) #povezuje socket s racunalom
#na tom portu server slusa, daje novi po uspostavi dvosmjernog tunela
#listen(n) #ceka spajanje, n je max duljina reda
#accept() #prihvaca klijenta i vraca (conn,adress) #conn=socket(neki novi port), adress=ip adresa

#recv(maxsize)
#close()
#decode("utf-8") #vraca string

#pr 1 jedno spajanje i preinaka poruke #SOA

import socket as soc
import threading as thr

"""
sock=soc.socket()
sock.bind((soc.gethostname(),10000))#localhost umjesto
#otvoreni tunel za inicijalno spajanje na server na port 10000
#za inicijalni port dozvola u firewall
sock.listen(5)#aktivacija tunela za prihvacanje
print(f"cekam klijenta i zovem se {soc.gethostname()}")

#prihvacanje klijenta(jednog jer nema petlje)
sockklijent,addrklijent=sock.accept()
print(f"spojen s klijentom s adresom {addrklijent}")

poruka=sockklijent.recv(1024).decode("utf-8")
#1024 je dogovor ili se pokupi s mreze kao MTU
print(f"klijent salje poruku {poruka}")

poruka=poruka.upper()
sockklijent.send(poruka.encode("utf-8"))

sockklijent.close()
sock.close()
"""

#pr 2 provjeravati je li broj prost
#parareraliam i accept u petlji
def cekajPoruke(s):
    global radi
    #svaki klijent svoj socket u svojoj dretvi
    gotovo=False
    while not gotovo:
        try:
            n=int(s.recv(1024).decode("utf-8"))
            print(f"Broj:{n} od klijenta {s}\n",end="")
            if n==-1:
                radi=False#tajni kod za gasenje servera
                #ak se gasi prije slj acc, onda global S i S.close()
            if n<2:
                gotovo=True#gasenje klijenta i servera
            elif prost(n):
                s.send(f"Broj {n} je prost".encode("utf-8"))
            else:
                s.send(f"Broj {n} nije prost".encode("utf-8"))
        except:
            gotovo=True#ubijamo dretvu/klijenta koji je izazvao problem
    #print(f"Klijent {s} odspojen")
    s.close()
def prost(n):
    for i in range(2,round(n**0.5)+1):
        if n%i==0:return False
    return True
H=soc.gethostname()
P=10000
S=soc.socket()
S.bind((H,P))
S.listen(5)
radi=True
while radi:
    print("cekam klijenta...\n",end="")
    s,a=S.accept()
    #print(f"klijent spojen {s} {a}\n",end="")
    t=thr.Thread(target=cekajPoruke,args=(s,))
    t.start()
    