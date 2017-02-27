# dsoffload
delay-sensitive offloading algorithm

已完成功能
- 產生UE/AP(uniform distribution)，用txt儲存
- minT: 為UE尋找T最小的BS來加入
- delay-sensitive offloading
====
- 測試AP密度

預期新增之功能
- 對照組(隨機、SINR)
--------------------------------------------------
- 產生packet實際模擬UE的資料傳送
- 計算評估效能的參數(Throughput, capacity, delay)
- 輸出結果

已知問題
- rho(lambda/mu)會大於1，導致T計算出來有負值
- 程式執行時間過長(當depth>1)
