Feature: Backing
AS a member
I WANT to set a backing amount
SO I can support my Common Good community in making grants and investments.

Setup:
  Given members:
  | uid  | legalName | backing | floor | flags                         |*
  | .ZZA | Abe One   |       0 |    10 | ok,confirmed,nosearch,paper   |
  | .ZZB | Bea Two   |      10 |     0 | ok,co,confirmed,secret |
  
Scenario: A member visits the backing page
  When member ".ZZA" visits page "community/backing"
  Then we show "Backing Promise" with:
  | Amount | $1 |

Scenario: A member increases backing amount
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |   3 | %today-1m |    250 | ctty | .ZZA | grant   |
  And member ".ZZA" has no photo ID recorded
  When member ".ZZA" completes form "community/backing" with values:
  | amtChoice |*
  |       500 |
  Then members:
  | uid  | backing | backingDate |*
  | .ZZA |     500 | %daystart   |
  
  When member ".ZZA" completes form "community/backing" with values:
  | amtChoice |*
  |      1000 |
  Then members:
  | uid  | backing | backingDate |*
  | .ZZA |    1000 | %daystart   |

Scenario: A member decreases backing amount
  Given members have:
  | uid  | backingDate |*
  | .ZZB | %today-3m   |
  When member ".ZZB" completes form "community/backing" with values:
  | amtChoice |*
  |         5 |
  Then members:
  | uid  | backing | backingDate | backingNext |*
  | .ZZB |      10 | %today-3m   |           5 |
  And we say "status": "backing in effect" with subs:
  | renewDate | %mdY+9m |**
  