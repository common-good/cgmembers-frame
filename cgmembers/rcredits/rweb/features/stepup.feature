Feature: A user chooses Step Up amounts
AS a member
I WANT to pay a little extra on each transaction
SO I can support my favorite causes easily and often

Setup:
  Given members:
  | uid  | fullName | flags            | balance |*
  | .ZZA | Abe One  | member           | 0       |
  | .ZZB | Bea Two  | ok,confirmed     | 500     |
  | .ZZC | Cor Pub  | ok,confirmed,co  | 0       |
  | .ZZF | Fox Co   | ok,confirmed,co  | 0       |
  | .ZZG | Glo Co   | ok,confirmed,co  | 0       |
  | .ZZH | Hip Co   | ok,confirmed,co  | 0       |

Scenario: A member chooses to Step Up
  Given member ".ZZF" has "%STEPUP_MIN" stepup rules
  And member ".ZZG" has "%(%STEPUP_MIN+1)" stepup rules
  When member ".ZZB" visits page "settings/stepup"
  Then we show "Step Up" with:
  | Organization | $ or % | Max |
  | Fox Co       |        |     |
  | Glo Co       |        |     |
  
  When member ".ZZB" steps up with:
  | .ZZF   | 1% | 1 |
  | .ZZG   | $2 |   |
  | Hip Co | 3% | 4 |
  Then we say "status": "info saved"
  And these "tx_rules":
  | payer     | .ZZB          | .ZZB          | .ZZB          |**
  | payerType | account       | account       | account       |
  | payee     |               |               |               |
  | payeeType | anyCompany    | anyCompany    | anyCompany    |
  | from      | .ZZB          | .ZZB          | .ZZB          |
  | to        | .ZZF          | .ZZG          | .ZZH          |
  | action    | pay           | pay           | pay           |
  | amount    | 0             | 2             | 0             |
  | portion   | .01           | 0             | .03           |
  | purpose   | %STEPUP_DESC  | %STEPUP_DESC  | %STEPUP_DESC  |
  | minimum   | 0             | 0             | 0             |
  | useMax    |               |               |               |
  | extraMax  | 1             |               | 4             |
  | template  |               |               |               |
  | start     | %now          | %now          | %now          |
  | end       |               |               |               |
  | code      |               |               |               |

Scenario: A member's rules come into play
  Given these "tx_rules":
  | id        | 1             | 2             | 3             |**
  | payer     | .ZZB          | .ZZB          | .ZZB          |
  | payerType | account       | account       | account       |
  | payee     |               |               |               |
  | payeeType | anyCompany    | anyCompany    | anyCompany    |
  | from      | .ZZB          | .ZZB          | .ZZB          |
  | to        | .ZZF          | .ZZG          | .ZZH          |
  | action    | pay           | pay           | pay           |
  | amount    | 0             | 2             | 0             |
  | portion   | .01           | 0             | .03           |
  | purpose   | %STEPUP_DESC  | %STEPUP_DESC  | %STEPUP_DESC  |
  | minimum   | 0             | 0             | 0             |
  | useMax    |               |               |               |
  | extraMax  | 1             |               | 2             |
  | template  |               |               |               |
  | start     | %now          | %now          | %now          |
  | end       |               |               |               |
  | code      |               |               |               |
  When member ".ZZB" confirms form "pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Cor Pub | 100    | %FOR_GOODS | labor   |
  Then these "txs":
  | eid | xid | created | amount | payer | payee | purpose      | rule | type        |*
  |   1 |   1 | %today  | 100    | .ZZB  | .ZZC  | labor        |      | %E_PRIME    |
  |   3 |   1 | %today  | 1      | .ZZB  | .ZZF  | %STEPUP_DESC | 1    | %E_DONATION |
  |   4 |   1 | %today  | 2      | .ZZB  | .ZZG  | %STEPUP_DESC | 2    | %E_DONATION |
  |   5 |   1 | %today  | 2      | .ZZB  | .ZZH  | %STEPUP_DESC | 3    | %E_DONATION |
