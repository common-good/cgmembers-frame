Feature: Gifts
AS a member
I WANT to make recurring payments
SO I can save on memory and labor.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | country | postalAddr | flags               | risks   |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01000 | US      | 1 A, A, AK | ok,confirmed,bankOk | hasBank |
  | .ZZC | Cor Pub  | 3 C St. | Ctown | CT    | 03000 | US      | 3 C, C, CT | ok,co,confirmed     |         |
  And balances:
  | uid  | balance | floor |*
  | cgf  |       0 |     0 |
  | .ZZA |     100 |   -20 |

Scenario: A recurring donation happened yesterday
  Given these "recurs":
  | created    | payer | payee | amount | period |*
  | %yesterday | .ZZA  | .ZZC  |     10 |      M |
  And transactions:
  | xid | created    | type     | amount | from | to   | purpose         | flags  |*
  |   1 | %yesterday | transfer |     10 | .ZZA | .ZZC | monthly payment | recurs |
  When cron runs "recurs"
  Then count "txs" is 1
  
Scenario: A recurring donation happened long enough ago to repeat
  Given these "recurs":
  | created    | payer | payee | amount | period |*
  | %today-35d | .ZZA  | .ZZC  |     10 |      M |
  And transactions:
  | xid | created    | type     | amount | from | to   | purpose         | flags  |*
  |   1 | %today-35d | transfer |     10 | .ZZA | .ZZC | monthly payment | recurs |
  When cron runs "recurs"
  Then count "txs" is 2
  And transactions:
  | xid | created | type     | amount | from | to   | purpose         | flags  |*
  |   2 | %today  | transfer |     10 | .ZZA | .ZZC | monthly payment | recurs |  