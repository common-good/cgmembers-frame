Feature: Flow
AS a member
I WANT my account to draw automatically from another, when I overdraw
SO I can spend up to my total credit line.

Setup:
  Given members:
  | uid  | fullName   | flags                | jid  |*
  | .ZZA | Abe One    | ok,confirmed         | .ZZD |
  | .ZZB | Bea Two    | ok,confirmed         |    0 |
  | .ZZC | Corner Pub | ok,confirmed,co,debt |    0 |
  | .ZZD | Dee Four   | ok,confirmed         | .ZZA |
  And relations:
  | main | agent | permission | draw |*
  | .ZZC | .ZZA  | manage     |    1 |
  | .ZZC | .ZZB  | sell       |    0 |
  | .ZZD | .ZZA  | joint      |    0 |
  | .ZZA | .ZZD  | joint      |    0 |
  And balances:
  | uid  | balance | floor |*
  | .ZZA |      10 |   -10 |
  | .ZZB |     100 |   -20 |
  | .ZZC |     100 |   -20 |

Scenario: A member draws
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |     30 | %FOR_GOODS   | food    |
  Then these "tx_entries":
  | xid | entryType | amount | uid  | description                    |*
  |   1 | %E_PRIME  |     20 | .ZZA | automatic transfer from NEWZZC |
  |   1 | %E_PRIME  |    -20 | .ZZC | automatic transfer to NEWZZA   |
  |   2 | %E_PRIME  |     30 | .ZZB | food                           |
  |   2 | %E_PRIME  |    -30 | .ZZA | food                           |
  
Scenario: A joint account slave member draws
  When member ".ZZD" confirms form "tx/pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |     30 | %FOR_GOODS   | food    |
  Then these "tx_entries":
  | xid | entryType | amount | uid  | description                    |*
  |   1 | %E_PRIME  |     20 | .ZZD | automatic transfer from NEWZZC |
  |   1 | %E_PRIME  |    -20 | .ZZC | automatic transfer to NEWZZD   |
  |   2 | %E_PRIME  |     30 | .ZZB | food                           |
  |   2 | %E_PRIME  |    -30 | .ZZD | food                           |
  When member ".ZZD" visits "history/transactions"
  Then we show "Transaction History" with:
  | %mdy | Corner Pub | automatic transfer from NEWZZC | 20.00 | 30.00 | X |
  
Scenario: A member draws again
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |    130 | %FOR_GOODS | food    |
  Then these "tx_entries":
  | xid | entryType | amount | uid  | description      |*
  |   1 | %E_PRIME  |    120 | .ZZA | automatic transfer from NEWZZC |
  |   1 | %E_PRIME  |   -120 | .ZZC | automatic transfer to NEWZZA   |
  |   2 | %E_PRIME  |    130 | .ZZB | food                           |
  |   2 | %E_PRIME  |   -130 | .ZZA | food                           |

Scenario: A member overdraws with not enough to draw on
  When member ".ZZA" completes form "tx/pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |    200 | %FOR_GOODS | food    |
  Then we say "status": "short to|when resolved" with subs:
  | short | avail |*
  | $70   | $130  |
  
# add a scenario for drawing from two sources