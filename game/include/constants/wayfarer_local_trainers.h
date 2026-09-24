#ifndef GUARD_CONSTANTS_WAYFARER_LOCAL_TRAINERS_H
#define GUARD_CONSTANTS_WAYFARER_LOCAL_TRAINERS_H

// 1723-1735 is reserved for Celadon Hideout; 1736 stays vacant for merging.
#define TRAINER_SILPH_GRUNT_23_HNS                         1737
#define TRAINER_SILPH_GRUNT_24_HNS                         1738
#define TRAINER_SILPH_GRUNT_25_HNS                         1739
#define TRAINER_SILPH_GRUNT_26_HNS                         1740
#define TRAINER_SILPH_GRUNT_27_HNS                         1741
#define TRAINER_SILPH_GRUNT_28_HNS                         1742
#define TRAINER_SILPH_GRUNT_29_HNS                         1743
#define TRAINER_SILPH_GRUNT_30_HNS                         1744
#define TRAINER_SILPH_GRUNT_31_HNS                         1745
#define TRAINER_SILPH_GRUNT_32_HNS                         1746
#define TRAINER_SILPH_GRUNT_33_HNS                         1747
#define TRAINER_SILPH_GRUNT_34_HNS                         1748
#define TRAINER_SILPH_GRUNT_35_HNS                         1749
#define TRAINER_SILPH_GRUNT_36_HNS                         1750
#define TRAINER_SILPH_GRUNT_37_HNS                         1751
#define TRAINER_SILPH_GRUNT_38_HNS                         1752
#define TRAINER_SILPH_GRUNT_39_HNS                         1753
#define TRAINER_SILPH_GRUNT_40_HNS                         1754
#define TRAINER_SILPH_GRUNT_41_HNS                         1755
#define TRAINER_SILPH_SCIENTIST_BEAU_HNS                   1756
#define TRAINER_SILPH_SCIENTIST_CONNOR_HNS                 1757
#define TRAINER_SILPH_SCIENTIST_ED_HNS                     1758
#define TRAINER_SILPH_SCIENTIST_JERRY_HNS                  1759
#define TRAINER_SILPH_SCIENTIST_JOSE_HNS                   1760
#define TRAINER_SILPH_SCIENTIST_JOSHUA_HNS                 1761
#define TRAINER_SILPH_SCIENTIST_PARKER_HNS                 1762
#define TRAINER_SILPH_SCIENTIST_RODNEY_HNS                 1763
#define TRAINER_SILPH_SCIENTIST_TAYLOR_HNS                 1764
#define TRAINER_SILPH_SCIENTIST_TRAVIS_HNS                 1765
#define TRAINER_SILPH_DALTON_HNS                           1766
#define TRAINER_SILPH_GIOVANNI_HNS                         1767
#define TRAINER_MT_MOON_ROCKET_GRUNT_1_HNS                 1768
#define TRAINER_MT_MOON_ROCKET_GRUNT_2_HNS                 1769
#define TRAINER_MT_MOON_ROCKET_GRUNT_3_HNS                 1770
#define TRAINER_MT_MOON_ROCKET_GRUNT_4_HNS                 1771
#define TRAINER_MT_MOON_MIGUEL_HNS                         1772
#define TRAINER_NUGGET_BRIDGE_CALE_HNS                     1773
#define TRAINER_NUGGET_BRIDGE_ALI_HNS                      1774
#define TRAINER_NUGGET_BRIDGE_TIMMY_HNS                    1775
#define TRAINER_NUGGET_BRIDGE_RELI_HNS                     1776
#define TRAINER_NUGGET_BRIDGE_ETHAN_HNS                    1777
#define TRAINER_NUGGET_BRIDGE_ROCKET_HNS                   1778

#define TRAINER_WAYFARER_LOCAL_FIRST 1737
#define TRAINER_WAYFARER_LOCAL_LAST 1778
#define TRAINER_WAYFARER_LOCAL_COUNT 42
#define WAYFARER_LOCAL_DEFEAT_SLOT_FIRST 661
#define WAYFARER_LOCAL_DEFEAT_FLAG_FIRST (TRAINER_FLAGS_START + WAYFARER_LOCAL_DEFEAT_SLOT_FIRST)
#if IS_WAYFARER
#undef TRAINERS_COUNT_WAYFARER
#define TRAINERS_COUNT_WAYFARER 1779
#endif

#endif // GUARD_CONSTANTS_WAYFARER_LOCAL_TRAINERS_H
