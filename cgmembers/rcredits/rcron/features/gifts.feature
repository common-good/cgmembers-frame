Feature: Gifts
AS a member
I WANT my recent requested donation to CGF to go through
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | country | postalAddr | flags               | bankAccount | floor |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01001 | US      | 1 A, A, AK | ok,confirmed,bankOk | USkk9000001 |   -20 |
  | .ZZB | Bea Two  | 2 B St. | Btown | PA    | 01002 | US      | 2 B, B, BC | ok,confirmed,cAdmin |             |  -200 |
  And transactions:
  | xid | created   | amount | payer | payee | purpose |*
  |   1 | %today-4m |    100 | .ZZB | .ZZA | loan    |

Scenario: A donation to CG is visible to admin
  Given these "tx_templates":
  | start  | from | to  | amount | period | purpose |*
  | %today | .ZZA | cgf |     10 | week   | gift!   |
  When member "A:B" visits page ""
  Then we show "Summary" with:
  | Donations: | $10 weekly |

Scenario: A brand new recurring donation to CG can be completed
  Given these "tx_templates":
  | id | start      | from | to  | amount | period | purpose |*
  |  7 | %yesterday | .ZZA | cgf |     10 | month  | gift!   |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose| flags       | recursId |*
  |   2 | %today  |     10 | .ZZA  | cgf   | gift!   | gift,recurs |        7 |
  And we notice "new payment linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | gift!        | ?        |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01001 |
  | ~footer | %PROJECT |
  And we notice "recur pay" to member ".ZZA" with subs:
  | amount | when    | purpose | payee    |*
  |    $10 | monthly | gift!   | %PROJECT |
  # and many other fields
  And count "txs" is 2
  And count "usd" is 0
  And count "tx_requests" is 0
  When cron runs "recurs"
  Then count "txs" is 2
  And count "usd" is 0
  And count "tx_requests" is 0

Scenario: A second recurring donation to CG can be completed
  Given these "tx_templates":
  | start     | from | to  | amount | period | purpose |*
  | %today-3m | .ZZA | cgf |     10 | month  | gift!   |
  And transactions:
  | xid | created    | amount | payer | payee | purpose | flags       |*
  |   1 | %today-32d |     10 | .ZZA  | cgf   | gift!   | gift,recurs |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose | flags          |*
  |   2 | %today  |     10 | .ZZA  | cgf   | gift!   | gift,recurs |

Scenario: A donation invoice (to CG) can be completed
# even if the member has never yet made a cgCard purchase
  Given these "tx_templates":
  | id | start      | from | to  | amount | period | purpose |*
  |  8 | %yesterday | .ZZA | cgf |     10 | month  | gift!   |
  And invoices:
  | nvid | created   | status       | amount | payer | payee | for      | flags       | recursId |*
  |    2 | %today    | %TX_APPROVED |     50 | .ZZA | cgf | donation | gift,recurs |        8 |
  And member ".ZZA" has no photo ID recorded
  When cron runs "getFunds"
  Then transactions: 
  | xid | created | amount | payer | payee | purpose  | flags       | recursId |*
  |   2 | %today  |     50 | .ZZA  | cgf   | donation | gift,recurs |        8 |
  And invoices:
  | nvid | created   | status | purpose  |*
  |    2 | %today    | 2      | donation |

Scenario: A recurring donation to CG cannot be completed
  Given these "tx_templates":
  | start     | from | to  | amount | period | purpose |*
  | %today-3m | .ZZA | cgf |    200 | month  | gift!   |
  When cron runs "recurs"
  Then invoices:
  | nvid | created   | status       | amount | payer | payee | for   | flags          |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA  | cgf   | gift! | gift,recurs |  
  And count "txs" is 2
  And count "usd" is 1
  # because invoice generated a bank transfer
  And count "tx_requests" is 1

  When cron runs "getFunds"
  Then count "txs" is 2
  And count "usd" is 1
  And count "tx_requests" is 1
  # (no change)
  
  When cron runs "recurs"
  Then count "txs" is 2
  And count "usd" is 1
  And count "tx_requests" is 1
  # (no change)

Scenario: A non-member chooses a donation to CG
  Given members:
  | uid  | fullName | flags  | bankAccount | activated | balance |*
  | .ZZD | Dee Four |        | USkk9000004 |         0 |       0 |
  | .ZZE | Eve Five | refill | USkk9000005 | %today-9m |     200 |
  And these "tx_templates":
  | id | start     | from | to  | amount | period | purpose  |*
  | 2  | %today-3y | .ZZD | cgf |      1 | year   | donation |
  | 3  | %today-3m | .ZZE | cgf |    200 | month  | donation |
  When cron runs "recurs"
  Then count "txs" is 1
  And count "usd" is 0
  And count "tx_requests" is 0
