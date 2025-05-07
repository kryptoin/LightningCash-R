# LightningCash Reborn

## What Is LightningCash-R?

LightningCash-R (LNCR) is a CPU-minable cryptocurrency based on a previous project called LightningCash (LNC), which itself was a clone of Litecoin Cash (LCC) which is a fork of Litecoin (LTC) that is a fork of Bitcoin (BTC). It introduces major protocol changes, including:

* A new Proof-of-Work (PoW) algorithm: YespowerLTNCG
* An advanced difficulty adjustment system based on DarkGravityWave v3
* Integration of the Hive mining system from LitecoinCash
* A new genesis block and a snapshot-based balance claim system for holders of the original LNC

## Key Features

### Proof-of-Work Algorithm

* Uses the YespowerLTNCG PoW algorithm (CPU-only)
* Enhanced with a modified version of Dash's DarkGravityWave v3

  * Difficulty adjusts every block
  * Difficulty can drop within a single block if a stale tip is detected
  * Designed to resist hash rate manipulation and protect against high variance attacks

### Hive Mining System

* Operates in parallel with PoW mining
* Allows users to create "bees" which generate Hive-mined blocks
* Hive and PoW blocks occur at approximately a 50/50 ratio (since Hive 1.1)

#### Bee Lifecycle:

* Maturation: 1152 blocks (\~3 hours 21 minutes)
* Lifespan after maturation: 16,128 blocks (\~2 days)

#### Hive Economics:

* Initially, 90–100% of the bee creation fee is burned
* After January 1, 2025, 50% of the fee is redirected to a community fund

### Network Parameters

* Average block time: 10.5 seconds (nominal time is 20s, Hive lowers the effective time)
* Confirmations required: 6 (\~1 minute)
* Block reward: 1.25 LNCR
* Halving interval: Every 8,400,000 blocks (\~2 years, 9 months, 17 days)

## Supply and Emissions

### Initial Distribution

* Maximum theoretical supply: 91,000,000 LNCR

  * 70,000,000 LNCR allocated at genesis:

    * 66,414,845 LNCR for LNC balance claims (accounts under 100 LNC excluded)
    * 1,682,694 LNCR for old exchange swap
    * 1,902,461 LNCR for premine
* Mineable supply: 21,000,000 LNCR

### Coin Burning

* As of December 14, 2024:

  * 74,634,033.75 LNCR emitted
  * 1,766,134.75 LNCR burned
  * Actual total supply: 72,867,899 LNCR
  * Burn rate: approximately 38.11% of emitted coins

* After January 1, 2025, the burn rate is expected to drop to approximately 20%

### Supply Timeline (Adjusted for Burns)

| Date              | Circulating Supply | % of Max Supply | % of Mineable Coins Mined |
| ----------------- | ------------------ | --------------- | ------------------------- |
| January 1, 2025   | 72,989,468         | 84.95%          | 18.77%                    |
| July 10, 2026     | 77,525,089         | 90.22%          | 47.25%                    |
| April 26, 2029    | 81,725,089         | 95.11%          | 73.63%                    |
| February 10, 2032 | 83,825,089         | 97.56%          | 86.81%                    |
| November 27, 2034 | 84,875,089         | 98.78%          | 93.41%                    |
| October 21, 2088  | 85,925,089         | 100%            | 100%                      |

## Halving Schedule (Corrected Estimates)

| Block Range          | Reward (LNCR) | Emission Period             | Total LNCR Emitted |
| -------------------- | ------------- | --------------------------- | ------------------ |
| 8 to 8,399,999       | 1.25          | Sep 23, 2023 – Jul 10, 2026 | 10,500,000         |
| 8,400,000–16,799,999 | 0.625         | Jul 10, 2026 – Apr 26, 2029 | 5,250,000          |
| 16.8M–25.2M          | 0.3125        | Apr 26, 2029 – Feb 10, 2032 | 2,625,000          |
| 25.2M–33.6M          | 0.15625       | Feb 10, 2032 – Nov 27, 2034 | 1,312,500          |
| 33.6M–42M            | 0.078125      | Nov 27, 2034 – Sep 13, 2037 | 656,250            |
| 42M–50.4M            | 0.0390625     | Sep 13, 2037 – Jun 30, 2040 | 328,125            |
| 50.4M–58.8M          | 0.01953125    | Jun 30, 2040 – Apr 17, 2043 | 164,062.5          |
| ...                  | ...           | ...                         | ...                |
| Final Halving        | -             | Until Oct 21, 2088          | Total 21,000,000   |

## Developer Notes

* DarkGravityWave v3 implementation adapted from Dash:

  * Original: [https://github.com/dashpay/dash/blob/master/src/pow.cpp#L82](https://github.com/dashpay/dash/blob/master/src/pow.cpp#L82)
  * Modified: [https://github.com/MerlinMagic2018/LightningCash-R/blob/master/src/pow.cpp#L110](https://github.com/MerlinMagic2018/LightningCash-R/blob/master/src/pow.cpp#L110)
* LightningCash-R Core is the full-node software for the LNCR peer-to-peer network

## Conclusion

LightningCash-R is a uniquely engineered cryptocurrency combining cutting-edge difficulty adjustment, dual mining systems (PoW and Hive), and a fair emission schedule. Its robust design targets network stability, accessibility, and long-term sustainability. With active coin burning, community funding, and predictable supply dynamics, LightningCash-R aims to be a resilient and inclusive digital currency for the future.