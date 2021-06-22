Feature: Recurs
AS a member
I WANT to make recurring payments (typically gifts) to other members
SO I can save on memory and labor.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | country | postalAddr | flags               | bankAccount | floor |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01000 | US      | 1 A, A, AK | ok,confirmed,bankOk | USkk9000001 |   -20 |
  | .ZZB | Bea Two  | 2 B St. | Btown | PA    | 01002 | US      | 2 B, B, BC | ok,confirmed,cAdmin |             |  -200 |
  | .ZZC | Cor Pub  | 3 C St. | Ctown | CT    | 03000 | US      | 3 C, C, CT | ok,co,confirmed     |             |     0 |
  And transactions:
  | xid | created   | amount | payer | payee | purpose |*
  |   1 | %today-4m |    100 | .ZZB | .ZZA | loan    |

Scenario: A brand new recurring payment can be completed
  Given these "tx_templates":
  | action | start      | from | to   | amount | period | purpose |*
  | pay    | %yesterday | .ZZA | .ZZB |     10 | week   | pmt     |
  When cron runs "recurs"
  Then transactions:
  | xid | created | amount | payer | payee | purpose | flags  |*
  |   2 | %today  |     10 | .ZZA  | .ZZB  | pmt     | recurs |
  And we notice "new payment" to member ".ZZB" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | pmt          | ?        |
  And we notice "recur pay" to member ".ZZA" with subs:
  | amount | when   | purpose | payee   |*
  |    $10 | weekly | pmt     | Bea Two |
  # and many other fields
  And count "txs" is 2
  And count "usd" is 0
  And count "tx_requests" is 0
  When cron runs "recurs"
  Then count "txs" is 2
  And count "usd" is 0
  And count "tx_requests" is 0

Scenario: A second recurring payment can be completed
  Given these "tx_templates":
  | id | start     | from | to   | amount | period | purpose |*
  |  8 | %today-8d | .ZZA | .ZZB |     10 | week   | pmt     |
  And transactions:
  | xid | created   | amount | payer | payee | purpose | flags  | recursId |*
  |   2 | %today-8d |     10 | .ZZA  | .ZZB  | pmt     | recurs |        8 |
  When cron runs "recurs"
  Then transactions:
  | xid | created   | amount | payer | payee | purpose | flags  | recursId |*
  |   3 | %today-1d |     10 | .ZZA  | .ZZB  | pmt     | recurs |        8 |

Scenario: A recurring payment happened yesterday
  Given these "tx_templates":
  | id | action | start      | from | to   | amount | period | purpose |*
  |  8 | pay    | %yesterday | .ZZA | .ZZC |     10 | month  | pmt     |
  And transactions:
  | xid | created    | amount | payer | payee | purpose | flags  | recursId |*
  |   2 | %yesterday |     10 | .ZZA  | .ZZC  | pmt     | recurs |        8 |
  When cron runs "recurs"
  Then count "txs" is 2
  
Scenario: A recurring payment happened long enough ago to repeat
  Given these "tx_templates":
  | id | start         | from | to   | amount | period | purpose |*
  |  8 | %yesterday-1w | .ZZA | .ZZC |     10 | week   | pmt     |
  And transactions:
  | xid | created       | amount | payer | payee | purpose | flags  | recursId |*
  |   2 | %yesterday-1w |     10 | .ZZA  | .ZZC  | pmt     | recurs |        8 |
  When cron runs "recurs"
  Then transactions:
  | xid | created    | amount | payer | payee | purpose | flags  | recursId |*
  |   3 | %yesterday |     10 | .ZZA  | .ZZC  | pmt     | recurs |        8 |
  And count "txs" is 3
  And count "tx_requests" is 0
  
Scenario: A delayed payment does not happen immediately
  Given these "tx_templates":
  | id | start   | from | to   | amount | period | purpose |*
  |  8 | %now+1w | .ZZA | .ZZC |     10 | week   | pmt     |
  Then count "txs" is 1
  When cron runs "recurs"
  Then count "txs" is 1
  
Scenario: A recurring payment cannot be completed
  Given these "tx_templates":
  | id | start      | from | to   | amount | period | purpose |*
  |  8 | %yesterday | .ZZA | .ZZB |    200 | week   | pmt     |
  When cron runs "recurs"
  Then invoices:
  | nvid | created   | status       | amount | payer | payee | for  | flags  | recursId |*
  |    1 | %today    | %TX_APPROVED |    200 | .ZZA  | .ZZB  | pmt  | recurs |        8 |
  And count "tx_requests" is 1
  And these "usd":
  | txid | amount | payee | completed | deposit |*
  |    1 |    100 | .ZZA  |         0 |       0 |
  And count "txs" is 2
  And count "usd" is 1
  And count "tx_requests" is 1

  When cron runs "recurs"
  And cron runs "getFunds"
  Then count "txs" is 2
  And these "usd":
  | txid | amount | payee | completed | deposit |*
  |    1 |    200 | .ZZA  |         0 |       0 |
  And count "usd" is 1
  And count "tx_requests" is 1

Skip because member should be allowed to be invoiced?
Scenario: A recurring payment invoice cannot be completed because member is uncarded
  Given invoices:
  | nvid | created   | status       | amount | payer | payee | for | flags  |*
  |    1 | %today    | %TX_APPROVED |     50 | .ZZA  | .ZZB  | pmt | recurs |
  And member ".ZZA" has no photo ID recorded
  When cron runs "getFunds"
  Then count "txs" is 1
  And count "tx_requests" is 1
Resume
