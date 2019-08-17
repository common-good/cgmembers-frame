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
  And usd transfers:
  | payee | amount | completed |*
  | .ZZA  |    400 | %today-2m |  
  | .ZZB  |    100 | %today-2m |  
  | .ZZC  |    300 | %today-2m |  
  Then balances:
  | uid  | balance |*
  | .ZZA |     400 |
  | .ZZB |     100 |
  | .ZZC |     300 |
  When transactions: 
  | xid | created   | amount | from | to   | purpose |*
  |   4 | %today-9d |     10 | .ZZB | .ZZA | cash E  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     410 |
  | .ZZB |      90 |
  | .ZZC |     300 |
  When transactions: 
  | xid | created   | amount | from | to   | purpose |*
  |   5 | %today-8d |    100 | .ZZC | .ZZA | usd F   |
  Then balances:
  | uid  | balance |*
  | .ZZA |     510 |
  | .ZZB |      90 |
  | .ZZC |     200 |
  When transactions: 
  | xid | created   | amount | from | to    | purpose |*
  |   6 | %today-7d | 240.01 | .ZZA | .ZZB  | what G  |
  |   6 | %today-7d |    .99 | .ZZA | round | roundup donation |
  # pennies here and below, to trigger roundup contribution
  Then balances:
  | uid   | balance |*
  | round |    0.99 |
  | .ZZA  |  269.00 |
  | .ZZB  |  330.01 |
  | .ZZC  |  200.00 |
  When transactions: 
  | xid | created   | amount | from | to    | purpose |*
  |   7 | %today-6d |  99.99 | .ZZA | .ZZB  | pie N   |
  |   7 | %today-6d |   0.01 | .ZZA | round | roundup donation |
  Then balances:
  | uid  | balance |*
  | .ZZA |     169 |
  | .ZZB |     430 |
  | .ZZC |     200 |
  When transactions: 
  | xid | created   | amount | from | to   | purpose |*
  |   8 | %today-5d |    100 | .ZZC | .ZZA | labor M |
  Then balances:
  | uid  | balance |*
  | .ZZA |     269 |
  | .ZZB |     430 |
  | .ZZC |     100 |
  When transactions: 
  | xid | created   | amount | from | to   | purpose |*
  |   9 | %today-4d |     50 | .ZZB | .ZZC | cash P  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     269 |
  | .ZZB |     380 |
  | .ZZC |     150 |
  # A: (21*(100+400) + 110+400 + 130+480 + 92+280 + -3+280 + 2*(107+280) + 3*(31+140))/30 * R/12 = 
  When transactions: 
  | xid | created   | amount | from | to   | purpose |*
  |  10 | %today-3d |    120 | .ZZA | .ZZC | this Q  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     149 |
  | .ZZB |     380 |
  | .ZZC |     270 |
  When transactions: 
  | xid | created   | amount | from | to   | purpose |*
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
#  Then transactions: 
#  | xid| created| type      | amount | bonus                               | from | to   | purpose |*
#  | 12 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*29.2, 2)) | ctty | .ZZA | %IAOY average balance |
#  | 13 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*13.6, 2)) | ctty | .ZZB | %IAOY average balance |
#  | 14 | %today | inflation |      0 | %(round(%R_INFLATION_RATE*22.8, 2)) | ctty | .ZZC | %IAOY average balance |

Scenario: Paper statement warnings are sent
  When cron runs "everyMonth"
  # alerting admin about paper statements
  Then we tell admin "Send paper statements" with subs:
  | list |*
  | Corner Pub (Cvil) |

Scenario: Crumb and roundup donations are made
  When cron runs "everyMonth"
  Then transactions: 
  | xid | created        | amount | from  | to    | purpose                                      | flags       |*
  | 12  | %(%daystart-1) |   2.40 | .ZZC  | crumb | crumbs donation: 2.0% of past month receipts | gift,crumbs |
  | 13  | %(%daystart-1) |   1.00 | round | cgf   | roundup donations                            | gift        |
  | 14  | %(%daystart-1) |   2.40 | crumb | cgf   | crumb donations                              | gift        |
  # Note that tests simulate the previous month as the previous 30 days (created field is mdt1-1 when not testing)
  And count "txs" is 14
  And count "invoices" is 0

  When cron runs "everyMonth"
  Then count "txs" is 14
  And count "invoices" is 0
  # still
  
Scenario: Crumbs are invoiced
  Given transactions:
  | xid | created   | amount | from | to   | purpose |*
  |  12 | %today-4d |    770 | .ZZC | .ZZB | loan    |
  Then count "txs" is 12
  When cron runs "everyMonth"
  Then invoices:
  | nvid | created        | payer | payee | amount | flags       | purpose                                      | status       |*
  |    1 | %(%daystart-1) | .ZZC  | crumb |   2.40 | gift,crumbs | crumbs donation: 2.0% of past month receipts | %TX_APPROVED |
  And transactions:
  | xid | created        | amount | from  | to  | purpose           | flags |*
  | 13  | %(%daystart-1) |   1.00 | round | cgf | roundup donations | gift  |
  And count "txs" is 13
  And count "invoices" is 1

  When cron runs "everyMonth"
  Then count "txs" is 13
  And count "invoices" is 1
  
  Given transactions:
  | xid | created   | amount | from | to   | purpose |*
  |  14 | %today-4d |    770 | .ZZB | .ZZC | repay   |
  Then count "txs" is 14

  When cron runs "everyMonth"
  Then count "txs" is 14
  And count "invoices" is 1  

  When cron runs "invoices"
  Then transactions:
  | xid | created | amount | from | to    | purpose                                                       | flags       |*
  | 15  | %now    |   2.40 | .ZZC | crumb | crumbs donation: 2.0% of past month receipts (%PROJECT inv#1) | gift,crumbs |
  And count "txs" is 15

  When cron runs "invoices"
  Then count "txs" is 15
  
# NO (Seedpack gets no distribution) distribution of shares to CGCs
#  And transactions:
#  | xid | created | amount | from | to   | flags |*
#  |  20 |       ? |   2.20 |  cgf | ctty | gift  |
