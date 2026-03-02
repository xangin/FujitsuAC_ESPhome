# FujitsuAC ESPHome Custom Component

富士通冷氣（Fujitsu AC）ESPHome custom component，透過 UART 與室內機通訊，整合至 Home Assistant。

---

## ⚠ 重要安全警告

**切勿將冷氣機上的任何接腳連接到外部設備（例如電腦）。**
若將冷氣 GND 與筆電 GND 接觸，可能燒毀筆電 USB 埠及/或冷氣主板保險絲。
冷氣接腳並非電氣隔離，其電壓並非相對於大地 GND。

---

## 硬體需求（零件清單）

| 零件 | 說明 |
|------|------|
| DC/DC 降壓模組（12V → 5V）| **選購 5V 輸出版本**，電壓選錯會燒毀冷氣 UART 介面 |
| ESP32（30 pin）| 原作者使用標準 ESP32 Dev Module |
| JST 4P 連接器（10cm/20cm）| 對應冷氣端 UTY-TFSXW1 的 4 pin 接頭 |
| 萬用板（4×6 cm）| 需稍微裁切 |
| 邏輯電位轉換器（Logic Level Converter）| 用於 3.3V ↔ 5V 訊號轉換 |

---

## 接線方式

### JST 型連接器接線（4 pin，多數壁掛機型）

```
冷氣插座        CN3903（DC-DC）           ESP32
Pin 1 (+12V) → V in+ → V out+ ────────→ 5V
Pin 2 (GND)  → V in- → V out- ────────→ GND
Pin 3 (DATA) ─────────────────────────→ RX / GPIO16（冷氣 → ESP）
Pin 4 (DATA) ─────────────────────────→ TX / GPIO17（ESP  → 冷氣）
```

腳位順序由左至右為 1 2 3 4。

### USB 型連接器接線（4 pin，適用 UTY-TFSXF3 機型）

```
USB 型插座（由上至下）：
Pin 1 → 12V
Pin 2 → AC_TX → 接 ESP32 RX / GPIO16
Pin 3 → AC_RX → 接 ESP32 TX / GPIO17
Pin 4 → GND
```

> 📌 **注意**：富士通 UART 訊號為**反向邏輯**，需在 YAML 設定 `inverted: true`，或使用外接 NPN 電晶體反向電路。

---

## 安裝

1. 將本 `components/` 目錄放置於 ESPHome `yaml` 同一層目錄
2. 在 YAML 中加入：
   ```yaml
   external_components:
     - source:
         type: local
         path: components
       components: [fujitsu_ac]
   ```
3. 參考 `fujitsu_ac_sample.yaml` 進行設定

---

## 通訊協定

- **鮑率**：9600 bps，8N1
- **訊號**：反向 TTL（需 inverted RX/TX）
- **格式**：自訂二進位封包（2-byte 地址 + 2-byte 值，16-bit checksum）
- **週期**：每 400ms 輪詢一次，順序為 Init1 → Init2 → InitRegs1/2/3 → FrameA / FrameB / FrameC

---

## 支援功能

| 功能 | 說明 |
|------|------|
| 開關 | Power on/off |
| 模式 | Auto / Cool / Heat / Dry / Fan |
| 目標溫度 | 16～30°C（步進 0.5°C；Heating 最低 16°C，其他 18°C） |
| 風速 | Auto / Quiet / Low / Medium / High |
| 垂直風向 | Position 1～6 + Swing |
| 水平風向 | Position 1～6 + Swing（需機型支援） |
| 室內溫度 | ActualTemp 暫存器（0.1°C 解析度） |
| 戶外溫度 | OutdoorTemp 暫存器（0.1°C 解析度） |

---

## 檔案結構

```
FujitsuAC/
├── components/
│   └── fujitsu_ac/
│       ├── __init__.py              # ESPHome component 定義
│       ├── fujitsu_ac.h             # 主 Climate 類別
│       ├── fujitsu_controller.h     # 通訊控制器（含 Buffer）
│       ├── fujitsu_enums.h          # 列舉定義
│       ├── fujitsu_register.h       # 暫存器與地址定義
│       └── fujitsu_registry_table.h # 暫存器查詢表
├── fujitsu_ac_sample.yaml           # 範例 YAML 設定
└── README.md                        # 本說明文件
```

---

## 參考資料

- 原始 FujitsuAC Arduino 程式庫：[github.com/Benas09/FujitsuAC](https://github.com/Benas09/FujitsuAC)
- ESPHome Climate 文件：[esphome.io/components/climate/](https://esphome.io/components/climate/)
