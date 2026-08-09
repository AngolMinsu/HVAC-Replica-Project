# CAN Protocol

| ID | 용도 |
|---:|---|
| `0x100` | Control Request |
| `0x101` | Control Response / Gateway ACK |
| `0x300` | HVAC Status Broadcast |

```text
D0 Service | D1 Result | D2 Signal | D3 Value
D4 Option  | D5 Reserved | D6 Counter | D7 XOR Checksum
```

```text
D7 = D0 ^ D1 ^ D2 ^ D3 ^ D4 ^ D5 ^ D6
```

주요 Signal:

| Signal | 의미 |
|---:|---|
| `0x02` | FAN_SPEED |
| `0x03` | Driver Temperature |
| `0x04` | HVAC_MODE / Wind |
| `0x0A` | VOLUME |
| `0x0C` | Passenger Temperature |
| `0x13` / `0x14` | HU Focus Prev / Next |
| `0x15` / `0x16` / `0x17` | HU Open Home / Map / Media |

Gateway ACK는 Gateway 수신 확인. 반대 노드 반영 완료는 `0x22` 또는 `0x32` Response로 판단.
