Feature: Cash out
AS a member company
I WANT to transfer funds to the bank automatically -- weekly or monthly --
SO I don't have to think about it.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | country | postalAddr | flags               | bankAccount | floor |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01000 | US      | 1 A, A, AK | ok,confirmed,bankOk | USkk9000001 |   -20 |
  | .ZZB | Bea Two  | 2 B St. | Btown | PA    | 01002 | US      | 2 B, B, BC | ok,confirmed,cAdmin |             |  -400 |
  | .ZZC | Cor Pub  | 3 C St. | Ctown | CT    | 03000 | US      | 3 C, C, CT | ok,co,confirmed     | USkk9000003 |     0 |
  And transactions:
  | xid | created   | amount | payer | payee | purpose |*
  |   1 | %today-4d |    231 | .ZZB | .ZZC | food    |

Scenario: A member company cashes out monthly
  Given members have:
  | uid  | activated   | flags                    |*
  | .ZZC | %today-30d | ok,co,confirmed,cashoutM |
  When cron runs "tickle"
  Then these "txs2":
  | amount | payee | completed |*
  |   -220 | .ZZC  | %today    |
  
Scenario: A member company cashes out weekly
  Given members have:
  | uid  | activated  | flags                    |*
  | .ZZC | %today-30d | ok,co,confirmed,cashoutW |
  When cron runs "tickle"
  Then balances:
  | uid  | floor  | balance |*
  | .ZZC | -38.50 |     231 |
  Given members have:
  | uid  | activated  |*
  | .ZZC | %today-14d |
  When cron runs "tickle"
  Then these "txs2":
  | amount | payee | completed |*
  |   -220 | .ZZC  | %today    |
  And balances:
  | uid  | floor  | balance |*
  | .ZZC | -38.50 |      11 |
