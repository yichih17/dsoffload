# dsoffload
delay-sensitive offloading algorithm

已完成功能
- 產生UE/AP(uniform distribution)，用txt儲存
- 為UE尋找T最小的BS來加入
  1.尋找UE可連接的BS
  2.試算加入BS後，BS的T
  3.選擇T最小的加入(T一樣比Capacity)
  
已知問題
- rho(lambda/mu)會大於1，導致T計算出來有負值

預期新增之功能
- 計算UE加入後，會影響的UE數量
- 加入dsoffload演算法
- 對照組(不同K值、SINR)
--------------------------------------------------
- 產生packet實際模擬UE的資料傳送
- 計算評估效能的參數(Throughput, capacity, delay)
- 輸出結果