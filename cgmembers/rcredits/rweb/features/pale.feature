Feature: Pay A Little Extra
AS a member
I WANT to pay a little extra to the food fund
SO I can feel good about myself

Setup:
  Given members:
  | uid  | fullName   | floor | flags             |*
  | .ZZA | Abe One    |  -250 | ok,confirmed,debt |
  | .ZZB | Bea Two    |  -250 | ok,confirmed,debt |
  | .ZZC | Corner Pub |  -250 | ok,confirmed,co   |
  | .ZZD | Food Fund  |  -250 | ok,confirmed,debt |

Scenario: A member makes a transaction that triggers paying a little extra
  Given txRules:
  | id | payerType | payeeType | fromId | toId | amount | portion | action | start  | purpose   |*
  |  1 | 1         | 1         | -1     | .ZZD |      0 | 0.05    | 1      | %today | food fund |

  When member ".ZZA" confirms form "pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 100    | fun     |
  Then we say "status": "report tx" with subs:
  | did    | otherName  | amount |*
  | paid   | Corner Pub | $100   |
  And these "txs":
  | eid | xid | type   | created | amount | from  | to   | description       |*
  |   1 |   1 | prime  | %today  |    100 | .ZZA  | .ZZC | fun               |
  |   3 |   1 | rebate | %today  |      5 | .ZZA  | .ZZD | food fund         |
  And balances:
  | uid  | balance |*
  | .ZZA |    -105 |
  | .ZZB |       0 |
  | .ZZC |     100 |
  | .ZZD |       5 |

