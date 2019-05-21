Feature: Transactions
AS a member
I WANT to review my transactions
SO I can see what happened, accept or refuse offers, adjust descriptions, and correct errors.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags                | created    |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup,confirmed | %today-15m |
  | .ZZB | Bea Two    | -200  | personal    | ok,co                | %today-15m |
  | .ZZC | Corner Pub | -300  | corporation | ok,co                | %today-15m |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And usd transfers:
  | payee | amount | created    | completed  |*
  |  .ZZA |   1000 | %today-13m | %today-13m |
  |  .ZZB |   2000 | %today-13m | %today-13m |
  |  .ZZC |   3000 | %today-13m | %today-13m |
  |  .ZZA |     11 | %today-3d  |         0  |
  |  .ZZA |    -22 | %today-5d  | %today-5d  |
  |  .ZZA |    -33 | %today-5d  | %today-5d  |
  Then balances:
  | uid  | balance |*
  | .ZZA |     945 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  Given transactions: 
  | xid | created   | amount | from | to   | purpose | taking | payerTid | payeeTid |*
  |  14 | %today-5m |     10 | .ZZB | .ZZA | cash E  | 0      |       24 |       34 |
  |  15 | %today-4m |   1100 | .ZZC | .ZZA | usd F   | 1      |       25 |       35 |
  |  16 | %today-3m |    240 | .ZZA | .ZZB | what G  | 0      |       26 |       36 |
  |  17 | %today-2w |     50 | .ZZB | .ZZC | cash P  | 0      |       27 |       37 |
  |  18 | %today-1w |    120 | .ZZA | .ZZC | this Q  | 1      |       28 |       38 |
  |  19 | %today-6d |    100 | .ZZA | .ZZB | cash V  | 0      |       29 |       39 |
  Then balances:
  | uid  | balance |*
  | .ZZA |    1595 |
  | .ZZB |    2280 |
  | .ZZC |    2070 |

Scenario: A member looks at transactions for the past year
  Given members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365"
  Then we show "Transaction History" with:
  | Start        |   | 1,000.00 | %dmy-12m |
  | From Bank    | + |     0.00 | + 11.00 Pending |
  | Received     | + | 1,110.00 |          |
  | Out          | - |   515.00 |          |
#  | Credit Line+ |   |          |          |
  | End          |   | 1,595.00 | %dmy     |
  And with:
  |~tid | Date    | Name          | Purpose          | Amount   |  Balance | Action |
  |  3  | %mdy-5d | --            | transfer to bank |   -33.00 | 1,595.00 |        |
  |  2  | %mdy-5d | --            | transfer to bank |   -22.00 | 1,628.00 |        |
  | 29  | %mdy-6d | Bea Two       | cash V           |  -100.00 | 1,650.00 |        |
  | 28  | %mdy-1w | Corner Pub    | this Q           |  -120.00 | 1,750.00 |        |
  | 26  | %mdy-3m | Bea Two       | what G           |  -240.00 | 1,870.00 |        |
  | 35  | %mdy-4m | Corner Pub    | usd F            | 1,100.00 | 2,110.00 |        |
  | 34  | %mdy-5m | Bea Two       | cash E           |    10.00 | 1,010.00 |        |
#  | 1   | %mdy-7m | ZZrCred       | signup           |     0.00 |      .00 |        |
  And without:
  | rebate  |
  | bonus   |

Scenario: A member looks at transactions for the past few days
  When member ".ZZA" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Start        |   | 1,870.00 | %dmy-15d |
  | From Bank    | + |     0.00 | + 11.00 Pending |
  | Received     | + |     0.00 |          |
  | Out          | - |   275.00 |          |
#  | Credit Line+ | + |          |          |
  | End          |   | 1,595.00 | %dmy     |
  And with:
  |~tid | Date    | Name          | Purpose          | Amount  |  Balance |
  |  3  | %mdy-5d | --            | transfer to bank |  -33.00 | 1,595.00 |
  |  2  | %mdy-5d | --            | transfer to bank |  -22.00 | 1,628.00 |
  | 29  | %mdy-6d | Bea Two       | cash V           | -100.00 | 1,650.00 |
  | 28  | %mdy-1w | Corner Pub    | this Q           | -120.00 | 1,750.00 |
  And without:
  | pie N    |
  | whatever |
  | usd F    |
  | cash E   |
  | signup   |
  | rebate   |
  | bonus    |

Scenario: A member looks at transactions with roundups
  Given tx headers:
  | xid | actorId | actorAgentId | flags | channel | goods      | created |*
  |  21 |    .ZZC | .ZZC         |       | %TX_POS | %FOR_GOODS | %today  |
  And tx entries:
  | xid | entryType | amount | uid              | agentUid | description      | acctTid |*
  |  21 |         1 | -50.00 | .ZZA             | .ZZA     | sundries         |      51 |
  |  21 |         2 |  49.95 | .ZZC             | .ZZC     | sundries         |      61 |
  |  21 |         0 |   0.05 | %CG_ROUNDUPS_UID | .ZZA     | roundup donation |      71 |
  # | xid | created | type     | amount | from | to   | purpose  | payerTid | payeeTid |*
  # |  10 | %today  | transfer |  49.95 | .ZZA | .ZZC | sundries |       40 |       50 |
  Then balances:
  | uid  | balance |*
  | .ZZA | 1545.00 |
  When member ".ZZA" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Start        |   | 1,870.00 | %dmy-15d |
  | From Bank    | + |     0.00 | + 11.00 Pending |
  | Received     | + |     0.00 |          |
  | Out          | - |   325.00 |          |
#  | Credit Line+ | + |          |          |
  | End          |   | 1,545.00 | %dmy     |
  And with:
  |~tid | Date    | Name              | Purpose          | Amount  |  Balance |~do |
  | 51  | %mdy    | Corner Pub        | sundries         |  -49.95 | 1,545.00 |    |
  |     |         | Roundup Donations | roundup donation |   -0.05 |          |    | 
  |  3  | %mdy-5d | --                | transfer to bank |  -33.00 | 1,595.00 |    |
  |  2  | %mdy-5d | --                | transfer to bank |  -22.00 | 1,628.00 |    |
  | 29  | %mdy-6d | Bea Two           | cash V           | -100.00 | 1,650.00 |    |
  | 28  | %mdy-1w | Corner Pub        | this Q           | -120.00 | 1,750.00 |    |
  
#Scenario: Transactions with other states show up properly
#  Given transactions:
#  | xid   | created   | state    | amount | from | to   | purpose  | taking |*
#  | .AACA | %today-5d | denied   |    100 | .ZZC | .ZZA | labor CA | 0      |
#  | .AACB | %today-5d | denied   |      5 | ctty | .ZZC | 0      |
#  | .AACC | %today-5d | denied   |     10 | ctty | .ZZA | 0      |
#  | .AACD | %today-5d | denied   |      5 | .ZZA | .ZZC | cash CE  | 1      |
#  | .AACE | %today-5d | disputed |     80 | .ZZA | .ZZC | this CF  | 1      |
#  | .AACF | %today-5d | disputed |      4 | ctty | .ZZA | 0      |
#  | .AACG | %today-5d | disputed |      8 | ctty | .ZZC | 0      |
#  | .AACH | %today-5d | deleted  |    200 | .ZZA | .ZZC | never    | 1      |
#  | .AACK | %today-5d | disputed |    100 | .ZZC | .ZZA | cash CL  | 1      |
#  Then balances:
#  | uid  |    r |*
#  | .ZZA | 1942 |
#  | .ZZB | 2554 |
#  | .ZZC | 2320 |
#  When member ".ZZA" visits page "history/transactions/period=5"
#  Then we show "Transaction History" with:
#  |~tid | Date   | Name       | From you | To you | Status   | ~  | Purpose    | Reward/Fee |
#  | 15  | %dm-5d | Corner Pub | --       | 100.00 | disputed | X  | cash CL    | --     |
#  | 13  | %dm-5d | Corner Pub | 80.00    | --     | disputed | OK | this CF    | 4.00   |
#  | 11  | %dm-5d | Corner Pub | --       | 100.00 | denied   | X  | labor CA   | 10.00  |
#  | b4  | %dm-5d |            |  22.00   | --     | pending  |    | to bank    | --     |
#  | b3  | %dm-5d |            |  33.00   | --     | pending  |    | to bank    | --     |
#  # 12 is missing because ZZA denied it
#  And without:
#  | cash CE |
#  | never   |
#  | rebate  |
#  | bonus   |
#  When member ".ZZC" visits page "history/transactions/period=5"
#  Then we show "Transaction History" with:
#  |~tid | Date   | Name       | From you | To you | Status   | ~  | Purpose    | Reward/Fee |
#  | 10  | %dm-5d | Abe One    | 100.00   | --     | disputed | OK | cash CL    | --     |
#  | 8   | %dm-5d | Abe One    | --       | 80.00  | disputed | X  | this CF    | 8.00   |
#  | 7   | %dm-5d | Abe One    | --       | 5.00   | denied   | X  | cash CE    | --     |
#  And without:
#  | labor CA|
#  | never   |
#  | rebate  |
#  | bonus   |
