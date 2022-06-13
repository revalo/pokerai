# PokerAI

Attempt to build a strong 6-player poker bot for Texas Holdem No-Limit Poker.

## Building

Monorepo should have everything. Cmake should just work anywhere, cross-platform.

pkmeans instructions are in `pai/third-party/pkmeans`.

## Usage

### Abstraction Precomputation

Generate histograms for hands,

```
Release\generate_distributions.exe --round flop --num_samples 1000 --output_file flop_dist_1000.dat
```

Cluster them,
