# Audio mix range / 2.0.21

Default mix strength is100%. These are native-bus signal gains, not perceived-loudness percentages. Beds and Chants share one curve.

| Mapped positions | Fullness | Background / chants | Collective reactions | Individual detail |
| ---: | ---: | ---: | ---: | ---: |
| 6,060 | 0% | Muted | Muted | Muted |
| 6,060 | 25% | 50.0% | 62.5% | 95.8% |
| 6,060 | 30% | 53.9% | 65.5% | 96.2% |
| 6,060 | 70% | 100.0% | 100.0% | 100.0% |
| 6,060 | 100% | 115.0% | 115.0% | 115.0% |
| 61,888 | 0% | Muted | Muted | Muted |
| 61,888 | 25% | 92.9% | 94.7% | 99.4% |
| 61,888 | 30% | 100.0% | 100.0% | 100.0% |
| 61,888 | 70% | 118.2% | 118.2% | 118.2% |
| 61,888 | 100% | 158.5% | 130.0% | 130.0% |

Nonempty floors are40% background/chants,55% collective reactions and95% individual detail. The smallest venue at25% fullness uses50% background signal. The smooth crossover moves from70% fullness at6060 positions to30% at61888. Exactly zero attendance explicitly mutes the Crowd parent.

Dense Beds/Chants/StudentSection can rise toward+3.5dB, and up to+4dB in the largest packed crowd. Collective reactions retain their separate+30% signal ceiling. The native excitement limiter and separate chant cut are removed. Current gains use a.001 signal lattice with synchronized native fades to limit insignificant preset updates.

Individual detail is the existing Sweeteners group; changing its gain does not prove which fan voice will play. The steady background roar is a separate sustained-channel policy, not another bus-gain multiplier. See the [code-measured facts](releases/Update3-Code-Facts.json) and [full feature list](../FEATURES.md).
