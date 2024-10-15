Feature: Gifts
AS a member
I WANT my recent requested donation to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | country | phone      | postalAddr | flags               | bankAccount | floor |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01001 | US      | 4132530001 | 1 A, A, AK | ok,confirmed,bankOk | USkk9000001 |   -20 |
  | .ZZB | Bea Two  | 2 B St. | Btown | PA    | 01002 | US      | 4132530002 | 2 B, B, BC | ok,confirmed,admin  |             |  -200 |
  And these "txs":
  | xid | created   | amount | payer | payee | purpose |*
  |   1 | %today-4m |    100 | .ZZB | .ZZA | loan    |
  And member ".ZZB" has admin permissions: "seeAccts"

Scenario: A donation to CG is visible to admin
  Given these "tx_timed":
  | action | start  | from | to  | amount | period | purpose |*
  | pay    | %today | .ZZA | cgf |     10 | week   | gift!   |
  When member "A:B" visits page ""
  Then we show "Summary" with:
  | Donations: | $10 Weekly |

Scenario: A brand new recurring donation to CG can be completed
  Given members:
  | .ZZC | Cor Pub  | 3 C St. | Ctown | CA    | 01003 | US      | 4132530003 | 3 C, C, CA | ok,confirmed,co    |             |  -200 |
  And these "tx_timed":
  | id | action | start      | from | to   | amount | period | purpose |*
  |  7 | pay    | %yesterday | .ZZA | .ZZC |     10 | month  | gift!   |
  When cron runs "payInvoices"
  Then these "txs":
  | xid | created | amount | payer | payee    | purpose | flags     | recursId |*
  |   2 | %today  |     10 | .ZZA  | regulars | gift!   | gift,self |        7 |
  And we message "paid you linked" to member ".ZZC" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | gift!        | ?        |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01001 |
  | ~footer | %PROJECT |
  And we message "recur pay" to member ".ZZA" with subs:
  | amount | when    | purpose | payee    |*
  |    $10 | monthly | gift!   | %PROJECT |
  # and many other fields
  And we email "cggift-thanks-member" to member ".ZZA" with subs:
  | amount | $10 monthly |**
  | date   | %mdY |
  And we email "gift-report" to member ".ZZC" with subs:
  | amount      | $10 monthly |**
  | date        | %mdY |
  | fromName    | Abe One |
  | fromAddress | 1 A, A, AK |
  | fromEmail   | a@example.com |
  | fromPhone   | +1 413 253 0001 |
  | note        | |
  And count "txs" is 2
  And count "txs2" is 0
  And count "tx_requests" is 0
  When it's later
  And cron runs "recurs"
  Then count "txs" is 2
  And count "txs2" is 0
  And count "tx_requests" is 0

Scenario: A brand new recurring donation to CG can be completed
  Given these "tx_timed":
  | id | action | start      | from | to  | amount | period | purpose |*
  |  7 | pay    | %yesterday | .ZZA | cgf |     10 | month  | gift!   |
  When cron runs "recurs"
  Then these "txs":
  | xid | created | amount | payer | payee    | purpose | flags         | recursId |*
  |   2 | %today  |     10 | .ZZA  | regulars | gift!   | thx,gift,self |        7 |
  And we message "recur pay" to member ".ZZA" with subs:
  | amount | when    | purpose | payee    |*
  |    $10 | monthly | gift!   | %PROJECT |
  # and many other fields
  And we email "cggift-thanks-member" to member ".ZZA" with subs:
  | amount | $10 monthly |**
  | date   | %mdY |
  And count "txs" is 2
  And count "txs2" is 0
  And count "tx_requests" is 0
  When it's later
  And cron runs "recurs"
  Then count "txs" is 2
  And count "txs2" is 0
  And count "tx_requests" is 0

Scenario: A second recurring donation to CG can be completed
  Given these "tx_timed":
  | id | action | start     | from | to  | amount | period | purpose |*
  | 1  | pay    | %today-3m | .ZZA | cgf |     10 | month  | gift!   |
  And these "txs":
  | xid | created    | amount | payer | payee    | purpose | flags     | recursId |*
  |   2 | %today-32d |     10 | .ZZA  | regulars | gift!   | gift,self | 1        |
  When cron runs "recurs"
  Then these "txs":
  | xid | created | amount | payer | payee    | purpose | flags         |*
  |   3 | %today  |     10 | .ZZA  | regulars | gift!   | thx,gift,self |
  And count "txs" is 3

  Given it's later
  When cron runs "recurs"
  Then count "txs" is 3

Scenario: A donation invoice (to CG) can be completed
# even if the member has never yet made a cgCard purchase
  Given these "tx_timed":
  | id | action | start      | from | to  | amount | period | purpose |*
  |  8 | pay    | %yesterday | .ZZA | cgf |     10 | month  | gift!   |
  And these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for      | flags     | recursId |*
  |    2 | %today    | %TX_APPROVED |     50 | .ZZA  | cgf   | donation | gift,self |        8 |
  And member ".ZZA" has no photo ID recorded
  When cron runs "payInvoices"
  Then these "txs": 
  | xid | created | amount | payer | payee    | purpose  | flags         | recursId |*
  |   2 | %today  |     50 | .ZZA  | regulars | donation | thx,gift,self |        8 |
  And these "tx_requests":
  | nvid | created   | status | purpose  |*
  |    2 | %today    | 2      | donation |

Scenario: A recurring donation to CG cannot be completed
  Given these "tx_timed":
  | action | start     | from | to  | amount | period | purpose |*
  | pay    | %today-3m | .ZZA | cgf |    200 | month  | gift!   |
  When cron runs "recurs"
  Then only these "tx_requests":
  | nvid | created   | status       | amount | payer | payee | for   | flags             |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA  | cgf   | gift! | self,gift,funding |  
  And only these "txs2":
  | txid | xid | amount | payee |*
  | 1    | 2   | 200    | .ZZA  |
  And count "txs" is 2
  And these "txs":
  | xid | created | amount | payer | payee | purpose   |*
  |   2 | %today  | 0      | bank  | .ZZA  | from bank |

  When cron runs "payInvoices"
  And cron runs "getFunds"
  And cron runs "completeUsdTxs"
  Then count "txs" is 2
  And count "txs2" is 1
  And count "tx_requests" is 1
  # (no change)
  
  Given it's later
  When cron runs "recurs"
  Then count "txs" is 2
  And count "txs2" is 1
  And count "tx_requests" is 1
  # (no change)

Scenario: A non-member chooses a donation to CG
  Given members:
  | uid  | fullName | flags  | bankAccount | activated | balance |*
  | .ZZD | Dee Four |        | USkk9000004 |         0 |       0 |
  | .ZZE | Eve Five | refill | USkk9000005 | %today-9m |     200 |
  And these "tx_timed":
  | id | action | start     | from | to  | amount | period | purpose  |*
  | 2  | pay    | %today-3y | .ZZD | cgf |      1 | year   | donation |
  | 3  | pay    | %today-3m | .ZZE | cgf |    200 | month  | donation |
  When cron runs "recurs"
  Then count "txs" is 1
  And count "txs2" is 0
  And count "tx_requests" is 0
