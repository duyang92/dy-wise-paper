import pandas as pd
data=pd.read_excel("~/Desktop/xxnw地址.xlsx",sheet_name="内网")
result=open("address.txt","w")
for line in data.values:
    city=line[0][:-2]
    ip=line[1].split("/")[0]
    mask=line[1].split("/")[1]
    result.write(city+" "+ip+" "+mask+"\n")