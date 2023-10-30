#connect((host,adress)) #spaja se na trazeni port posluzitelja
#send(s) #paketi niz bajtova
#encode("utf-8") #niz bajtova
#import socket as soc

"""
#pr jedno spajanje i preinaka poruke
sock=soc.socket()
sock.connect(("17C-02",10000))#localhost
#connect tako dugo dok se ne spoji
print("spojeno")
poruka=input()
sock.send(poruka.encode("utf-8"))
for i in range(3):
    primljena=sock.recv(1).decode("utf-8")
    print(primljena)
primljena=sock.recv(1024).decode("utf-8")
print(primljena)
sock.close()
"""

import socket as soc
H="17C-02"
P=10000
S=soc.socket()
S.connect((H,P))

print(f"ja sam {S} na {P}\n",end="")
t=2
while t>=2:
    t=int(input("unesi broj, <2 na kraju"))
    S.send(str(t).encode("utf-8"))
    print(S.recv(1024).decode("utf-8"))