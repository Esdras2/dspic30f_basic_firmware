# dsPIC30F4011‑UART‑ADC‑Demo
Minimal example: **UART2 @ 115 200 bps + ADC(AN0) + Heartbeat LED** on the *dsPIC30F4011*.

---

## ✨ Features

* **Clock**: Internal FRC × 8 → Fosc ≈ 58.96 MHz, FCY ≈ 14.74 MHz
* **UART2**: 8‑N‑1, 115 200 bps (default)
* **ADC**: Single‑shot on AN0, 10‑bit result
* **Packet format**: `0xAA 0x55 [High] [Low]` every 100 ms
* **Heartbeat**: RD0 toggles each loop (Timer TODO)

## 📂 Repo structure

```
├── src/
│   └── main.c        # Firmware source
├── include/          # Optional headers
├── .gitignore        # XC16, MPLAB X, build artefacts
├── LICENSE           # MIT
└── README.md         # You are here
```

## 🛠️ Hardware

| Pin | Signal | Notes                         |
| --- | ------ | ----------------------------- |
| RF5 | U2TX   | Connect to USB‑TTL RX         |
| RF4 | U2RX   | Connect to USB‑TTL TX         |
| RD0 | LED    | Heartbeat (330 Ω → LED → GND) |
| AN0 | ADC    | Pot, sensor, etc.             |
| 5 V | Vdd    | 5 V regulated                 |
| GND | GND    | Common ground                 |

> **Tip:** Change PPS if your board exposes different UART pins.

## 🚀 Quick start

```bash
# 1. Clone
$ git clone https://github.com/<you>/dspic30f4011-uart-adc-demo.git
$ cd dspic30f4011-uart-adc-demo

# 2. Open with MPLAB X (Device: dsPIC30F4011)
#    or build from CLI:
$ xc16-gcc src/main.c -mcpu=30F4011 -o build/main.elf

# 3. Flash (PICkit 4, ICD3, etc.)
$ mplab_ipe_cli --device=30F4011 --MCHPFSUSB --hex=build/main.hex

# 4. Monitor serial
$ screen /dev/ttyUSB0 115200
```

### Prerequisites

* MPLAB X IDE ≥ 6.10 or XC16 ≥ v3.01
* Programmer/debugger (PICkit‑3/4, ICD3‑4, Snap)
* USB‑TTL adapter (3 V/5 V)

## ⚙️ Configuration

| Macro / Symbol | Purpose         | Default    |
| -------------- | --------------- | ---------- |
| `BAUD_RATE`    | UART baud       | `115200UL` |
| `ADC_CHANNEL`  | AN index        | `0`        |
| `SAMPLING_TAD` | Tad count       | `10`       |
| `BRGVAL`       | Auto‑calculated | —          |

Edit these in `src/main.c` and rebuild.

## 📡 Packet format

```
AA 55 HH LL
└┬┘ └┬┘ └─┬──┘
 │   │    └─── 10‑bit ADC value (high ↑, low ↓)
 │   └──────── Start word (0x55)
 └──────────── Start word (0xAA)
```

Consumers (Python, MATLAB, etc.) can sync on `0xAA 0x55`.

## 📝 Roadmap

* [ ] Move sampling to **Timer1 ISR**
* [ ] Add **DMA** UART TX
* [ ] Provide **Python plotter** example
* [ ] CI build via **GitHub Actions** (XC16 Docker)

## 🤝 Contributing

PRs and issues are welcome! Please fork the repo, create a topic branch (`feat/<name>`), and submit a pull request.

## 📜 License

This project is released under the MIT License – see [`LICENSE`](LICENSE) for details.

---

> *Happy hacking & may your bits never flip!*
