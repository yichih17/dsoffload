#Delay-sensitive offloading  

##目前完成功能  

####1. 以Uniform distribution生成UE和AP  

####2. UE連接BS的機制  

- minT  
- delay-sensitive offloading  

####3. 結果輸出    
- 無法連接BS的UE數量  
- 所有BS的平均delay、load、連接UE數、底下UE的平均capacity  
- 所有UE的平均capacity、delay  

##待完成功能  

####1. UE連接BS的機制
- UE隨機選擇BS
- UE根據SINR大小選擇BS  

####2. Packet level simulation(為了算throughput)  

##只知問題  

####1. M/G/1公式在當Rho>=1時，得到的T值會是負值
- (解決)  當rho>=1時，拒絕UE加入  

####2. 承上，rho有可能超出電腦小數點範圍，而被當成1
- (解決)  把條件改為當rho>=0.9999時，拒絕UE加入
