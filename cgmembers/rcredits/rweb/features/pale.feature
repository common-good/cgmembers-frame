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
  | id | payer | payerType | payeeType | from         | to   | amount | portion | action | start  | purpose   |*
  |  1 | .ZZA  | account   | anybody   | %MATCH_PAYER | .ZZD |      0 | 0.05    | surtx  | %today | food fund |

  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who        | amount | purpose |*
  | pay | Corner Pub | 100    | fun     |
  Then we say "status": "report tx" with subs:
  | did    | otherName  | amount |*
  | paid   | Corner Pub | $100   |
  And these "txs":
  | eid | xid | type     | created | amount | from  | to   | description       | rule |*
  |   1 |   1 | prime    | %today  |    100 | .ZZA  | .ZZC | fun               | 1    |
  |   3 |   1 | donation | %today  |      5 | .ZZA  | .ZZD | food fund         | 1    |
  # MariaDb bug: autonumber skips id=2 when there are record ids 1 and -1
  And balances:
  | uid  | balance |*
  | .ZZA |    -105 |
  | .ZZB |       0 |
  | .ZZC |     100 |
  | .ZZD |       5 |

