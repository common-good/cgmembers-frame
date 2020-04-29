Feature: Recurs
AS a member
I WANT my stepup or discount rules to repeat sometimes
SO I can have things recur in just the right way.

Setup:
  Given members:
  | uid  | fullName  | flags            | balance |*
  | .ZZA | Abe One   | member           | 0       |
  | .ZZB | Bea Two   | ok,confirmed     | 500     |
  | .ZZC | Cor Pub   | ok,confirmed,co  | 0       |
  | .ZZF | Food Fund | ok,confirmed,co  | 0       |
  | .ZZG | Glo Co    | ok,confirmed,co  | 0       |
  | .ZZH | Hip Co    | ok,confirmed,co  | 0       |
  And these "u_groups":
  | id | name      |*
  | 1  | Food Fund |
  And these "u_groupies":
  | grpId | uid  |*
  | 1     | .ZZA |
  | 2     | .ZZB |
  And these "tx_templates":
  | id        | 1            | 2            | 3            | 4             | 5            |**
  | action    | pay          | pay          | surtx        | charge        | surtx        |
  | from      | .ZZB         | .ZZB         | %MATCH_PAYEE | .ZZA          | .ZZF         |
  | to        | .ZZC         | .ZZG         | %MATCH_PAYER | .ZZC          | %MATCH_PAYER |
  | amount    | 100          | 2            | 0            | 123           | 20           |
  | portion   | 0            | 0            | .03          | 0             | 0            |
  | purpose   | payment      | donation     | discount     | invoice       | food help    |
  | payerType | anybody      | anybody      | anybody      | account       | group        |
  | payer     | %NULL        | %NULL        | %NULL        | %NULL         | 1 %NULL      |
  | payeeType | anybody      | anybody      | account      | anybody       | industry     |
  | payee     | %NULL        | %NULL        | .ZZC         | %NULL         | 2            |
  | minimum   | 0            | 0            | 0            | 0             | 0            |
  | useMax    | %NULL        | %NULL        | 2            | %NULL         | %NULL            |
  | amtMax    | 30           | 2            | %NULL        | 123           | 20           |
  | start     | %now-15d     | %now         | %now         | %now-3m       | %now         |
  | end       | %NULL        | %NULL        | %NULL        | %NULL         | %NULL        |
  | period    | week         | month        | quarter      | month         | month        |
  | periods   | 2            | 1            | 1            | 1             | 1            |
  | duration  | once         | once         | month        | once          | forever      |
  | durations | 1            | 1            | 1            | 1             | 1            |

Scenario: Rules get instantiated
  When cron runs "recurs"
  Then these "tx_rules":
  | id        | 1            | 2           |**
  | action    | surtx        | surtx       |
  | from      | %MATCH_PAYEE |.ZZF         |
  | to        | %MATCH_PAYER |%MATCH_PAYER |
  | amount    | 0            | 20          |
  | portion   | .03          | 0           |
  | purpose   | discount     | food help   |
  | payerType | anybody      | group       |
  | payer     |              | 1           |
  | payeeType | account      | industry    |
  | payee     | .ZZC         | 2           |
  | minimum   | 0            | 0           |
  | useMax    | 2            | %NULL       |
  | amtMax    | %NULL        | 20          |
  | start     | %now         | %now        |
  | end       | %now+1m      | %NULL       |
  | template  | 3            | 5           |
  And these "txs":
  | eid | xid | type   | created | amount | payer | payee | purpose                 | rule  | recursId |*
  |   1 |   1 | prime  | %today  |    100 | .ZZB  | .ZZC  | payment (every 2 weeks) | %NULL | 1        |
  |   3 |   1 | rebate | %today  |      3 | .ZZC  | .ZZB  | %REBATE_DESC            | 1     | 1        |
  |   4 |   2 | prime  | %today  |      2 | .ZZB  | .ZZG  | donation (monthly)      | %NULL | 2        |
  And these "invoices":
  | nvid | created | amount | payer | payee | purpose           | recursId |*
  |    1 | %today  |    123 | .ZZA  | .ZZC  | invoice (monthly) | 4        |
