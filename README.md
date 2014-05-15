台灣政治獻金資料處理-圖檔的文字定位,擷取,辨識

1. 影像角度歪斜校正(default:true)
2. table 欄位擷取
3. 影像上下顛倒判斷
4. 去除雜訊(default:false)
5. 文字定位
6. 辨識
7. json file 輸出
   
   (inverse:影像是否上下相反, theta:傾斜角度, left_top:左上角座標, right_down:右下角座標, result:辨識結果)
   

執行方法 : 

1. 執行檔要和 database 資料夾放在同一個路徑
2. ./CharRecognition 'imagePath'

   (如果要關掉歪斜校正後面加上 "-s")
   (如果要啟動去除雜訊後面加上 "-d")

- 有些圖檔看起來是因為想要減少浮水印, 所以掃描的圖檔顏色比較淡, 這時候就不需要去 denoise
  如果 denoise 會造成文字斷裂反而辨識結果更糟, 目前去 denoise default 為 false
  如果要用要另外去啟動
  
- table 欄位定位需要影像是"正的", 否則會定位失敗, 所以歪斜校正 default 為 true

欄位的定位不是使用線偵測再找交點的方式, 是直接去分析欄位, 所以速度會比較快
但是由於圖檔很大, 影響到定位的準確度, 所以欄位座標會有些 pixel 的誤差
在有些數字靠近欄位邊緣因為上述定位的誤差造成文字切割錯誤然後辨識也跟著錯
這部份之後要再解決

授權方式 MIT: http://g0v.mit-license.org


