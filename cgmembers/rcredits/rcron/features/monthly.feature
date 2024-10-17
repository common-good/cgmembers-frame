Feature: Monthly
AS a member
I WANT various monthly automatic account calculations and transactions
SO I can support the Common Good System and be supported by it.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags       | crumbs | city |*
  | .ZZA | Abe One    | -500  | personal    | ok,roundup  |      0 | Avil |
  | .ZZB | Bea Two    | -500  | personal    | ok,co       |      0 | Bvil |
  | .ZZC | Corner Pub | -500  | corporation | ok,co,paper |   0.02 | Cvil |
  And these "txs2":
  | payee | amount | completed | deposit   |*
  | .ZZA  |    400 | %today-2m | %today-2m |
  | .ZZB  |    100 | %today-2m | %today-2m |  
  | .ZZC  |    300 | %today-2m | %today-2m |
  Then balances:
  | uid  | balance |*
  | .ZZA |     400 |
  | .ZZB |     100 |
  | .ZZC |     300 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   4 | %today-9d |     10 | .ZZB | .ZZA | cash E  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     410 |
  | .ZZB |      90 |
  | .ZZC |     300 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   5 | %today-8d |    100 | .ZZC | .ZZA | usd F   |
  Then balances:
  | uid  | balance |*
  | .ZZA |     510 |
  | .ZZB |      90 |
  | .ZZC |     200 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   6 | %today-7d | 240.01 | .ZZA | .ZZB  | what G  |
  |   6 | %today-7d |    .99 | .ZZA | round | roundup donation |
  # pennies here and below, to trigger roundup contribution
  Then balances:
  | uid   | balance |*
  | round |    0.99 |
  | .ZZA  |  269.00 |
  | .ZZB  |  330.01 |
  | .ZZC  |  200.00 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   7 | %today-6d |  99.99 | .ZZA | .ZZB  | pie N   |
  |   7 | %today-6d |   0.01 | .ZZA | round | roundup donation |
  Then balances:
  | uid  | balance |*
  | .ZZA |     169 |
  | .ZZB |     430 |
  | .ZZC |     200 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   8 | %today-5d |    100 | .ZZC | .ZZA | labor M |
  Then balances:
  | uid  | balance |*
  | .ZZA |     269 |
  | .ZZB |     430 |
  | .ZZC |     100 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   9 | %today-4d |     50 | .ZZB | .ZZC | cash P  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     269 |
  | .ZZB |     380 |
  | .ZZC |     150 |
  # A: (21*(100+400) + 110+400 + 130+480 + 92+280 + -3+280 + 2*(107+280) + 3*(31+140))/30 * R/12 = 
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |  10 | %today-3d |    120 | .ZZA | .ZZC | this Q  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     149 |
  | .ZZB |     380 |
  | .ZZC |     270 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |  11 | %today-1d |    100 |  .ZZA | .ZZB | cash V  |
  Then balances:
  | uid  | balance |*
  | .ZZA |      49 |
  | .ZZB |     480 |
  | .ZZC |     270 |

# no inflation at present
#Scenario: Inflation adjustments are made
#  When cron runs "everyMonth"

# inflation  
#  Then these "txs": 
#  | xid| created| type      | amount | bonus                               | payer | payee | purpose |*
#  | 12 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*29.2, 2)) | ctty | .ZZA | %IAOY average balance |
#  | 13 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*13.6, 2)) | ctty | .ZZB | %IAOY average balance |
#  | 14 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*22.8, 2)) | ctty | .ZZC | %IAOY average balance |

Scenario: Paper statement warnings are sent
  When cron runs "everyMonth"
  # alerting admin about paper statements
  Then we tell admin "Send paper statements" with subs:
  | list              |*
  | Corner Pub (Cvil) |

Scenario: Crumb and roundup donations are made
  When cron runs "everyMonth"
  Then these "txs": 
  | xid | created        | amount | payer | payee | purpose                                      | flags           |*
  | 12  | %(%daystart-1) |   2.40 | .ZZC  | crumb | crumbs donation: 2.0% of past month receipts | thx,gift,crumbs |
  | 13  | %(%daystart-1) |   1.00 | round | cgf   | roundup donations: %mY                       | gift            |
  | 14  | %(%daystart-1) |   2.40 | crumb | cgf   | crumb donations: %mY                         | gift            |
  # Note that tests simulate the previous month as the previous 30 days (created field is monthDt1-1 when not testing)
  And count "tx_hdrs" is 14
  And count "tx_requests" is 0

  When cron runs "everyMonth"
  Then count "tx_hdrs" is 14
  And count "tx_requests" is 0
  # still
  
Scenario: Crumbs are invoiced
  Given these "txs":
  | xid | created   | amount | payer | payee | purpose |*
  |  12 | %today-4d |    770 | .ZZC | .ZZB | loan    |
  Then count "tx_hdrs" is 12
  When cron runs "everyMonth"
  Then count "tx_requests" is 1
  And these "tx_requests":
  | nvid | created        | payer | payee | amount | flags               | purpose                                      | status       |*
  |    1 | %(%daystart-1) | .ZZC  | crumb |   2.40 | gift,crumbs,funding | crumbs donation: 2.0% of past month receipts | %TX_APPROVED |
  And these "txs2":
  | xid | payee | amount | completed | deposit |*
  | 13  | .ZZC  |   2.40 | %now      |       0 |
  And these "txs":
  | xid | created        | amount | payer | payee | purpose                | flags |*
  | 13  | %now           |   2.40 | bank  | .ZZC  | from bank              |       |
  | 14  | %(%daystart-1) |   1.00 | round | cgf   | roundup donations: %mY | gift  |
  And count "tx_hdrs" is 14

  When cron runs "everyMonth"
  Then count "tx_hdrs" is 14
  And count "tx_requests" is 1
  
  Given these "txs":
  | xid | created   | amount | payer | payee | purpose |*
  |  15 | %today-4d |    770 | .ZZB | .ZZC | repay   |
  Then count "tx_hdrs" is 15

  When cron runs "everyMonth"
  Then count "tx_hdrs" is 15
  And count "tx_requests" is 1  

  When cron runs "payInvoices"
  And cron runs "getFunds"
  And cron runs "completeUsdTxs"
  Then count "tx_hdrs" is 16
  And these "txs":
  | xid | created | amount | payer | payee | purpose                                      | flags           |*
  | 16  | %now    |   2.40 | .ZZC  | crumb | crumbs donation: 2.0% of past month receipts | thx,gift,crumbs |

  When cron runs "payInvoices"
  And cron runs "getFunds"
  And cron runs "completeUsdTxs"
  Then count "tx_hdrs" is 16
  
# NO (Seedpack gets no distribution) distribution of shares to CGCs
#  And these "txs":
#  | xid | created | amount | payer | payee | flags |*
#  |  20 |       ? |   2.20 |  cgf | ctty | gift  |
