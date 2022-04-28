Feature: Sweep
AS Common Good
I WANT to batch the donations coming into our main account
SO it's not hard to find the less common transactions

Setup:
  Given members:
  | uid  | fullName | flags             | floor   | coType    |*
  | .ZZA | Abe One  | member            | 0       |           |
  | .ZZB | Bea Two  | ok,confirmed,debt | -500    |           |
  | .ZZC | Cor Pub  | ok,confirmed,co   | 0       | nonprofit |

Scenario: Cron sweeps CG's batch donation accounts into its main account
  Given these "txs":
  | eid | xid | created | amount | payer | payee    | purpose  | type     |*
  |   1 |   1 | %now-2m | 100    | .ZZB  | .ZZC     | labor    | %E_PRIME |
  |   3 |   1 | %now-2m | 1      | .ZZB  | round    | donation | %E_AUX   |
  |   4 |   1 | %now-2m | 2      | .ZZB  | crumb    | donation | %E_AUX   |
  |   5 |   1 | %now-2m | 3      | .ZZC  | stepups  | donation | %E_AUX   |
  |   6 |   2 | %now-2m | 4      | .ZZA  | regulars | donation | %E_PRIME |
  |  11 |  11 | %now+2m | 2100   | .ZZB  | .ZZC     | labor    | %E_PRIME |
  |  13 |  11 | %now+2m | 21     | .ZZB  | round    | donation | %E_AUX   |
  |  14 |  11 | %now+2m | 22     | .ZZB  | crumb    | donation | %E_AUX   |
  |  15 |  11 | %now+2m | 23     | .ZZC  | stepups  | donation | %E_AUX   |
  |  16 |  12 | %now+2m | 24     | .ZZA  | regulars | donation | %E_PRIME |
  Then balances:
  | uid      | balance |*
  | regulars | 28      |
  | round    | 22      |
  | crumb    | 24      |
  | stepups  | 26      |

  When cron runs "sweep"
  Then these "txs":
  | xid | amount | payer    | payee | purpose           |*
  | 13  | 4      | regulars | cgf   | regular donations |
  | 14  | 1      | round    | cgf   | roundup donations |
  | 15  | 2      | crumb    | cgf   | crumb donations   |
  | 16  | 3      | stepups  | cgf   | step-up donations |
  And balances:
  | uid      | balance |*
  | cgf      | 10      |
  | regulars | 24      |
  | round    | 21      |
  | crumb    | 22      |
  | stepups  | 23      |
