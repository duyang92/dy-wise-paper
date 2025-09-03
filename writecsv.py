import sys
import pandas as pd
sizelist=[10000,20000,40000,60000,80000]
result=open("result.txt","r")
alllines = result.readlines()
i=0
while i<len(alllines):
    sketchname=alllines[i].strip()
    dataset=alllines[i+1].strip()
    are=alllines[i+2].strip()
    mae=alllines[i+3].strip()
    querytime=alllines[i+4].strip()
    insertth=alllines[i+5].strip()
    memory=alllines[i+6].strip() 
    idx=sizelist.index(int(sys.argv[1]))
    arecsv=pd.read_csv("plot/"+dataset+"-"+"are.csv")
    arecsv.loc[idx,sketchname]=are
    maecsv=pd.read_csv("plot/"+dataset+"-"+"mae.csv")
    maecsv.loc[idx,sketchname]=mae
    querytimecsv=pd.read_csv("plot/"+dataset+"-"+"query.csv")
    querytimecsv.loc[idx,sketchname]=querytime
    insertthcsv=pd.read_csv("plot/"+dataset+"-"+"th.csv")
    insertthcsv.loc[idx,sketchname]=insertth
    memcsv=pd.read_csv("plot/"+dataset+"-"+"mem.csv")
    arecsv.to_csv("plot/"+dataset+"-"+"error.csv",index=False)
    maecsv.to_csv("plot/"+dataset+"-"+"mae.csv",index=False)
    querytimecsv.to_csv("plot/"+dataset+"-"+"query.csv",index=False)
    insertthcsv.to_csv("plot/"+dataset+"-"+"th.csv",index=False)
    memcsv.loc[idx,sketchname]=memory
    memcsv.to_csv("plot/"+dataset+"-"+"mem.csv",index=False)
    i+=7




