Feature: Risk
AS a Common Good Community Administrator or Member or Compliance Officer at a partnering financial institution
I WANT to know what financial or regulatory risks each account, transaction, and ACH pose
SO I can handle those risks appropriately

Setup:
  Given members:
  | uid  | fullName   | address | zip   | flags      | risks    | tenure | moves |*
  | .ZZA | Abe One    | 1 A St. | 01001 | ok         | adminOk  | 21     | 0     |
  | .ZZB | Bea Two    | 2 A St. | 01001 | ok         | rents    | 43     | 1     |
  | .ZZC | Corner Pub | 3 C St. | 01003 | ok,co      | cashCo   | 18     |       |
  | .ZZD | Dee Four   | 3 C St. | 01003 | ok         | hasBank  | 25     | 0     |
  | .ZZE | Eve Five   | 5 A St. | 01001 | ok         | shady    | 1      | 0     |
  | .ZZF | Flo Six    | 6 A St. | 01001 | ok,roundup |          | 32     | 0     |
  | .ZZG | Guy Seven  | 7 A St. | 01001 | ok         | addrOff  | 11     | 5     |
  | .ZZH | Hal Eight  | 8 A St. | 01001 | ok         | ssnOff   | 100    | 10    |
  | .ZZI | Ida Nine   | 9 A St. | 01001 | ok         | fishy    | 3      | 20    |
  And invites:
  | inviter | invitee | email |*
  | .ZZA    | .ZZD    | d2@   |
  | .ZZA    |    0    | e@    |
  | .ZZG    | .ZZH    | h2@   |
  | .ZZG    | .ZZI    | i@    |
  And proxies:
  | person | proxy | priority |*
  | .ZZA   | .ZZB  |        1 |
  | .ZZA   | .ZZD  |        2 |
  | .ZZB   | .ZZD  |        1 |
  | .ZZB   | .ZZA  |        2 |
  | .ZZD   | .ZZA  |        1 |
  | .ZZD   | .ZZB  |        2 |
  And relations:
  | main | agent | permission | employee | owner | draw |*
  | .ZZC | .ZZA  | scan       |        Y |     0 |    0 |
  | .ZZC | .ZZB  |            |          |       |      |
  | .ZZC | .ZZD  |            |          |       |      |
  And these "recurs":
  | payer | payee | amount | period |*
  | .ZZA  |   cgf |     10 |     Y |
  | .ZZB  |   cgf |      5 |     Y |
  | .ZZC  |   cgf |      1 |     Y |
  | .ZZD  |   cgf |      1 |     Q |
  | .ZZE  |   cgf |      1 |     M |
  | .ZZF  |   cgf |      1 |     Y |
  | .ZZG  |   cgf |      5 |     Y |
  | .ZZH  |   cgf |      5 |     Y |
  | .ZZI  |   cgf |      5 |     Y |
# share is irrelevant here as long as it is non-negative
  And transactions: 
  | xid | created   | amount | from | to   | purpose | channel |*
  |   1 | %today-7m |    250 | ctty | .ZZA | signup  | %TX_SYS |
  |   2 | %today-6m |    250 | ctty | .ZZB | signup  | %TX_SYS |
  |   3 | %today-6m |    250 | ctty | .ZZE | signup  | %TX_SYS |
  |   4 | %today-5m |     10 | .ZZB | .ZZA | cash E  | %TX_POS |
  |   5 | %today-1m |   1100 | .ZZA | .ZZC | USD in (cash) | %TX_POS |
  # (cash) is required else a transaction fee transaction is created
  |   6 | %today-3w |    240 | .ZZA | .ZZB | what G  | %TX_POS |

  |   7 | %today-2w |     50 | .ZZB | .ZZC | cash P  | %TX_POS |
  |   8 | %today-1w |    120 | .ZZA | .ZZH | offline | %TX_POS |

  |   9 | %today-6d |    100 | .ZZA | .ZZB | cash V  | %TX_WEB |
  |  10 | %today-1d |    120 | .ZZA | .ZZC | undoneBy:17 | %TX_POS |
  |  11 | %today-1d |   -120 | .ZZA | .ZZC | undoes:14 | %TX_POS |
  |  12 | %today-1d |     40 | .ZZC | .ZZE | labor   | %TX_WEB |
  |  13 | %today-1d |     10 | .ZZF | .ZZE | cash    | %TX_WEB |
  |  14 | %today-1d |     11 | .ZZF | .ZZE | cash    | %TX_WEB |  
  And usd transfers:
  | txid | payee | amount | completed |*
  |    1 | .ZZA  |    400 | %today-2m |  
  |    2 | .ZZB  |    100 | %today-2m |  
  |    3 | .ZZC  |    300 | %today-2m |  
  |    4 | .ZZE  |    200 | %today    |  
  |    5 | .ZZF  |   -600 | %today    |  
  |    6 | .ZZC  |   -500 | %today    |
  And member field values:
  | uid  | field      | value |*
  | .ZZA | community  |    -2 |
  | .ZZE | postalAddr | Box 5 |
# don't set community to -2 until after transactions  
  And riskThresholds:
  | Day | Week | 7Week | Year |*
  | 300 |  600 |  1200 | 2400 |

  When cron runs "trust"
  Then members have:
  | uid  | trust |*
  | .ZZA |  8.57 |
  | .ZZB |  8.57 |
  | .ZZD |  8.57 |
  | .ZZE |     1 |
  | .ZZF |     1 |
  | .ZZG |     1 |
  | .ZZH |     1 |
  | .ZZI |     1 |

Scenario: We calculate risks
  When cron runs "acctRisk"
  Then members have:
  | uid  | risks |*
  | .ZZE | new,shady,poBox,moreIn |
  | .ZZI | new,moves,fishy |
  | .ZZH | moves,ssnOff |
  | .ZZC | cashCo,homeCo,miser,bigDay,bigWeek,big7Week |
#  | .ZZC | cashCo,homeCo,miser,bigDay | (this happens sometimes but is WRONG: 1270>1200 and 660>600)
  | .ZZG | new,moves,badConx,addrOff |
  | .ZZF | bigDay,bigWeek |
  | .ZZB | trusted,moves,rents,moreIn,moreOut |
  | .ZZA | adminOk,trusted,geography,badConx,moreOut,big7Week |
  | .ZZD | trusted,hasBank,miser |
# Do not specify exact risk because minor tweaks in the calculations cause major changes

  Given riskThresholdPercent is "10"
  When cron runs "acctRiskFinish"
  Then riskThresholds:
  | Day | Week | 7Week | Year |*
  | 621 |  621 |  1270 | 1570 |
#  | 330 |  340 |   635 |  785 |

  When cron runs "txRisk"
  Then transactions:
  | xid | risks |*
  |   1 | |
  |   2 | |
  |   3 | |
  |   4 | exchange,p2p |
  |   5 | cashIn,inhouse,toSuspect,biggestFrom,biggestTo |
  |   6 | p2p,biggestTo |
  |   7 | cashOut,toSuspect,biggestFrom |
  |   8 | p2p,toSuspect,biggestTo,offline,firstOffline |
  |   9 | exchange,p2p,absent,invoiceless,bigFrom,bigTo |
  |  10 | inhouse,toSuspect,oftenFrom,oftenTo |
  |  11 | fromSuspect,biggestFrom,biggestTo,origins |
  |  12 | b2p,fromSuspect,toSuspect,absent,invoiceless,biggestTo,origins |
  |  13 | exchange,p2p,fromSuspect,toSuspect,absent,invoiceless,bigFrom,bigTo,suspectOut |
  |  14 | exchange,p2p,fromSuspect,toSuspect,absent,invoiceless,biggestFrom,suspectOut |
  When cron runs "achRisk"
  Then usd transfers:
  | txid | payee | risks |*
  |    1 |  .ZZA | |
  |    2 |  .ZZB | |
  |    3 |  .ZZC | |
  |    4 |  .ZZE | toSuspect |
  |    5 |  .ZZF | toBank,suspectOut |
  |    6 |  .ZZC | toBank,origins,suspectOut |
