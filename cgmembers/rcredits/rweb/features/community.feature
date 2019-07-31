Feature: Statistics
AS a member
I WANT accurate, up-to-date system statistics
SO I can see how well the rCredits system is doing for myself, for my ctty, and for the world.
#   r floor rewards usd minimum maximum 
#   signup rebate bonus inflation grant loan fine maxRebate
#   balance (-r) demand (minimum - r)

Setup:
  Given members:
  | uid  | fullName   | flags | jid   | minimum | floor | created   | activated |*
  | .ZZA | Abe One    | ok    | 0     |       5 |     0 | %today-6m | %today-5m |
  | .ZZB | Bea Two    | ok    | .ZZD  |    1000 |   -20 | %today-5w | %today-4w |
  | .ZZC | Corner Pub | ok,co | 0     |    2000 |    10 | %today-4w | %today-3w |
  | .ZZD | Dee Four   | ok    | .ZZB  |    1000 |   -20 | %today-5w | %today-4w |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  | .ZZB | .ZZD  | joint      |
  | .ZZD | .ZZB  | joint      |
  And recurs:
  | id | created   | ended | amount | payer | payee | period | purpose |*
  |  1 | %today-8m |     0 |     12 | .ZZC  | .AAB  |      Y | gift    |
  And usd transfers:
  | txid | payee | amount | created    | completed  |*
  |  100 | .ZZA  |   1000 | %today-20d | %today-13d |
  |  101 | .ZZB  |   2000 | %today-21d | %today-14d |
  |  102 | .ZZC  |   3050 | %today-22d | %today-15d |
  |  103 | .ZZC  |    -50 | %today-12d | %today-12d |
  # txs #1,2,3,4
  Then balances:
  | uid  | balance |*
  | ctty |       0 |
  | .ZZA |    1000 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  | .ZZD |    2000 |
  Given transactions: 
  | xid | created   | amount | from | to   | purpose | goods      |*
  |   6 | %today-3m |     10 | .ZZB | .ZZA | cash E  | %FOR_USD   |
  |   7 | %today-3m |    100 | .ZZC | .ZZA | usd F   | %FOR_USD   |
  |   8 | %today-3m |    240 | .ZZA | .ZZB | what G  | %FOR_GOODS |
#  And statistics get set "%tomorrow-1m"
  And transactions: 
  | xid | created   | amount | from | to   | purpose | goods      | channel  | flags  |*
  |  15 | %today-2w |     50 | .ZZB | .ZZC | p2b     | %FOR_GOODS | %TX_WEB  |        |
  |  18 | %today-1w |    120 | .ZZA | .ZZC | this Q  | %FOR_GOODS | %TX_WEB  |        |
  |  23 | %today-6d |    100 | .ZZA | .ZZB | real V  | %FOR_GOODS | %TX_WEB  |        |
  |  27 | %today-2d |      4 | ctty | .ZZA | grant   | %FOR_GOODS | %TX_WEB  |        |
  |  28 | %today-2d |      5 | ctty | .ZZD | loan    | %FOR_USD   | %TX_WEB  |        |
  |  29 | %today-2d |     -6 | ctty | .ZZC | fine    | %FOR_GOODS | %TX_WEB  |        |
  |  30 | %today-1d |    100 | .ZZC | .ZZA | payroll | %FOR_GOODS | %TX_WEB  |        |
  |  33 | %today-1d |      1 | .ZZC | .AAB | gift    | %FOR_GOODS | %TX_CRON | recurs,gift |
  Then balances:
  | uid  | balance |*
  | ctty |   -3.00 |
  | .ZZA |  754.00 |
  | .ZZB | 2285.00 |
  | .ZZC | 2963.00 |
  | .ZZD | 2285.00 |
  | .AAB |    1.00 |

Scenario: cron calculates the statistics
# Many of the following statistics exclude the community itself, so balance may differ from usd
#  When cron runs "acctStats"
  Given statistics get set "%daystart-30d"
  When cron runs "cttyStats"
  Then these "stats":
  | id           |         7 |**
  | ctty         |      ctty |
  | created      | %daystart |
  | pAccts       |         3 |
  | bAccts       |         3 |
  | newbs        |         0 |
  | aAccts       |         4 |
  | conx         |         3 |
  | conxLocal    |         3 |
  | balsPos      |   6003.00 |
  | balsNeg      |      0.00 |
  | balsPosCount |         4 |
  | balsNegCount |         0 |
  | topN         |   6002.00 |
  | botN         |   3040.00 |
  | floors       |    -10.00 |
  | p2b          |    170.00 |
  | b2b          |      7.00 |
  | b2p          |    104.00 |
  | p2p          |    340.00 |
  | p2bCount     |         2 |
  | b2bCount     |         2 |
  | b2pCount     |         2 |
  | p2pCount     |         2 |
  | cashs        |    115.00 |
  | cashsCount   |         3 |
  | cgIn         |      0.00 |
  | cgOut        |      0.00 |
  | cgInCount    |         0 |
  | cgOutCount   |         0 |
  | usdIn        |   6050.00 |
  | usdOut       |    -50.00 |
  | usdInCount   |         3 |
  | usdOutCount  |         1 |
  | payees       |      1.25 |
  | basket       |     48.25 |
  | patronage    |      1.00 |
  | roundups     |      0.00 |
  | crumbs       |      0.00 |
  | invites      |         0 |
 
  When member ".ZZA" visits page "community/graphs"
  Then we show "Statistics" with:
#  | Community: | Seedpack |
# was 2 co, but that included the community, which is not activated
  |~Success: | 0.04 |
  |~CG Growth: | 3 members + 3 co |
  |~Dollar Pool: | $6,000 |
#  |~CG | $6,002 |
  |~Circulation Velocity: | 6.4% per mo. |
  |~Monthly Bank Transfers | $6,000 (net) |
  |~Monthly Transactions | 8 @ $48.25 |
# 2 members and 2 companies -- including CGF
  
#  | Accounts        | 5 (3 personal, 2 companies) — up 5 from a month ago |
#  | rCredits issued | $835.90r — up $835.90r from a month ago |
#  | | signup: $750r, inflation adjustments: $6r, rebates/bonuses: $70.90r, grants: $4r, loans: $5r, fees: $-6r |
#  | Demand          | $5,999.75 — up $5,999.75 from a month ago |
#  | Total funds     | $835.90r + $5,999.75us = $6,835.65 |
#  | | including about $6,564.65 in savings = 109.4% of demand (important why?) |
#  | Banking / mo    | $6,050us (in) - $50us (out) - $0.25 (fees) = +$5,999.75us (net) |
#  | Purchases / mo  | 4 ($271) / mo = $54.20 / acct |
#  | p2p             | 0 ($0) / mo = $0 / acct |
#  | p2b             | 2 ($170) / mo = $56.67 / acct |
#  | b2b             | 1 ($1) / mo = $0.50 / acct |
#  | b2p             | 1 ($100) / mo = $50 / acct |
#  | Velocity        | 4.0% per month |
