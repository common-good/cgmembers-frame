Feature: Flow
AS a member
I WANT my account to draw automatically from another, when I overdraw
SO I can spend up to my total credit line.

Setup:
  Given members:
  | uid  | fullName   | flags                |*
  | .ZZA | Abe One    | ok,confirmed         |
  | .ZZB | Bea Two    | ok,confirmed         |
  | .ZZC | Corner Pub | ok,confirmed,co,debt |
  And relations:
  | main | agent | permission | draw |*
  | .ZZC | .ZZA  | manage     |    1 |
  | .ZZC | .ZZB  | sell       |    0 |
  And balances:
  | uid  | balance | floor |*
  | .ZZA |      10 |   -10 |
  | .ZZB |     100 |   -20 |
  | .ZZC |     100 |   -20 |

Scenario: A member draws
  When member ".ZZA" confirms form "pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |     30 | %FOR_GOODS | food    |
  Then transactions:
  | xid | amount | from | to   | purpose      |*
  |   1 |     20 | .ZZC | .ZZA | automatic transfer to NEWZZA,automatic transfer from NEWZZC |
  |   2 |     30 | .ZZA | .ZZB | food         |
  
Scenario: A member draws again
  When member ".ZZA" confirms form "pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |    130 | %FOR_GOODS | food    |
  Then transactions:
  | xid | amount | from | to   | purpose      |*
  |   1 |    120 | .ZZC | .ZZA | automatic transfer to NEWZZA,automatic transfer from NEWZZC |
  |   2 |    130 | .ZZA | .ZZB | food         |

Scenario: A member overdraws with not enough to draw on
  When member ".ZZA" completes form "pay" with values:
  | op  | who  | amount | goods        | purpose |*
  | pay | .ZZB |    200 | %FOR_GOODS | food    |
  Then we say "error": "short to" with subs:
  | short | avail |*
  | $70   | $130  |
  
# add a scenario for drawing from two sources