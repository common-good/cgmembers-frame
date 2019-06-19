Feature: Preferences
AS a member
I WANT certain adjustments to my account to be made automatically every week
SO my financial position will be progressively better.

Setup:
  Given members:
  | uid  | fullName | minimum | savingsAdd | saveWeekly | achMin | floor | risks   | flags                 |*
  | .ZZA | Abe One  |    -100 |          0 |         20 |     20 |    10 | hasBank | ok,confirmed,refill   |
  | .ZZB | Bea Two  |     100 |          0 |         20 |     20 |    10 | hasBank | ok,confirmed,cashoutW |

Scenario: A member crawls out of debt
  When cron runs "everyWeek"
  Then balances:
  | uid  | minimum |*
  | .ZZA |     -80 |

Scenario: A member builds up savings
  Given members have:
  | uid  | minimum |*
  | .ZZA |     100 |
  When cron runs "everyWeek"
  Then balances:
  | uid  | minimum | savingsAdd |*
  | .ZZA |     120 |          0 |

#Scenario: A member draws down savings bit by bit
#  Given members have:
#  | uid  | minimum | savingsAdd | saveWeekly | achMin |*
#  | .ZZA |     100 |         25 |        -20 |     20 |
#  When cron runs "everyWeek"
#  Then balances:
#  | uid  | minimum | savingsAdd |*
#  | .ZZA |     100 |          5 |
#  When cron runs "everyWeek"
#  Then balances:
#  | uid  | minimum | savingsAdd |*
#  | .ZZA |     100 |          0 |

Scenario: A member cashes out automatically
  Given transactions:
  | xid | created   | amount | from | to   | purpose |*
  |   1 | %today-8w |    900 | ctty | .ZZA | signup  |
  |   2 | %today-7w |    200 | .ZZA | .ZZB | stuff   |
  |   3 | %today-6w |    500 | .ZZA | .ZZB | stuff   |
  And members have:
  | uid  | activated | floor |*
  | .ZZB | %today-9w |  -100 |
  Then balances:
  | uid  | balance |*
  | .ZZB |     700 |
  When cron runs "tickle"
  Then usd transfers:
  | txid | payee | amount |*
  |    1 | .ZZB  |   -670 |
#  And we notice "banked|bank tx number" to member ".ZZB" with subs:
#  | action     | amount | checkNum |*
#  | deposit to | $670   |        1 |
  And we notice "banked" to member ".ZZB" with subs:
  | action     | amount | why |*
  | deposit to | $670   |     |
