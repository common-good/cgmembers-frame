Feature: Backing
AS a member
I WANT to set a backing amount
SO I can support my Common Good community in making grants.

Setup:
  Given members:
  | uid  | legalName | crumbs | minimum | savingsAdd | saveWeekly | achMin | backing | floor | flags   |*
  | .ZZA | Abe One   |    .01 |     100 |          0 |          1 |     20 |       0 |    10 | ok,confirmed,nosearch,paper |
  | .ZZB | Bea Two   |    .02 |     -10 |         10 |          0 |     50 |      10 |     0 | ok,co,confirmed,weekly,secret |
  And transactions: 
  | xid | created   | type   | amount | from | to   | purpose | taking |*
  |   1 | %today-6m | signup |    250 | ctty | .ZZA | signup  | 0      |
  |   2 | %today-6m | signup |    250 | ctty | .ZZB | signup  | 0      |
  
Scenario: A member visits the backing page
  When member ".ZZA" visits page "community/backing"
  Then we show "Backing Promise" with:
  | Amount | $1 |
  | Signed |  |

Scenario: A member changes backing amount
  Given transactions: 
  | xid | created   | type   | amount | from | to   | purpose |*
  |   3 | %today-1m | grant  |    250 | ctty | .ZZA | grant   |
  And member ".ZZA" has no photo ID recorded
  When member ".ZZA" completes form "community/backing" with values:
  | amtChoice | signedBy |*
  |       500 | Abe One  |
  Then members:
  | uid  | backing | backingDate |*
  | .ZZA |     500 | %today      |
  
  When member ".ZZA" completes form "community/backing" with values:
  | amtChoice | signedBy |*
  |      1000 | Abe One  |
  Then members:
  | uid  | backing | backingDate |*
  | .ZZA |    1000 | %today      |

  When member ".ZZA" completes form "community/backing" with values:
  | amtChoice | signedBy |*
  |         1 | Abe One  |
  Then members:
  | uid  | backing | backingDate |*
  | .ZZA |       1 | %today      |
  And we say "status": "backing in effect"
