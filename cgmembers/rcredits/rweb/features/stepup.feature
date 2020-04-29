Feature: A user chooses Step Up amounts
AS a member
I WANT to pay a little extra on each transaction
SO I can support my favorite causes easily and often

Setup:
  Given members:
  | uid  | fullName | flags            | balance | coType    |*
  | .ZZA | Abe One  | member           | 0       |           |
  | .ZZB | Bea Two  | ok,confirmed     | 500     |           |
  | .ZZC | Cor Pub  | ok,confirmed,co  | 0       | nonprofit |
  | .ZZF | Fox Co   | ok,confirmed,co  | 0       | nonprofit |
  | .ZZG | Glo Co   | ok,confirmed,co  | 0       | nonprofit |
  | .ZZH | Hip Co   | ok,confirmed,co  | 0       | nonprofit |
  | .ZZI | Ida Co   | ok,confirmed,co  | 0       |           |

Scenario: A member chooses a per-transaction Step Up
  Given member ".ZZF" has "%STEPUP_MIN" stepup rules
  And member ".ZZG" has "%(%STEPUP_MIN+1)" stepup rules
  And member ".ZZI" has "%(%STEPUP_MIN+2)" stepup rules
  When member ".ZZB" visits page "settings/stepup"
  Then we show "Step Up" with:
  | Organization | Amount | Whe | Max |
  | Fox Co       |        |     |     |
  | Glo Co       |        |     |     |
  And without:
  | Ida Co       |        |     |     |
  
  When member ".ZZB" steps up with:
  | .ZZF   | 1 | tx$   |   |
  | .ZZG   | 2 | tx$   |   |
  | Hip Co | 3 | pctmx | 4 |
  Then we say "status": "info saved"
  And these "tx_rules":
  | id        | %(%STEPUP_MIN*3+4) | %(%STEPUP_MIN*3+5) | %(%STEPUP_MIN*3+6) |**
  | payer     | .ZZB          | .ZZB          | .ZZB          |
  | payerType | account       | account       | account       |
  | payee     |               |               |               |
  | payeeType | anyCo         | anyCo         | anyCo         |
  | from      | .ZZB          | .ZZB          | .ZZB          |
  | to        | .ZZF          | .ZZG          | .ZZH          |
  | action    | surtx         | surtx         | surtx         |
  | amount    | 1             | 2             | 0             |
  | portion   | 0             | 0             | .03           |
  | purpose   | %STEPUP_DESC  | %STEPUP_DESC  | %STEPUP_3PCT  |
  | minimum   | 0             | 0             | 0             |
  | useMax    |               |               |               |
  | amtMax    |               |               | 4             |
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
  | payeeType | anyCo         | anyCo         | anyCo         |
  | from      | .ZZB          | .ZZB          | .ZZB          |
  | to        | .ZZF          | .ZZG          | .ZZH          |
  | action    | surtx         | surtx         | surtx         |
  | amount    | 0             | 2             | 0             |
  | portion   | 1             | 0             | .03           |
  | purpose   | %STEPUP_DESC  | %STEPUP_DESC  | %STEPUP_3PCT  |
  | minimum   | 0             | 0             | 0             |
  | useMax    |               |               |               |
  | amtMax    | 1             |               | 2             |
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
  |   5 |   1 | %today  | 2      | .ZZB  | .ZZH  | %STEPUP_3PCT | 3    | %E_DONATION |
  # MariaDb bug: autonumber skips id=2 when there are record ids 1 and -1

Scenario: A member chooses recurring and per-transaction donations
  When member ".ZZB" steps up with:
  |~org    |~amt |~when  |~max |
  | .ZZF   | 1   | pct   |     |
  | .ZZG   | 2   | tx$   |     |
  | .ZZH   | 3   | pctmx | 4   |
  | .ZZG   | 4   | week  |     |
  | .ZZG   | 5   | pct   |     |
  | .ZZC   | 6   | tx$   |     |
  Then we say "status": "info saved"

  When member ".ZZB" visits page "settings/stepup"
  Then we show "Step Up" with:
  | Organization | Amount | Whe          | Max |
  | Cor Pub      | 6      | $ per tx     |     |
  | Fox Co       | 1      | % per tx     |     |
  | Glo Co       | 5      | % per tx     |     |
  | Glo Co       | 4      | week         |     |
  | Hip Co       | 3      | % / tx up to | $4  |
  
  When member ".ZZB" steps up with:
  |~org    |~amt |~when  |~max |
  | .ZZC   | 0   | tx$   |     |
  | .ZZF   | 1   | pct   |     |
  | .ZZG   | 2   | tx$   |     |
  | .ZZH   | 3   | pctmx | 4   |
  | .ZZG   | 0   | week  |     |
  | .ZZG   | 5   | pct   |     |
  Then we say "status": "info saved"
  
  When member ".ZZB" visits page "settings/stepup"
  Then we show "Step Up" with:
  | Organization | Amount | Whe          | Max |
  | Fox Co       | 1      | % per tx     |     |
  | Glo Co       | 5      | % per tx     |     |
  | Hip Co       | 3      | % / tx up to | $4  |
