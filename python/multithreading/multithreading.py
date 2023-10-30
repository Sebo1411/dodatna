#Thread(target=ime_potprograma,args=(argumenti odvojeni od potprograma))
#start()
#join()
#povezivanje programa i dretve

#dretve mogu biti u listi
#L=[]
#L.append(potpr,(1,2))
#L.append(...)

#for i in range(len(L)):L[i].start() ili for d in L: d.start()
#for d in L: d.join()

import threading as thr
from time import sleep
"""
#pr 1 koristenje dretvi nad funkcijama bez parametara
def parni():
    for i in range(2,11,2):
        print(i)
        sleep(1)
    print("parni gotovi")
    
def neparni():
    for i in range(1,11,2):
        print(i)
        sleep(2)
    print("neparni gotovi")

if __name__=="__main__":
    t1=thr.Thread(target=parni)
    t1.start()
    t2=thr.Thread(target=neparni)
    t2.start()
    t1.join()
    t2.join()
"""
#primjer 1a: koristenje dretvi i parametri funkcija
def brojevi(poc,cekaj):
    for i in range(poc,11,2):
        print(f"{i}\n",end="")
        sleep(cekaj)
    print(f"gotovo s {poc}{cekaj}\n",end="")
# if __name__=="__main__":
#     t1=thr.Thread(target=brojevi,args=(2,1))
#     t1.start()
#     t2=thr.Thread(target=brojevi,args=(1,2))
#     t2.start()
#     t1.join()
#     t2.join()


#primjer 1b: koristenje dretvi i paramteri funkcije ali dretve u listama(flex obrada)
if __name__=="__main__":
    L=[]
    for i in range(2):
        t=thr.Thread(target=brojevi,args=(2-i,i+1))
        L.append(t)
    for d in L:
        d.start()
    for d in L:
        d.join()
        